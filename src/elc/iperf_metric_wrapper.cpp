/* -*- mode C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <iperf_metric_wrapper.hpp>

#include <dot11/data_frame.hpp>
#include <dot11/ip_hdr.hpp>
#include <dot11/llc_hdr.hpp>
#include <dot11/udp_hdr.hpp>
#include <util/exceptions.hpp>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::iperf_metric_wrapper;

iperf_metric_wrapper::iperf_metric_wrapper(metric_sptr wrapped_metric) :
   metric(),
   wrapped_metric_(wrapped_metric)
{
   CHECK_NOT_NULL(wrapped_metric);
}

iperf_metric_wrapper::iperf_metric_wrapper(const iperf_metric_wrapper& other) :
   metric(other),
   wrapped_metric_(other.wrapped_metric_->clone())
{
}

iperf_metric_wrapper&
iperf_metric_wrapper::operator=(const iperf_metric_wrapper& other)
{
   if(this != &other) {
      metric::operator=(other);
      wrapped_metric_ = metric_sptr(other.wrapped_metric_->clone());
   }
   return *this;
}

iperf_metric_wrapper::~iperf_metric_wrapper()
{
}

void
iperf_metric_wrapper::add(buffer_sptr b)
{
   frame f(b);
   data_frame_sptr df(f.as_data_frame());
   if(df) {
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

      wrapped_metric_->add(b);
   }
}

iperf_metric_wrapper*
iperf_metric_wrapper::clone() const
{
   return new iperf_metric_wrapper(*this);
}

double
iperf_metric_wrapper::compute(uint64_t mactime, uint32_t delta_us)
{
   return wrapped_metric_->compute(mactime, delta_us);
}

void
iperf_metric_wrapper::reset()
{
   wrapped_metric_->reset();
}

void
iperf_metric_wrapper::write(ostream& os) const
{
   wrapped_metric_->write(os);
}
