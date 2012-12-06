/* -*- mode C++; tab-width: 3; -*- */

/*
 * Copyright 2011-2012 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS

#include <airtime_metric_kernel.hpp>

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
using metrics::airtime_metric_kernel;

airtime_metric_kernel::airtime_metric_kernel() :
   abstract_metric(),
   airtime_sum_(0),
   last_info_(),
   metric_(0.0),
   packets_(0)
{
}

airtime_metric_kernel::airtime_metric_kernel(const airtime_metric_kernel& other) :
   abstract_metric(other),
   airtime_sum_(other.airtime_sum_),
   last_info_(other.last_info_),
   metric_(other.metric_),
   packets_(other.packets_)
{
}

airtime_metric_kernel&
airtime_metric_kernel::operator=(const airtime_metric_kernel& other)
{
   if(this != &other) {
      abstract_metric::operator=(other);
      airtime_sum_ = other.airtime_sum_;
      last_info_ = other.last_info_;
      metric_ = other.metric_;
      packets_ = other.packets_;
   }
   return *this;
}

airtime_metric_kernel::~airtime_metric_kernel()
{
}

void
airtime_metric_kernel::add(buffer_sptr b)
{
   frame f(b);
   frame_control fc(f.fc());
   buffer_info_sptr info(b->info());
   if(DATA_FRAME == fc.type() && info->has(TX_FLAGS)) {
      bool tx_success = (0 == (info->tx_flags() & TX_FLAGS_FAIL));
      airtime_sum_ += info->metric();
      ++packets_;
      last_info_ = info;
   }
}

airtime_metric_kernel*
airtime_metric_kernel::clone() const
{
   return new airtime_metric_kernel(*this);
}

double
airtime_metric_kernel::compute(uint32_t ignored_delta_us)
{
   if(packets_) {
      metric_ = airtime_sum_ / static_cast<double>(packets_);
   } else {
      metric_ = 0;
   }
   return metric_;
}

void
airtime_metric_kernel::reset()
{
   airtime_sum_ = 0;
   packets_ = 0;
   buffer_info_sptr null;
   last_info_ = null;
}

void
airtime_metric_kernel::write(ostream& os) const
{
   os << "Airtime-Kernel-Avg: " << metric_ << ", ";
   os << "Airtime-Kernel: ";
   if(last_info_)
      os << last_info_->metric();
   else
      os << "N/A";
}
