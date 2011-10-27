/* -*- mode C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <airtime_metric.hpp>

#include <dot11/frame.hpp>

#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdlib.h>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::airtime_metric;

airtime_metric::airtime_metric(encoding_sptr enc, uint16_t rts_cts_threshold) :
   abstract_metric(),
   enc_(enc),
   rts_cts_threshold_(rts_cts_threshold),
   packets_(0),
   packet_octets_(0),
   rates_Kbs_sum_(0),
   rate_fdr_(),
   airtime_(0.0)
{
}

airtime_metric::airtime_metric(const airtime_metric& other) :
   abstract_metric(other),
   enc_(other.enc_),
   rts_cts_threshold_(other.rts_cts_threshold_),
   packets_(other.packets_),
   packet_octets_(other.packet_octets_),
   rates_Kbs_sum_(other.rates_Kbs_sum_),
   rate_fdr_(other.rate_fdr_),
   airtime_(other.airtime_)
{
}

airtime_metric&
airtime_metric::operator=(const airtime_metric& other)
{
   if(this != &other) {
      abstract_metric::operator=(other);
      enc_ = other.enc_;
      rts_cts_threshold_ = other.rts_cts_threshold_;
      packets_ = other.packets_;
      packet_octets_ = other.packet_octets_;
      rates_Kbs_sum_ = other.rates_Kbs_sum_;
      rate_fdr_ = other.rate_fdr_;
      airtime_ = other.airtime_;
   }
   return *this;
}

airtime_metric::~airtime_metric()
{
}

void
airtime_metric::add(buffer_sptr b)
{
   frame f(b);
   frame_control fc(f.fc());
   const uint32_t CRC_SZ = 4;
   buffer_info_sptr info(b->info());
   if(DATA_FRAME == fc.type() && info->has(TX_FLAGS)) {
      bool tx_success = (0 == (info->tx_flags() & TX_FLAGS_FAIL));
      if(tx_success && info->channel_encoding() == enc_) {
         ++packets_;
         packet_octets_ += b->data_size() + CRC_SZ;
         rates_Kbs_sum_ += info->rate_Kbs();
      }
      // ToDo: update rate_2_fdr ratio for each rate in rates tuple
   }
}

airtime_metric*
airtime_metric::clone() const
{
   return new airtime_metric(*this);
}

double
airtime_metric::compute(uint32_t ignored_delta_us)
{
   const double PKTS = packets_;
   const double AVG_PKT_SZ = packet_octets_ / PKTS;
   const double AVG_PKT_RATE_Kbs = rates_Kbs_sum_ / PKTS;

   const uint32_t rate_Kbs = closest_rate(AVG_PKT_RATE_Kbs);
   map_rate_fdr_t::iterator fdr(rate_fdr_[rate_Kbs]);
   if(rate_fdr_.end() == fdr) {
      ostringstream msg;
      msg << "can't find FDR for rate " << rate_Kbs;
      raise<logic_error>(__PRETTY_FUNCTION__, __FILE__, __LINE__, r.str());
   }
   const double FDR = fdr->second ? fdr->first / (fdr->first + fdr->second) : 1;

   const uint32_t O = enc->DIFS() + ;
   const uint32_t Bt = 8192;


   airtime = enc->

   return airtime_;
}

void
airtime_metric::reset()
{
   packets_ = 0;
   packet_octets_ = 0;
   rates_Kbs_sum_ = 0;
   rate_fdr_.clear();
}

void
airtime_metric::write(ostream& os) const
{
   os << "airtime: " << airtime_;
}

uint32_t
airtime_metric::closest_rate(uint32_t r) const
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
