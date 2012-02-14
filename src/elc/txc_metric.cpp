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

txc_metric::txc_metric() :
   abstract_metric(),
   n_(0),
   transmissions_(0),
   txc_(0.0)
{
}

txc_metric::txc_metric(const txc_metric& other) :
   abstract_metric(other),
   n_(other.n_),
   transmissions_(other.transmissions_),
   txc_(other.txc_)
{
}

txc_metric&
txc_metric::operator=(const txc_metric& other)
{
   if(this != &other) {
      abstract_metric::operator=(other);
      n_ = other.n_;
      transmissions_ = other.transmissions_;
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
   frame_control fc(f.fc());
   buffer_info_sptr info(b->info());
   if(DATA_FRAME == fc.type() && info->has(TX_FLAGS)) {
      ++n_;
      transmissions_ += info->has(DATA_RETRIES) ? 1 + info->data_retries() : 1;
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
   txc_ = transmissions_ / static_cast<double>(n_);
   return txc_;
}

void
txc_metric::reset()
{
   n_ = 0;
   transmissions_ = 0;
}

void
txc_metric::write(ostream& os) const
{
   os << "TXC: " << txc_;
}
