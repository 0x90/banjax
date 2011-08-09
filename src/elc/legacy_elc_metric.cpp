/* -*- mode C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <legacy_elc_metric.hpp>

#include <dot11/frame.hpp>

#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdlib.h>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::legacy_elc_metric;

legacy_elc_metric::legacy_elc_metric(encoding_sptr enc) :
   enc_(enc),
   frames_(0),
   packets_(0),
   packet_octets_(0),
   rates_Kbs_sum_(0)
{
}

legacy_elc_metric::legacy_elc_metric(const legacy_elc_metric& other) :
   abstract_metric(other),
   enc_(other.enc_),
   frames_(other.frames_),
   packets_(other.packets_),
   packet_octets_(other.packet_octets_),
   rates_Kbs_sum_(other.rates_Kbs_sum_)
{
}

legacy_elc_metric&
legacy_elc_metric::operator=(const legacy_elc_metric& other)
{
   if(this != &other) {
      abstract_metric::operator=(other);
      enc_ = other.enc_;
      frames_ = other.frames_;
      packets_ = other.packets_;
      packet_octets_ = other.packet_octets_;
      rates_Kbs_sum_ = other.rates_Kbs_sum_;
   }
   return *this;
}

legacy_elc_metric::~legacy_elc_metric()
{
}

void
legacy_elc_metric::add(buffer_sptr b)
{
   frame f(b);
   frame_control fc(f.fc());
   buffer_info_sptr info(b->info());
   if(DATA_FRAME == fc.type() && info->has(TX_FLAGS)) {
      bool tx_success = (0 == (info->tx_flags() & TX_FLAGS_FAIL));
      if(tx_success && info->channel_encoding() == enc_) {
         ++packets_;
         packet_octets_ += b->data_size();
         rates_Kbs_sum_ += info->rate_Kbs();
      }
      frames_ += info->has(DATA_RETRIES) ? 1 + info->data_retries() : 1;
   }
}

legacy_elc_metric*
legacy_elc_metric::clone() const
{
   return new legacy_elc_metric(*this);
}

double
legacy_elc_metric::metric() const
{
   const double FRMS = frames_;
   const double PKTS = packets_;
   const double AVG_PKT_SZ = packet_octets_ / PKTS;
   const double AVG_PKT_RATE_Kbs = rates_Kbs_sum_ / PKTS;

   const double BEST_TX_TIME = successful_tx_time(closest_rate(AVG_PKT_RATE_Kbs), AVG_PKT_SZ);
   const double EMT = AVG_PKT_SZ / BEST_TX_TIME;
   const double FDR = PKTS / FRMS;
   return FDR * EMT;
}

void
legacy_elc_metric::reset()
{
   frames_ = 0;
   packets_ = 0;
   packet_octets_ = 0;
   rates_Kbs_sum_ = 0;
}

void
legacy_elc_metric::write(ostream& os) const
{
   os << "LegacyELC: " << metric();
}

uint32_t
legacy_elc_metric::closest_rate(uint32_t r) const
{
   uint32_t rate = 0;
   uint32_t d = UINT32_MAX;
   rateset rates(enc_->supported_rates());
   for(rateset::const_iterator i(rates.begin()); i != rates.end(); ++i) {
      uint32_t t = llabs(static_cast<int64_t>(*i) - static_cast<int64_t>(r));
      if(t < d) {
         d = t;
         rate = *i;
      }      
   }
   return rate;
}

uint32_t 
legacy_elc_metric::successful_tx_time(uint32_t rate_Kbs, uint16_t frame_sz) const
{
   const uint32_t T_CW = avg_contention_time(enc_, 0);
   const uint32_t T_RTS_CTS = /* (rts_cts_threshold_ <= FRAME_SZ) ? rts_cts_time(enc, FRAME_SZ, PREAMBLE) : */ 0;
   const uint32_t T_DATA = enc_->txtime(frame_sz, rate_Kbs, false);
   const uint32_t ACK_SZ = 14;
   const uint32_t T_ACK = enc_->txtime(ACK_SZ, enc_->response_rate(rate_Kbs), false);

   return T_CW + T_RTS_CTS + T_DATA + enc_->SIFS() + T_ACK + enc_->DIFS();
}
