/* -*- mode C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <airtime_metric_measured.hpp>

#include <dot11/frame.hpp>
#include <util/exceptions.hpp>

#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::airtime_metric_measured;

airtime_metric_measured::airtime_metric_measured() :
   abstract_metric(),
   airtime_(0.0),
   packets_(0),
   metric_(0.0)
{
}

airtime_metric_measured::airtime_metric_measured(const airtime_metric_measured& other) :
   abstract_metric(other),
   airtime_(other.airtime_),
   packets_(other.packets_),
   metric_(other.metric_)
{
}

airtime_metric_measured&
airtime_metric_measured::operator=(const airtime_metric_measured& other)
{
   if(this != &other) {
      abstract_metric::operator=(other);
      airtime_ = other.airtime_;
      packets_ = other.packets_;
      metric_ = other.metric_;
   }
   return *this;
}

airtime_metric_measured::~airtime_metric_measured()
{
}

void
airtime_metric_measured::add(buffer_sptr b)
{
   frame f(b);
   buffer_info_sptr info(b->info());
   if(info->has(TX_FLAGS)) {
      int32_t airtime = info->packet_time();
      if(0 < airtime && airtime < UINT32_C(0x80000000)) {
         bool tx_success = (0 == (info->tx_flags() & TX_FLAGS_FAIL));
         if(tx_success) {
            ++packets_;
         }
         airtime_ += airtime;
      }
   }
}

airtime_metric_measured*
airtime_metric_measured::clone() const
{
   return new airtime_metric_measured(*this);
}

double
airtime_metric_measured::compute(uint32_t ignored_delta_us)
{
   valid_ = (packets_ > 0);
   if (valid_) {
      metric_ = airtime_ / static_cast<double>(packets_);
   } else {
      metric_ = 0.0;
   }
   return metric_;
}

void
airtime_metric_measured::reset()
{
   airtime_ = 0;
   packets_ = 0;
}

void
airtime_metric_measured::write(ostream& os) const
{
   os << "Airtime-Measured: " << metric_;
}
