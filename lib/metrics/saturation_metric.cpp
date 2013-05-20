/* -*- mode C++; tab-width: 3; -*- */

/*
 * Copyright 2013 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <metrics/saturation_metric.hpp>

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
using metrics::saturation_metric;

saturation_metric::saturation_metric(const string& name) :
   metric(),
   name_(name),
   rx_packets_(0),
   rx_time_(0),
   tx_packets_(0),
   tx_time_(0),
   bad_packets_(0),
   saturation_(0.0),
   valid_(false),
   debug_()
{
}

saturation_metric::saturation_metric(const saturation_metric& other) :
   metric(other),
   name_(other.name_),
   rx_packets_(other.rx_packets_),
   rx_time_(other.rx_time_),
   tx_packets_(other.tx_packets_),
   tx_time_(other.tx_time_),
   bad_packets_(other.bad_packets_),
   saturation_(other.saturation_),
   valid_(other.valid_),
   debug_(other.debug_)
{
}

saturation_metric&
saturation_metric::operator=(const saturation_metric& other)
{
   if(this != &other) {
      metric::operator=(other);
      name_ = other.name_;
      rx_packets_ = other.rx_packets_;
      rx_time_ = other.rx_time_;
      tx_packets_ = other.tx_packets_;
      tx_time_ = other.tx_time_;
      bad_packets_ = other.bad_packets_;
      saturation_ = other.saturation_;
      valid_ = other.valid_;
      debug_ = other.debug_;
   }
   return *this;
}

saturation_metric::~saturation_metric()
{
}

void
saturation_metric::add(buffer_sptr b)
{
   buffer_info_sptr info(b->info());
   if(info->has(RX_FLAGS)) {
      rx_packets_++;
      rx_time_ += info->packet_time();
   } else if(info->has(TX_FLAGS)) {
      tx_packets_++;
      tx_time_ += info->packet_time();
   } else {
      bad_packets_++;
   }
}

saturation_metric*
saturation_metric::clone() const
{
   return new saturation_metric(*this);
}

double
saturation_metric::compute(uint64_t time, uint32_t delta_us)
{
   valid_ = rx_packets_ + tx_packets_;
   saturation_ = (static_cast<double>(tx_time_ + rx_time_) / delta_us) * 100.0;
#ifndef NDEBUG
   ostringstream os;
   os << ", " << name_ << "-tx: " << (static_cast<double>(tx_time_) / delta_us) * 100.0;
   os << ", " << name_ << "-bad-packets: " << bad_packets_;
   os << ", " << name_ << "-good-packets: " << rx_packets_ + tx_packets_;
   os << ", " << name_ << "-rx-packets: " << rx_packets_;
   os << ", " << name_ << "-rx-time: " << rx_time_;
   os << ", " << name_ << "-tx-packets: " << tx_packets_;
   os << ", " << name_ << "-tx-time: " << tx_time_;
   debug_ = os.str();
#endif
   return saturation_;
}

void
saturation_metric::reset()
{
   rx_packets_ = 0;
   rx_time_ = 0;
   tx_packets_ = 0;
   tx_time_ = 0;
   bad_packets_ = 0;
   saturation_ = 0;
   valid_ = false;
}

void
saturation_metric::write(ostream& os) const
{
   if(valid_) {
      os << name_ << ": " << saturation_;
   } else {
      os << name_ << ": - ";
   }
   os << debug_;
}
