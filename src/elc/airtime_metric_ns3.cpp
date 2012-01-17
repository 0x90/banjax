/* -*- mode C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <airtime_metric_ns3.hpp>

#include <dot11/frame.hpp>
#include <util/exceptions.hpp>

#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdlib.h>
#include <sstream>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::airtime_metric_ns3;

airtime_metric_ns3::airtime_metric_ns3(encoding_sptr enc, uint16_t rts_cts_threshold) :
   abstract_metric(),
   enc_(enc),
   rts_cts_threshold_(rts_cts_threshold),
   frames_(0),
   packets_(0),
   packet_octets_(0),
   rates_Kbs_sum_(0),
   frames_rate_(),
   packets_rate_(),
   airtime_(0.0)
{
}

airtime_metric_ns3::airtime_metric_ns3(const airtime_metric_ns3& other) :
   abstract_metric(other),
   enc_(other.enc_),
   rts_cts_threshold_(other.rts_cts_threshold_),
   frames_(other.frames_),
   packets_(other.packets_),
   packet_octets_(other.packet_octets_),
   rates_Kbs_sum_(other.rates_Kbs_sum_),
   frames_rate_(other.frames_rate_),
   packets_rate_(other.packets_rate_),
   airtime_(other.airtime_)
{
}

airtime_metric_ns3&
airtime_metric_ns3::operator=(const airtime_metric_ns3& other)
{
   if(this != &other) {
      abstract_metric::operator=(other);
      enc_ = other.enc_;
      rts_cts_threshold_ = other.rts_cts_threshold_;
      frames_ = other.frames_;
      packets_ = other.packets_;
      packet_octets_ = other.packet_octets_;
      rates_Kbs_sum_ = other.rates_Kbs_sum_;
      frames_rate_ = other.frames_rate_;
      packets_rate_ = other.packets_rate_;
      airtime_ = other.airtime_;
   }
   return *this;
}

airtime_metric_ns3::~airtime_metric_ns3()
{
}

void
airtime_metric_ns3::add(buffer_sptr b)
{
   frame f(b);
   frame_control fc(f.fc());
   const uint32_t CRC_SZ = 4;
   buffer_info_sptr info(b->info());
   if(DATA_FRAME == fc.type() && info->has(TX_FLAGS)) {
      // update frame stats
      ++frames_;
      vector<uint32_t> rates(info->rates());
      uint8_t txc = rates.size();
      for(uint8_t i = 0; i < txc; ++i) {
         ++(frames_rate_[rates[i]]);
      }
      // update packet stats
      bool tx_success = (0 == (info->tx_flags() & TX_FLAGS_FAIL));
      if(tx_success) {
         ++packets_;
         packet_octets_ += b->data_size() + CRC_SZ;
         rates_Kbs_sum_ += info->rate_Kbs();
         ++(packets_rate_[rates[txc - 1]]);
      }
   } else if (MGMT_FRAME == fc.type()) {
      // ToDo: adjust for time lost to other's beacons
   }
}

airtime_metric_ns3*
airtime_metric_ns3::clone() const
{
   return new airtime_metric_ns3(*this);
}

double
airtime_metric_ns3::compute(uint32_t ignored_delta_us)
{
   const double PKTS = packets_;
   const double AVG_PKT_SZ = packet_octets_ / PKTS;
   const double AVG_PKT_RATE_Kbs = rates_Kbs_sum_ / PKTS;
   const uint32_t rate_Kbs = closest_rate(AVG_PKT_RATE_Kbs);

#if 0
   // use the rate-specific FDR
   const double FDR = packet_rate_[rate_Kbs] / static_cast<double>(frame_rate_[rate_Kbs]);
#else
   // use the average FDR (should be better for MRR environments)
   const double FDR = packets_ / static_cast<double>(frames_);
#endif


#if 0

   // O_ca + O_p constants from Aure and Li, Optimized Path-Selection using Airtime Metric in NS3 Networks, 2008

   const double O_ca = 75;
   const double O_p = 110;
   const double O = O_ca + O_p;
   const double Bt = 8192;

   const double rate_Mbs = rate_Kbs / 1e3L;
   airtime_ = (O + Bt / rate_Mbs) * (1.0 / FDR);

#else

   // The D3.03 standard appendix V.6 uses this method as does NS-3

   const bool SHORT_PREAMBLE = false;
   const uint32_t TEST_FRAME_SZ = 1024;
   const uint32_t T_RTS_CTS = (rts_cts_threshold_ <= TEST_FRAME_SZ) ? rts_cts_time(enc_, TEST_FRAME_SZ, SHORT_PREAMBLE) : 0;
   const uint32_t T_DATA = enc_->txtime(TEST_FRAME_SZ, rate_Kbs, SHORT_PREAMBLE);
   const uint32_t ACK_SZ = 14;
   const uint32_t ACK_RATE = enc_->response_rate(rate_Kbs);
   const uint32_t T_ACK = enc_->txtime(ACK_SZ, ACK_RATE, SHORT_PREAMBLE);

   const double O =  enc_->DIFS() + T_RTS_CTS + T_DATA + enc_->SIFS() + T_ACK;
   const double Bt = 8 * TEST_FRAME_SZ;

   airtime_ = (O) * (1.0 / FDR);

#endif

   // now convert it to channel rate
   airtime_ = Bt / airtime_; 

   return airtime_;
}

void
airtime_metric_ns3::reset()
{
   frames_ = 0;
   packets_ = 0;
   packet_octets_ = 0;
   rates_Kbs_sum_ = 0;
   frames_rate_.clear();
   packets_rate_.clear();
}

void
airtime_metric_ns3::write(ostream& os) const
{
   os << "airtime: " << airtime_;
}

uint32_t
airtime_metric_ns3::closest_rate(uint32_t r) const
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
