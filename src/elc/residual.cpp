/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS

#include <residual.hpp>
#include <dot11/data_frame.hpp>

#include <iostream>
#include <iomanip>
#include <math.h>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::residual;

residual::residual(string name, metric_sptr m) :
   metric(),
   m_(m),
   name_(name),
   busy_time_(0),
   residual_(0.0)
{
}

residual::residual(const residual& other) :
   metric(other),
   m_(other.m_),
   name_(other.name_),
   busy_time_(other.busy_time_),
   residual_(other.residual_)
{
}

residual&
residual::operator=(const residual& other)
{
   if(this != &other) {
      metric::operator=(other);
      m_ = other.m_;
      name_ = other.name_;
      busy_time_ = other.busy_time_;
      residual_ = other.residual_;
   }
   return *this;
}

residual::~residual()
{
}

void
residual::add(buffer_sptr b)
{
   buffer_info_sptr info(b->info());
   encoding_sptr enc(info->channel_encoding());

   const size_t CRC_SZ = 4;
   const uint16_t RATE_Kbs = info->rate_Kbs();
   const size_t FRAME_SZ = b->data_size() + CRC_SZ;
   const bool PREAMBLE = false; // ToDo: get from encoding

   if(info->has(TX_FLAGS)) {
      uint16_t txc = 1 + (info->has(DATA_RETRIES) ? info->data_retries() : 0);
      busy_time_ += txc * enc->txtime(FRAME_SZ, RATE_Kbs, PREAMBLE);
   } else {
      busy_time_ += enc->txtime(FRAME_SZ, RATE_Kbs, PREAMBLE);
   }
}

residual*
residual::clone() const
{
   return new residual(*this);
}

double
residual::compute(uint32_t delta_us)
{
   double idle_fraction = static_cast<double>(delta_us - busy_time_) / delta_us;
   residual_ = m_->compute(delta_us) * idle_fraction;
   return residual_;
}

void
residual::reset()
{
   m_->reset();
   busy_time_ = 0;
}

void
residual::write(ostream& os) const
{
   os << name_ << ": " << residual_;
}
