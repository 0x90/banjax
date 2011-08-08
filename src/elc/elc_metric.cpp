/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#include <elc_metric.hpp>
#include <dot11/frame.hpp>

#include <iostream>
#include <iomanip>
#include <math.h>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::elc_metric;

elc_metric::elc_metric(uint16_t rts_cts_threshold) :
   abstract_metric(),
   rts_cts_threshold_(rts_cts_threshold),
   n_pkt_succ_(0),
   t_pkt_succ_(0.0),
   t_pkt_fail_(0.0),
   packet_octets_(0),
   packet_count_(0)
{
}

elc_metric::elc_metric(const elc_metric& other) :
   abstract_metric(other),
   rts_cts_threshold_(other.rts_cts_threshold_),
   n_pkt_succ_(other.n_pkt_succ_),
   t_pkt_succ_(other.t_pkt_succ_),
   t_pkt_fail_(other.t_pkt_fail_),
   packet_octets_(other.packet_octets_),
   packet_count_(other.packet_count_)
{
}

elc_metric&
elc_metric::operator=(const elc_metric& other)
{
   if(&other != this) {
      abstract_metric::operator=(other);
      rts_cts_threshold_ = other.rts_cts_threshold_;
      n_pkt_succ_ = other.n_pkt_succ_;
      t_pkt_succ_ = other.t_pkt_succ_;
      t_pkt_fail_ = other.t_pkt_fail_;
      packet_octets_ = other.packet_octets_;
      packet_count_ = other.packet_count_;
   }
   return *this;
}

elc_metric::~elc_metric()
{
}

void
elc_metric::add(buffer_sptr b)
{
   frame f(b);
   frame_control fc(f.fc());
   buffer_info_sptr info(b->info());
   if(info->has(TX_FLAGS) && fc.type() == DATA_FRAME) {
      // update totals for packet size and count
      const uint32_t IEEE80211_HDR_SZ = 26;
      const uint32_t LLC_HDR_SZ = 8;
      const uint32_t IP_HDR_SZ = 20;
      const uint32_t UDP_HDR_SZ = 8;
      // ToDo: consider ignoring frames which have a non-TCP/UDP payload?
      packet_octets_ += b->data_size() - IEEE80211_HDR_SZ - LLC_HDR_SZ - IP_HDR_SZ - UDP_HDR_SZ;
      ++packet_count_;

      // compute the time taken to send this packet - whether good or bad
      uint32_t tx_flags = info->tx_flags();
      if(tx_flags & TX_FLAGS_FAIL) {
         t_pkt_fail_ += packet_fail_time(b);
      } else {
         ++n_pkt_succ_;
         t_pkt_succ_ += packet_succ_time(b);
      }
   }
}

elc_metric*
elc_metric::clone() const
{
   return new elc_metric(*this);
}

double
elc_metric::metric() const
{
   double m = 0.0;
   if(0 < packet_count_) {
      const double AVG_PKT_SZ = packet_octets_ / static_cast<double>(packet_count_);
      m = (n_pkt_succ_ * AVG_PKT_SZ) / (t_pkt_succ_ + t_pkt_fail_);
   }
   return m;
}

void
elc_metric::reset()
{
   n_pkt_succ_ = 0;
   t_pkt_succ_ = 0;
   t_pkt_fail_ = 0;
   packet_octets_ = 0;
   packet_count_ = 0;
}

void
elc_metric::write(ostream& os) const
{
   os << "ELC: " << metric();
}

double
elc_metric::packet_succ_time(buffer_sptr b) const
{
   double usecs = 0.0;
   buffer_info_sptr info(b->info());
   encoding_sptr enc(info->channel_encoding());
   uint8_t txc = 1 + info->data_retries();
   for(uint8_t i = 0; i < txc - 1; ++i) {
      usecs += avg_contention_time(enc, i) + frame_fail_time(b);
   }
   return usecs + avg_contention_time(enc, txc) + frame_succ_time(b);
}

double
elc_metric::packet_fail_time(buffer_sptr b) const
{
   double usecs = 0.0;
   buffer_info_sptr info(b->info());
   encoding_sptr enc(info->channel_encoding());
   uint8_t txc = 1 + info->data_retries();
   for(uint8_t i = 0; i < txc; ++i) {
      usecs += avg_contention_time(enc, i) + frame_fail_time(b);
   }
   return usecs;
}

double 
elc_metric::frame_succ_time(buffer_sptr b) const
{
   buffer_info_sptr info(b->info());
   encoding_sptr enc(info->channel_encoding());

   const uint32_t CRC_SZ = 4;
   const uint32_t FRAME_SZ = b->data_size() + CRC_SZ;
   const bool PREAMBLE =  info->has(CHANNEL_FLAGS) && (info->channel_flags() & CHANNEL_PREAMBLE_SHORT);
   const uint32_t T_RTS_CTS =  (rts_cts_threshold_ <= FRAME_SZ) ? rts_cts_time(enc, FRAME_SZ, PREAMBLE) : 0;
   const uint32_t DATA_RATE = info->rate_Kbs();
   const uint32_t T_DATA = enc->txtime(FRAME_SZ, DATA_RATE, PREAMBLE);
   const uint32_t ACK_SZ = 14;
   const uint32_t ACK_RATE = enc->response_rate(DATA_RATE);
   const uint32_t T_ACK = enc->txtime(ACK_SZ, ACK_RATE, PREAMBLE);

   return T_RTS_CTS + T_DATA + enc->SIFS() + T_ACK + enc->DIFS();
}

double
elc_metric::frame_fail_time(buffer_sptr b) const
{
   buffer_info_sptr info(b->info());
   encoding_sptr enc(info->channel_encoding());

   const uint32_t CRC_SZ = 4;
   const uint32_t FRAME_SZ = b->data_size() + CRC_SZ;
   const bool PREAMBLE =  info->has(CHANNEL_FLAGS) && (info->channel_flags() & CHANNEL_PREAMBLE_SHORT);
   const uint32_t T_RTS_CTS = (rts_cts_threshold_ <= FRAME_SZ) ? rts_cts_time(enc, FRAME_SZ, PREAMBLE) : 0;
   const uint32_t DATA_RATE = info->rate_Kbs();
   const uint32_t T_DATA = enc->txtime(FRAME_SZ, DATA_RATE, PREAMBLE);

   return T_RTS_CTS + T_DATA + enc->SIFS() + enc->ACKTimeout() + enc->DIFS();
}
