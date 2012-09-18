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
   packets_(0),
   packet_octets_(0),
   rates_Kbs_sum_(0),
   memory_time_(1000000),
   last_update_(0),
   fail_avg_(1.0),
   airtime_(0.0)
{
}

airtime_metric_ns3::airtime_metric_ns3(const airtime_metric_ns3& other) :
   abstract_metric(other),
   enc_(other.enc_),
   rts_cts_threshold_(other.rts_cts_threshold_),
   packets_(other.packets_),
   packet_octets_(other.packet_octets_),
   rates_Kbs_sum_(other.rates_Kbs_sum_),
   memory_time_(other.memory_time_),
   last_update_(other.last_update_),
   fail_avg_(other.fail_avg_),
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
      packets_ = other.packets_;
      packet_octets_ = other.packet_octets_;
      rates_Kbs_sum_ = other.rates_Kbs_sum_;
      memory_time_ = other.memory_time_;
      last_update_ = other.last_update_;
      fail_avg_ = other.fail_avg_;
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
   const uint64_t NOW = info->timestamp_wallclock();
   if(DATA_FRAME == fc.type() && info->has(TX_FLAGS)) {
      double avg_coeff = exp((double)(-1.0 * (NOW - last_update_)) / memory_time_);
      last_update_ = NOW;
      bool tx_success = (0 == (info->tx_flags() & TX_FLAGS_FAIL));
      if(tx_success) {
         const double TXC = 1.0 + info->data_retries();
         fail_avg_ = TXC / (1.0 + TXC) * (1.0 - avg_coeff) + avg_coeff * fail_avg_;
      } else {
         fail_avg_ = (1.0 - avg_coeff) + avg_coeff * fail_avg_;
      }
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
   // ToDo: still don't know how we can do any better than this!
   const double PKTS = packets_;
   const double AVG_PKT_SZ = packet_octets_ / PKTS;
   const double AVG_PKT_RATE_Kbs = rates_Kbs_sum_ / PKTS;
   const uint32_t rate_Kbs = closest_rate(enc_, AVG_PKT_RATE_Kbs);

   // 802.11s appendix Y.5 uses this sort of method to calculate O + \frac{B_t}{r}
   const bool SHORT_PREAMBLE = false;
   const uint32_t IP_SZ = 62;
   const uint32_t CRC_SZ = 4;
   const uint32_t TEST_FRAME_SZ = 1024 + IP_SZ + CRC_SZ;
   const uint32_t T_RTS_CTS = (rts_cts_threshold_ <= TEST_FRAME_SZ) ? rts_cts_time(enc_, TEST_FRAME_SZ, SHORT_PREAMBLE) : 0;
   const uint32_t T_DATA = enc_->txtime(TEST_FRAME_SZ, rate_Kbs, SHORT_PREAMBLE);
   const uint32_t ACK_SZ = 14;
   const uint32_t ACK_RATE = enc_->response_rate(rate_Kbs);
   const uint32_t T_ACK = enc_->txtime(ACK_SZ, ACK_RATE, SHORT_PREAMBLE);

   // diagnostix
   if(1.0 <= fail_avg_) {
      cerr << "eek! " << fail_avg_ << endl;
   }

   // this is how NS-3 does it (but without conversion to TUs)
   airtime_ =  static_cast<double>(enc_->DIFS() + T_RTS_CTS + T_DATA + enc_->SIFS() + T_ACK) / (1.0 - fail_avg_);

   // NOTE: we convert airtime to a channel rate in MB/s so we can compare
   const double Bt = TEST_FRAME_SZ;
   airtime_ = TEST_FRAME_SZ / airtime_; 

   return airtime_;
}

void
airtime_metric_ns3::reset()
{
   packets_ = 0;
   packet_octets_ = 0;
   rates_Kbs_sum_ = 0;
}

void
airtime_metric_ns3::write(ostream& os) const
{
   os << "Airtime-NS3: " << airtime_;
}
