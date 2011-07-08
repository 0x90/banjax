/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS

#include <utilization_metric.hpp>
#include <dot11/frame.hpp>

#include <iostream>
#include <iomanip>
#include <math.h>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::utilization_metric;

utilization_metric::utilization_metric() :
   packet_octets_(0)
{
}

utilization_metric::utilization_metric(const utilization_metric& other) :
   packet_octets_(0)
{
}

utilization_metric&
utilization_metric::operator=(const utilization_metric& other)
{
   if(this != &other) {
      packet_octets_ = other.packet_octets_;
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
   frame_control fc(f.fc());
   buffer_info_sptr info(b->info());
   if(DATA_FRAME == fc.type() && info->has(TX_FLAGS) && 0 == (info->tx_flags() & TX_FLAGS_FAIL)) {
      const uint16_t HDR_SZ = 62;
      const uint16_t FRAME_SZ = b->data_size();
      if(HDR_SZ < FRAME_SZ) {
         packet_octets_ += FRAME_SZ - HDR_SZ;
      }
   }
}

utilization_metric*
utilization_metric::clone() const
{
   return new utilization_metric(*this);
}

double
utilization_metric::metric() const
{
   return packet_octets_ / 1e6;
}

void
utilization_metric::reset()
{
   packet_octets_ = 0;
}

void
utilization_metric::write(ostream& os) const
{
   os << "utilization: " << metric();
}
