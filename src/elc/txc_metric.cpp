/* -*- mode C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <txc_metric.hpp>

#include <dot11/frame.hpp>

#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdlib.h>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::txc_metric;

txc_metric::txc_metric(string name, bool use_all_packets) :
   abstract_metric(),
   name_(name),
   use_all_packets_(use_all_packets),
   txc_(0.0),
   frames_delivered_(0),
   frame_transmissions_(0),
   max_txc_(0),
   max_txc_stash_(0)
{
}

txc_metric::txc_metric(const txc_metric& other) :
   abstract_metric(other),
   name_(other.name_),
   use_all_packets_(other.use_all_packets_),
   txc_(other.txc_),
   frames_delivered_(other.frames_delivered_),
   frame_transmissions_(other.frame_transmissions_),
   max_txc_(other.max_txc_),
   max_txc_stash_(other.max_txc_stash_)
{
}

txc_metric&
txc_metric::operator=(const txc_metric& other)
{
   if(this != &other) {
      abstract_metric::operator=(other);
      name_ = other.name_;
      use_all_packets_ = other.use_all_packets_;
      txc_ = other.txc_;
      frames_delivered_ = other.frames_delivered_;
      frame_transmissions_ = other.frame_transmissions_;
      max_txc_ = other.max_txc_;
      max_txc_stash_ = other.max_txc_stash_;
   }
   return *this;
}

txc_metric::~txc_metric()
{
}

void
txc_metric::add(buffer_sptr b)
{
   frame f(b);
   frame_control fc(f.fc());
   buffer_info_sptr info(b->info());
   if(DATA_FRAME == fc.type() && info->has(TX_FLAGS)) {
      bool tx_success = !(info->tx_flags() & TX_FLAGS_FAIL);
      if(tx_success || use_all_packets_) {
         uint8_t txc = info->has(DATA_RETRIES) ? 1 + info->data_retries() : 1;
         max_txc_ = max(max_txc_, txc);
         ++frames_delivered_;
         frame_transmissions_ += txc;
      }
   }
}

txc_metric*
txc_metric::clone() const
{
   return new txc_metric(*this);
}

double
txc_metric::compute(uint32_t junk)
{
   txc_ = frame_transmissions_ / static_cast<double>(frames_delivered_);
   max_txc_stash_ = max_txc_;
   return txc_;
}

void
txc_metric::reset()
{
   frames_delivered_ = 0;
   frame_transmissions_ = 0;
   max_txc_ = 0;
}

void
txc_metric::write(ostream& os) const
{
   os << "Max-" << name_ << ": " << max_txc_stash_ << ", ";
   os << name_ << ": " << txc_;
}
