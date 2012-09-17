/* -*- mode C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <airtime_metric.hpp>

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
using metrics::airtime_metric;

airtime_metric::airtime_metric(encoding_sptr enc, uint16_t rts_cts_threshold) :
   abstract_metric(),
   enc_(enc),
   rts_cts_threshold_(rts_cts_threshold),
   frames_(0),
   packets_(0),
   packet_octets_(0),
   rates_Kbs_sum_(0),
   airtime_(0.0)
{
}

airtime_metric::airtime_metric(const airtime_metric& other) :
   abstract_metric(other),
   enc_(other.enc_),
   rts_cts_threshold_(other.rts_cts_threshold_),
   frames_(other.frames_),
   packets_(other.packets_),
   packet_octets_(other.packet_octets_),
   rates_Kbs_sum_(other.rates_Kbs_sum_),
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
      frames_ = other.frames_;
      packets_ = other.packets_;
      packet_octets_ = other.packet_octets_;
      rates_Kbs_sum_ = other.rates_Kbs_sum_;
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
      // update frame stats
      frames_ += 1 + info->data_retries();
      // update packet stats
      ++packets_;
      packet_octets_ += b->data_size() + CRC_SZ;
      bool tx_success = (!(info->tx_flags() & TX_FLAGS_FAIL));
      if(tx_success) {
         rates_Kbs_sum_ += info->rate_Kbs();
      }
   } else if (MGMT_FRAME == fc.type()) {
      // ToDo: adjust for time lost to other's beacons?
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
   const double AVG_PKT_RATE_Kbs = rates_Kbs_sum_ / PKTS;
   const uint32_t rate_Kbs = closest_rate(enc_, AVG_PKT_RATE_Kbs);
   const double FDR = packets_ / static_cast<double>(frames_);

   const bool SHORT_PREAMBLE = false;
   const uint32_t UDP_SZ = 62;
   const uint32_t CRC_SZ = 4;
   const uint32_t TEST_FRAME_SZ = 1024 + UDP_SZ + CRC_SZ;
   const uint32_t T_RTS_CTS = (rts_cts_threshold_ <= TEST_FRAME_SZ) ? rts_cts_time(enc_, TEST_FRAME_SZ, SHORT_PREAMBLE) : 0;
   const uint32_t T_DATA = enc_->txtime(TEST_FRAME_SZ, rate_Kbs, SHORT_PREAMBLE);
   const uint32_t ACK_SZ = 14;
   const uint32_t ACK_RATE = enc_->response_rate(rate_Kbs);
   const uint32_t T_ACK = enc_->txtime(ACK_SZ, ACK_RATE, SHORT_PREAMBLE);

   // ToDo: use enc->AIFS instead of DIFS + slot_time
   const double O = /**/ enc_->DIFS() + enc_->slot_time() /**/ + T_RTS_CTS + enc_->SIFS() + T_ACK;

   airtime_ = TEST_FRAME_SZ / ((O + T_DATA) * (1.0 / FDR));

   return airtime_;
}

void
airtime_metric::reset()
{
   frames_ = 0;
   packets_ = 0;
   packet_octets_ = 0;
   rates_Kbs_sum_ = 0;
}

void
airtime_metric::write(ostream& os) const
{
   os << "Airtime: " << airtime_;
}
