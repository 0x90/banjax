/* -*- mode C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <metrics/iperf_metric_wrapper.hpp>

#include <dot11/data_frame.hpp>
#include <dot11/ip_hdr.hpp>
#include <dot11/llc_hdr.hpp>
#include <dot11/udp_hdr.hpp>
#include <util/exceptions.hpp>
#include <sstream>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::iperf_metric_wrapper;

iperf_metric_wrapper::iperf_metric_wrapper(metric_sptr wrapped_metric) :
   metric(),
   octets_(0),
   frames_attempted_(0),
   frames_delivered_(0),
   debug_(),
   wrapped_metric_(wrapped_metric)
{
   CHECK_NOT_NULL(wrapped_metric);
}

iperf_metric_wrapper::iperf_metric_wrapper(const iperf_metric_wrapper& other) :
   metric(other),
   octets_(other.octets_),
   frames_attempted_(other.frames_attempted_),
   frames_delivered_(other.frames_delivered_),
   debug_(other.debug_),
   wrapped_metric_(other.wrapped_metric_->clone())
{
}

iperf_metric_wrapper&
iperf_metric_wrapper::operator=(const iperf_metric_wrapper& other)
{
   if(this != &other) {
      metric::operator=(other);
      octets_ = other.octets_;
      frames_attempted_ = other.frames_attempted_;
      frames_delivered_ = other.frames_delivered_;
      debug_ = other.debug_;
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
   buffer_info_sptr info(b->info());
   data_frame_sptr df(f.as_data_frame());
   if(!(info->has(TX_FLAGS) && df))
      return;

   // is it an iperf packet?
   llc_hdr_sptr llc(df->get_llc_hdr());
   if(llc) {
      ip_hdr_sptr ip(llc->get_ip_hdr());
      if(ip) {
         udp_hdr_sptr udp(ip->get_udp_hdr());
         if(udp) {
            if(udp->dst_port() == 5001) {
               wrapped_metric_->add(b);
               return;
            }
         }
      }
   }

   // collect summary info on non-iperf packets
   const uint32_t CRC_SZ = 4;
   octets_ += b->data_size() + CRC_SZ;
   frames_attempted_ += info->has(DATA_RETRIES) ? 1 + info->data_retries() : 1;
   frames_delivered_ += info->has(TX_FLAGS) ? ((info->tx_flags() & TX_FLAGS_FAIL) ? 0 : 1) : 1;
}

iperf_metric_wrapper*
iperf_metric_wrapper::clone() const
{
   return new iperf_metric_wrapper(*this);
}

double
iperf_metric_wrapper::compute(uint64_t mactime, uint32_t delta_us)
{
#ifndef NDEBUG
   ostringstream os;
   os << ", x-octets: " << octets_;
   os << ", x-attempts: " << frames_attempted_;
   os << ", x-delivered: " << frames_delivered_;
   debug_ = os.str();
#endif
   return wrapped_metric_->compute(mactime, delta_us);
}

void
iperf_metric_wrapper::reset()
{
   octets_ = 0;
   frames_attempted_ = 0;
   frames_delivered_ = 0;
   wrapped_metric_->reset();
}

void
iperf_metric_wrapper::write(ostream& os) const
{
   wrapped_metric_->write(os);
   os << debug_;
}
