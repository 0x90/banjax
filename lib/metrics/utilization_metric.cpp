/* -*- mode C++; tab-width: 3; -*- */

/*
 * Copyright 2013 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <metrics/utilization_metric.hpp>

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
using metrics::utilization_metric;

utilization_metric::utilization_metric(const string& name) :
   metric(),
   name_(name),
   ctrl_(0),
   mgmt_(0),
   rx_(0),
   tx_(0),
   bad_(0),
   time_(0),
   utilization_(0.0),
   valid_(false),
   debug_()
{
}

utilization_metric::utilization_metric(const utilization_metric& other) :
   metric(other),
   name_(other.name_),
   ctrl_(other.ctrl_),
   mgmt_(other.mgmt_),
   rx_(other.rx_),
   tx_(other.tx_),
   bad_(other.bad_),
   time_(other.time_),
   utilization_(other.utilization_),
   valid_(other.valid_),
   debug_(other.debug_)
{
}

utilization_metric&
utilization_metric::operator=(const utilization_metric& other)
{
   if(this != &other) {
      metric::operator=(other);
      name_ = other.name_;
      ctrl_ = other.ctrl_;
      mgmt_ = other.mgmt_;
      rx_ = other.rx_;
      tx_ = other.tx_;
      bad_ = other.bad_;
      time_ = other.time_;
      utilization_ = other.utilization_;
      valid_ = other.valid_;
      debug_ = other.debug_;
   }
   return *this;
}

utilization_metric::~utilization_metric()
{
}

void
utilization_metric::add(buffer_sptr b)
{
   frame f(b);
   buffer_info_sptr info(b->info());
   encoding_sptr enc(info->channel_encoding());

   switch(f.fc().type()) {
   case CTRL_FRAME:
      ctrl_++;
      break;
   case MGMT_FRAME:
      mgmt_++;
      break;
   case DATA_FRAME:
	   {
         const uint16_t AIFS = 43;
         const uint16_t CW = 68;
         const uint16_t SIFS = 16;
         const uint16_t ACK_SZ = 14;
         const uint32_t DATA_RATE = info->rate_Kbs();
         const uint32_t ACK_RATE = enc->response_rate(DATA_RATE);
         const uint32_t ACK = enc->txtime(ACK_SZ, ACK_RATE, false);
         time_ += AIFS + CW + b->info()->packet_time() + SIFS + ACK;
         
         if(b->info()->has(TX_FLAGS)) {
            tx_++;
         } else if(b->info()->has(RX_FLAGS)) {
            rx_++;
         } else {
            bad_++;
         }
      }
      break;
   default:
      break;
   }

}

utilization_metric*
utilization_metric::clone() const
{
   return new utilization_metric(*this);
}

double
utilization_metric::compute(uint64_t time, uint32_t delta_us)
{
   valid_ = rx_ + tx_;
   utilization_ = (static_cast<double>(time_) / delta_us) * 100.0;
#ifndef NDEBUG
   ostringstream os;
   os << ", " << name_ << "-ctrl: " << ctrl_;
   os << ", " << name_ << "-mgmt: " << mgmt_;
   os << ", " << name_ << "-tx: " << tx_;
   os << ", " << name_ << "-rx: " << rx_;
   os << ", " << name_ << "-time: " << time_;
   os << ", " << name_ << "-unknown: " << bad_;
   debug_ = os.str();
#endif
   return utilization_;
}

void
utilization_metric::reset()
{
   ctrl_ = 0;
   mgmt_ = 0;
   rx_ = 0;
   tx_ = 0;
   bad_ = 0;
   time_ = 0;
   utilization_ = 0;
   valid_ = false;
}

void
utilization_metric::write(ostream& os) const
{
   if(valid_) {
      os << name_ << ": " << utilization_;
   } else {
      os << name_ << ": - ";
   }
   os << debug_;
}
