/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS

#include <goodput_metric.hpp>
#include <dot11/frame.hpp>
#include <dot11/data_frame.hpp>
#include <dot11/frame.hpp>
#include <dot11/ip_hdr.hpp>
#include <dot11/llc_hdr.hpp>
#include <dot11/udp_hdr.hpp>

#include <iostream>
#include <iomanip>
#include <math.h>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::goodput_metric;

goodput_metric::goodput_metric() :
   metric(),
   frame_octets_(0),
   packet_octets_(0),
   iperf_goodput_(0),
   mac_goodput_(0.0)
{
}

goodput_metric::goodput_metric(const goodput_metric& other) :
   metric(other),
   frame_octets_(other.frame_octets_),
   packet_octets_(other.packet_octets_),
   iperf_goodput_(other.iperf_goodput_),
   mac_goodput_(other.mac_goodput_)
{
}

goodput_metric&
goodput_metric::operator=(const goodput_metric& other)
{
   if(this != &other) {
      metric::operator=(other);
      frame_octets_ = other.frame_octets_;
      packet_octets_ = other.packet_octets_;
      iperf_goodput_ = other.iperf_goodput_;
      mac_goodput_ = other.mac_goodput_;
   }
   return *this;
}

goodput_metric::~goodput_metric()
{
}

void
goodput_metric::add(buffer_sptr b)
{
   frame f(b);
   buffer_info_sptr info(b->info());
   data_frame_sptr df(f.as_data_frame());
   if(info->has(TX_FLAGS) && df) {

      // ignore non-iperf traffic
      llc_hdr_sptr llc(df->get_llc_hdr());
      if(!llc)
         return;
      ip_hdr_sptr ip(llc->get_ip_hdr());
      if(!ip)
         return;
      udp_hdr_sptr udp(ip->get_udp_hdr());
      if(!udp)
         return;
      if(udp->dst_port() != 5001)
         return;

      // update metric info
      bool failed = (info->tx_flags() & TX_FLAGS_FAIL);
      if(!failed) {
         const uint32_t IEEE80211_HDR_SZ = 26;
         const uint32_t LLC_HDR_SZ = 8;
         const uint32_t IP_HDR_SZ = 20;
         const uint32_t UDP_HDR_SZ = 8;
         const uint16_t HDR_SZ = IEEE80211_HDR_SZ + LLC_HDR_SZ + IP_HDR_SZ + UDP_HDR_SZ;
         const uint16_t FRAME_SZ = b->data_size();

         packet_octets_ += FRAME_SZ - HDR_SZ;
         const uint32_t CRC_SZ = 4;
         frame_octets_ += FRAME_SZ + CRC_SZ;
      }

   }
}

goodput_metric*
goodput_metric::clone() const
{
   return new goodput_metric(*this);
}

void
goodput_metric::compute(uint32_t delta_us)
{
   double delta = delta_us;
   iperf_goodput_ = packet_octets_ / delta;
   mac_goodput_ = frame_octets_ / delta;
}

void
goodput_metric::reset()
{
   frame_octets_ = 0;
   packet_octets_ = 0;
}

void
goodput_metric::write(ostream& os) const
{
   os << "goodput: " << mac_goodput_ << ", iperf: " << iperf_goodput_;
}
