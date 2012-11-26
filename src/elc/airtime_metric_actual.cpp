/* -*- mode C++; tab-width: 3; -*- */

/*
 * Copyright 2011-2012 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS

#include <airtime_metric_actual.hpp>

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
using metrics::airtime_metric_actual;

airtime_metric_actual::airtime_metric_actual() :
   abstract_metric(),
   airtime_sum_(0),
   packets_(0),
   metric_(0.0)
{
}

airtime_metric_actual::airtime_metric_actual(const airtime_metric_actual& other) :
   abstract_metric(other),
   airtime_sum_(other.airtime_sum_),
   packets_(other.packets_),
   metric_(other.metric_)
{
}

airtime_metric_actual&
airtime_metric_actual::operator=(const airtime_metric_actual& other)
{
   if(this != &other) {
      abstract_metric::operator=(other);
      airtime_sum_ = other.airtime_sum_;
      packets_ = other.packets_;
      metric_ = other.metric_;
   }
   return *this;
}

airtime_metric_actual::~airtime_metric_actual()
{
}

void
airtime_metric_actual::add(buffer_sptr b)
{
   frame f(b);
   frame_control fc(f.fc());
   buffer_info_sptr info(b->info());
   if(DATA_FRAME == fc.type() && info->has(TX_FLAGS)) {
      bool tx_success = (0 == (info->tx_flags() & TX_FLAGS_FAIL));
      airtime_sum_ += info->metric();
      ++packets_;
   }
}

airtime_metric_actual*
airtime_metric_actual::clone() const
{
   return new airtime_metric_actual(*this);
}

double
airtime_metric_actual::compute(uint32_t ignored_delta_us)
{
   if(packets_) {
      metric_ = airtime_sum_ / static_cast<double>(packets_);
   } else {
      metric_ = 0;
   }
   return metric_;
}

void
airtime_metric_actual::reset()
{
   airtime_sum_ = 0;
   packets_ = 0;
}

void
airtime_metric_actual::write(ostream& os) const
{
   os << "Airtime-actual: " << metric_;
}
