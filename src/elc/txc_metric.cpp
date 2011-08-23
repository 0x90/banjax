/* -*- mode C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <txc_metric.hpp>

#include <dot11/frame.hpp>
#include <dot11/data_frame.hpp>

#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdlib.h>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::txc_metric;

txc_metric::txc_metric() :
   frames_(0),
   packets_(0),
   txc_(0.0)
{
}

txc_metric::txc_metric(const txc_metric& other) :
   abstract_metric(other),
   frames_(other.frames_),
   packets_(other.packets_),
   txc_(other.txc_)
{
}

txc_metric&
txc_metric::operator=(const txc_metric& other)
{
   if(this != &other) {
      abstract_metric::operator=(other);
      frames_ = other.frames_;
      packets_ = other.packets_;
      txc_ = other.txc_;
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
   buffer_info_sptr info(b->info());
   data_frame_sptr df(f.as_data_frame());
   if(info->has(TX_FLAGS) && df) {
      ++packets_;
      frames_ += info->has(DATA_RETRIES) ? 1 + info->data_retries() : 1;
   }
}

txc_metric*
txc_metric::clone() const
{
   return new txc_metric(*this);
}

void
txc_metric::compute(uint32_t junk)
{
   const double FRMS = frames_;
   const double PKTS = packets_;
   txc_ = FRMS / PKTS;

   frames_ = 0;
   packets_ = 0;
}

void
txc_metric::write(ostream& os) const
{
   os << "TXC: " << txc_;
}
