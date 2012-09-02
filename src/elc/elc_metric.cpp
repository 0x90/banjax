/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#include <elc_metric.hpp>
#include <dot11/frame.hpp>
#include <dot11/data_frame.hpp>

#include <iostream>
#include <iomanip>
#include <math.h>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::elc_metric;

elc_metric::elc_metric(uint16_t cw_time_us, uint16_t rts_cts_threshold, uint32_t dead_time) :
   abstract_metric(),
   t_dead_(dead_time),
   cw_time_us_(cw_time_us),
   rts_cts_threshold_(rts_cts_threshold),
   n_pkt_succ_(0),
   t_pkt_succ_(0.0),
   t_pkt_fail_(0.0),
   packet_octets_(0),
   elc_(0),
   stash_packet_octets_(0),
   stash_t_pkt_succ_(0.0),
   stash_t_pkt_fail_(0.0)
{
}

elc_metric::elc_metric(const elc_metric& other) :
   abstract_metric(other),
   t_dead_(other.t_dead_),
   cw_time_us_(other.cw_time_us_),
   rts_cts_threshold_(other.rts_cts_threshold_),
   n_pkt_succ_(other.n_pkt_succ_),
   t_pkt_succ_(other.t_pkt_succ_),
   t_pkt_fail_(other.t_pkt_fail_),
   packet_octets_(other.packet_octets_),
   elc_(other.elc_),
   stash_packet_octets_(other.stash_packet_octets_),
   stash_t_pkt_succ_(other.stash_t_pkt_succ_),
   stash_t_pkt_fail_(other.stash_t_pkt_fail_)
{
}

elc_metric&
elc_metric::operator=(const elc_metric& other)
{
   if(&other != this) {
      abstract_metric::operator=(other);
      t_dead_ = other.t_dead_;
      cw_time_us_ = other.cw_time_us_;
      rts_cts_threshold_ = other.rts_cts_threshold_;
      n_pkt_succ_ = other.n_pkt_succ_;
      t_pkt_succ_ = other.t_pkt_succ_;
      t_pkt_fail_ = other.t_pkt_fail_;
      packet_octets_ = other.packet_octets_;
      elc_ = other.elc_;
      stash_packet_octets_ = other.stash_packet_octets_;
      stash_t_pkt_succ_ = other.stash_t_pkt_succ_;
      stash_t_pkt_fail_ = other.stash_t_pkt_fail_;
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
   buffer_info_sptr info(b->info());
   data_frame_sptr df(f.as_data_frame());
   if(info->has(TX_FLAGS) && df) {
      uint32_t tx_flags = info->tx_flags();
      if(tx_flags & TX_FLAGS_FAIL) {
         t_pkt_fail_ += packet_fail_time(b);
      } else {
         ++n_pkt_succ_;
         t_pkt_succ_ += packet_succ_time(b);
         const uint32_t CRC_SZ = 4;
         packet_octets_ += b->data_size() + CRC_SZ;
      }
   }
}

elc_metric*
elc_metric::clone() const
{
   return new elc_metric(*this);
}

double
elc_metric::compute(uint32_t delta_us)
{
   elc_ = packet_octets_ / (t_pkt_succ_ + t_pkt_fail_ + t_dead_);
   stash_packet_octets_ = packet_octets_;
   stash_t_pkt_succ_ = t_pkt_succ_;
   stash_t_pkt_fail_ = t_pkt_fail_;
   return elc_;
}

void
elc_metric::reset()
{
   n_pkt_succ_ = 0;
   t_pkt_succ_ = 0;
   t_pkt_fail_ = 0;
   packet_octets_ = 0;
}

void
elc_metric::write(ostream& os) const
{
#if 1
   os << "n-octets: " << stash_packet_octets_ << ", ";
   os << "t-pkt-succ: " << stash_t_pkt_succ_ << ", ";
   os << "t-pkt-fail: " << stash_t_pkt_fail_ << ", ";
   os << "t-dead: " << t_dead_ << ", ";
#endif
   os << "ELC: " << elc_;
}

double
elc_metric::packet_succ_time(buffer_sptr b) const
{
   double usecs = 0.0;
   buffer_info_sptr info(b->info());
   encoding_sptr enc(info->channel_encoding());
   uint8_t retries = info->data_retries();
   for(uint8_t i = 0; i < retries; ++i) {
      usecs += (cw_time_us_ ? cw_time_us_ : avg_contention_time(enc, i)) + frame_fail_time(b);
   }
   return usecs + (cw_time_us_ ? cw_time_us_ : avg_contention_time(enc, retries)) + frame_succ_time(b);
}

double
elc_metric::packet_fail_time(buffer_sptr b) const
{
   double usecs = 0.0;
   buffer_info_sptr info(b->info());
   encoding_sptr enc(info->channel_encoding());
   uint8_t retries = info->data_retries();
   for(uint8_t i = 0; i < retries + 1; ++i) {
      usecs += (cw_time_us_ ? cw_time_us_ : avg_contention_time(enc, i)) + frame_fail_time(b);
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
   const uint32_t T_RTS_CTS = (rts_cts_threshold_ <= FRAME_SZ) ? rts_cts_time(enc, FRAME_SZ, PREAMBLE) : 0;
   const uint32_t DATA_RATE = info->rate_Kbs();
   const uint32_t T_DATA = enc->txtime(FRAME_SZ, DATA_RATE, PREAMBLE);
   const uint32_t ACK_SZ = 14;
   const uint32_t ACK_RATE = enc->response_rate(DATA_RATE);
   const uint32_t T_ACK = enc->txtime(ACK_SZ, ACK_RATE, PREAMBLE);

   /* TODO: make this QoS-aware? */
   return /**/ 9 + /**/ enc->DIFS() + T_RTS_CTS + T_DATA + enc->SIFS() + T_ACK;
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

   /* TODO: make this QoS-aware? */
//   return enc->DIFS() + T_RTS_CTS + T_DATA + enc->ACKTimeout();
   return /**/ 9 + /**/ enc->DIFS() + T_RTS_CTS + T_DATA;
}
