/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#include <legacy_elc_metric.hpp>

#include <dot11/frame.hpp>

#include <iostream>
#include <iomanip>
#include <math.h>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::legacy_elc_metric;

legacy_elc_metric::legacy_elc_metric() :
   frames_(0),
   packets_(0)
{
}

legacy_elc_metric::legacy_elc_metric(const legacy_elc_metric& other) :
   frames_(other.frames_),
   packets_(other.packets_)
{
}

legacy_elc_metric&
legacy_elc_metric::operator=(const legacy_elc_metric& other)
{
   if(this != &other) {
      frames_ = other.frames_;
      packets_ = other.packets_;
   }
   return *this;
}

legacy_elc_metric::~legacy_elc_metric()
{
}

void
legacy_elc_metric::add(buffer_sptr b)
{
   frame f(b);
   frame_control fc(f.fc());
   buffer_info_sptr info(b->info());
   if(DATA_FRAME == fc.type() && info->has(TX_FLAGS)) {
      bool tx_success = (0 == (info->tx_flags() & TX_FLAGS_FAIL));
      if(tx_success) {
         ++packets_;
         const uint16_t IP_OVERHEAD = 62;
         packet_octets_ += b->data_size() - IP_OVERHEAD;
         rates_Kbs_ += info->rate_Kbs();
      }
      frames_ += info->has(DATA_RETRIES) ? 1 + info->data_retries() : 1;
   }
}

legacy_elc_metric*
legacy_elc_metric::clone() const
{
   return new legacy_elc_metric(*this);
}

double
legacy_elc_metric::metric() const
{
   const double PDR = static_cast<double>(packets_) / static_cast<double>(frames_);
   const double PKTS = packets_;
   const double AVG_PKT_SZ = packet_octets_ / PKTS;
   const double AVG_PKT_RATE_Kbs = rates_Kbs_ / PKTS;

   // ToDo: pick the "closest" rate from the supported rate set

   const double EMT = AVG_PKT_SZ * AVG_PKT_RATE_Kbs;
   return PDR * EMT;
}

void
legacy_elc_metric::reset()
{
   frames_ = 0;
   packets_ = 0;
}

void
legacy_elc_metric::write(ostream& os) const
{
   os << "LegacyELC: " << metric();
}
