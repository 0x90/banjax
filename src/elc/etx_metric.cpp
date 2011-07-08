/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#include <etx_metric.hpp>
#include <dot11/frame.hpp>

#include <iostream>
#include <iomanip>


using namespace dot11;
using namespace net;
using namespace std;
using metrics::etx_metric;

etx_metric::etx_metric() :
   tx_frames_(0),
   tx_success_(0)
{
}

etx_metric::etx_metric(const etx_metric& other) :
   tx_frames_(other.tx_frames_),
   tx_success_(other.tx_success_)
{
}

etx_metric&
etx_metric::operator=(const etx_metric& other)
{
   if(&other != this) {
      tx_frames_ = other.tx_frames_;
      tx_success_ = other.tx_success_;
   }
   return *this;
}

etx_metric::~etx_metric()
{
}

void
etx_metric::add(buffer_sptr b)
{
   frame f(b);
   frame_control fc(f.fc());
   if(fc.type() == DATA_FRAME) {
      buffer_info_sptr info(b->info());
      if(info->has(TX_FLAGS)) {
         // gather data for forward delivery ratio
         uint8_t txc = 1 + info->data_retries();
         tx_frames_ += txc;
         uint32_t tx_flags = info->tx_flags();
         if(tx_flags & TX_FLAGS_FAIL) {
            ++tx_success_;
         }
      } else {
         // gather data for reverse delivery ratio
      }
   }
}

etx_metric*
etx_metric::clone() const
{
   return new etx_metric(*this);
}

double
etx_metric::metric() const
{
   double d_f = static_cast<double>(tx_frames_) / static_cast<double>(tx_success_);
   double d_r = 1.0; // ToDo: fix me!
   return /* 1.0 / */ d_f * d_r;
}

void
etx_metric::reset()
{
}

void
etx_metric::write(ostream& os) const
{
   os << "ETX: " << metric();
}
