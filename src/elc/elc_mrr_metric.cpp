/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#include <elc_mrr_metric.hpp>
#include <dot11/frame.hpp>
#include <util/exceptions.hpp>

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <math.h>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::elc_mrr_metric;
using util::raise;

elc_mrr_metric::elc_mrr_metric(uint16_t rts_cts_threshold) :
   rts_cts_threshold_(rts_cts_threshold),
   n_pkt_succ_(0),
   t_pkt_succ_(0.0),
   t_pkt_fail_(0.0),
   packet_octets_(0),
   packet_count_(0)
{
}

elc_mrr_metric::elc_mrr_metric(const elc_mrr_metric& other) :
   rts_cts_threshold_(other.rts_cts_threshold_),
   n_pkt_succ_(other.n_pkt_succ_),
   t_pkt_succ_(other.t_pkt_succ_),
   t_pkt_fail_(other.t_pkt_fail_),
   packet_octets_(other.packet_octets_),
   packet_count_(other.packet_count_)
{
}

elc_mrr_metric&
elc_mrr_metric::operator=(const elc_mrr_metric& other)
{
   if(&other != this) {
      rts_cts_threshold_ = other.rts_cts_threshold_;
      n_pkt_succ_ = other.n_pkt_succ_;
      t_pkt_succ_ = other.t_pkt_succ_;
      t_pkt_fail_ = other.t_pkt_fail_;
      packet_octets_ = other.packet_octets_;
      packet_count_ = other.packet_count_;
   }
   return *this;
}

elc_mrr_metric::~elc_mrr_metric()
{
}

void
elc_mrr_metric::add(buffer_sptr b)
{
   frame f(b);
   frame_control fc(f.fc());
   buffer_info_sptr info(b->info());
   if(info->has(TX_FLAGS) && fc.type() == DATA_FRAME) {
      // update totals for packet size and count
      const uint32_t LLC_HDR_SZ = 8;
      const uint32_t IEEE80211_HDR_SZ = 24;
      const uint32_t IP_HDR_SZ = 20;
      const uint32_t UDP_HDR_SZ = 8;
      packet_octets_ += b->data_size() - IEEE80211_HDR_SZ - LLC_HDR_SZ - IP_HDR_SZ - UDP_HDR_SZ;
      ++packet_count_;

      // compute the time taken to send this packet - whether good or bad
      buffer_info_sptr info(b->info());
      uint32_t tx_flags = info->tx_flags();
      if(tx_flags & TX_FLAGS_FAIL) {
         t_pkt_fail_ += packet_fail_time(b);
      } else {
         ++n_pkt_succ_;
         t_pkt_succ_ += packet_succ_time(b);
      }
   }
}

elc_mrr_metric*
elc_mrr_metric::clone() const
{
   return new elc_mrr_metric(*this);
}

double
elc_mrr_metric::metric() const
{
   const double AVG_PKT_SZ = packet_octets_ / static_cast<double>(packet_count_);
   return (n_pkt_succ_ * AVG_PKT_SZ) / (t_pkt_succ_ + t_pkt_fail_);
}

void
elc_mrr_metric::reset()
{
   n_pkt_succ_ = 0;
   t_pkt_succ_ = 0;
   t_pkt_fail_ = 0;
   packet_octets_ = 0;
   packet_count_ = 0;
}

void
elc_mrr_metric::write(ostream& os) const
{
   os << "ELC-MRR: ";
   if(0 < packet_count_)
      os << metric();
   else
      os << "N/A";
}

double
elc_mrr_metric::packet_succ_time(buffer_sptr b) const
{
   double usecs = 0.0;
   buffer_info_sptr info(b->info());
   vector<uint32_t> rates(info->rates());
   encoding_sptr enc(info->channel_encoding());
   uint8_t txc = rates.size();
   for(uint8_t i = 0; i < txc - 1; ++i) {
      usecs += avg_contention_time(enc, i) + frame_fail_time(b, rates[i]);
   }
   return usecs + avg_contention_time(enc, txc) + frame_succ_time(b, rates[txc - 1]);
}

double
elc_mrr_metric::packet_fail_time(buffer_sptr b) const
{
   double usecs = 0.0;
   buffer_info_sptr info(b->info());
   vector<uint32_t> rates(info->rates());
   encoding_sptr enc(info->channel_encoding());
   uint8_t txc = rates.size();
   for(uint8_t i = 0; i < txc; ++i) {
      usecs += avg_contention_time(enc, i) + frame_fail_time(b, rates[i]);
   }
   return usecs;
}

double
elc_mrr_metric::avg_contention_time(encoding_sptr enc, uint8_t txc) const
{
   return max_contention_time(enc, txc) / 2.0;
}

double
elc_mrr_metric::max_contention_time(encoding_sptr enc, uint8_t txc) const
{
  /* ath5k hack: collapse contention window after 10 attempts */
  if(txc >= 10) {
    txc %= 10;
  }
  /* end hack */
  const uint32_t CWMIN = enc->CWMIN();
  const uint32_t CWMAX = enc->CWMAX();
  const uint32_t CW = pow(2, txc+4) - 1; // ToDo: fix me so I work on any encoding!
  return min(max(CW, CWMIN), CWMAX) * enc->slot_time();
}

double 
elc_mrr_metric::frame_succ_time(buffer_sptr b, uint32_t rate_Kbs) const
{
   buffer_info_sptr info(b->info());
   encoding_sptr enc(info->channel_encoding());

   const uint32_t CRC_SZ = 4;
   const uint32_t FRAME_SZ = b->data_size() + CRC_SZ;
   const bool PREAMBLE =  info->has(CHANNEL_FLAGS) && (info->channel_flags() & CHANNEL_PREAMBLE_SHORT);
   const uint32_t T_RTS_CTS = rts_cts_time(enc, FRAME_SZ, PREAMBLE);
   const uint32_t T_DATA = enc->txtime(FRAME_SZ, rate_Kbs, PREAMBLE);
   const uint32_t ACK_SZ = 14;
   const uint32_t ACK_RATE = enc->response_rate(rate_Kbs);
   const uint32_t T_ACK = enc->txtime(ACK_SZ, ACK_RATE, PREAMBLE);

   return T_RTS_CTS + T_DATA + enc->SIFS() + T_ACK + enc->DIFS();
}

double
elc_mrr_metric::frame_fail_time(buffer_sptr b, uint32_t rate_Kbs) const
{
   buffer_info_sptr info(b->info());
   encoding_sptr enc(info->channel_encoding());

   const uint32_t CRC_SZ = 4;
   const uint32_t FRAME_SZ = b->data_size() + CRC_SZ;
   const bool PREAMBLE =  info->has(CHANNEL_FLAGS) && (info->channel_flags() & CHANNEL_PREAMBLE_SHORT);
   const uint32_t T_RTS_CTS = rts_cts_time(enc, FRAME_SZ, PREAMBLE);
   const uint32_t T_DATA = enc->txtime(FRAME_SZ, rate_Kbs, PREAMBLE);
   const uint32_t T_ACKTIMEOUT = 50; // ToDo: get me from encoding!!!

   return T_RTS_CTS + T_DATA + enc->SIFS() + T_ACKTIMEOUT + enc->DIFS();
}

double
elc_mrr_metric::rts_cts_time(encoding_sptr enc, uint32_t frame_sz, bool short_preamble) const
{
   double usecs = 0.0;
   if(rts_cts_threshold_ <= frame_sz) {
      const uint32_t RTS_SZ = 20;
      const uint32_t CTS_SZ = 14;
      const uint32_t T_SIFS = enc->SIFS();
      const uint32_t RATE = enc->default_rate();
      usecs = enc->txtime(RTS_SZ, RATE, short_preamble) + T_SIFS + enc->txtime(CTS_SZ, RATE, short_preamble) + T_SIFS;
   }
   return usecs;
}
