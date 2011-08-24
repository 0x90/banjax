/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#include <etx_metric.hpp>
#include <dot11/data_frame.hpp>
#include <dot11/frame.hpp>
#include <dot11/ip_hdr.hpp>
#include <dot11/llc_hdr.hpp>
#include <dot11/udp_hdr.hpp>

#include <iostream>
#include <iomanip>


using namespace dot11;
using namespace net;
using namespace std;
using metrics::etx_metric;

etx_metric::etx_metric(uint16_t probe_port) :
   probe_addr_(0),
   probe_port_(probe_port),
   tx_frames_(0),
   tx_success_(0)
{
}

etx_metric::etx_metric(const etx_metric& other) :
   probe_addr_(other.probe_addr_),
   probe_port_(other.probe_port_),
   tx_frames_(other.tx_frames_),
   tx_success_(other.tx_success_)
{
}

etx_metric&
etx_metric::operator=(const etx_metric& other)
{
   if(&other != this) {
      probe_addr_ = other.probe_addr_;
      probe_port_ = other.probe_port_;
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
   data_frame_sptr df(f.as_data_frame());
   if(df) {

      // ignore non-probe traffic
      llc_hdr_sptr llc(df->get_llc_hdr());
      if(!llc)
         return;
      ip_hdr_sptr ip(llc->get_ip_hdr());
      if(!ip)
         return;
      udp_hdr_sptr udp(ip->get_udp_hdr());
      if(!udp)
         return;


      buffer_info_sptr info(b->info());
      if(info->has(TX_FLAGS)) {

         if(udp->src_port() == probe_port_) {
#if 0
            // ToDo: remember IP address of outgoing frames
            // gather data for forward delivery ratio
            uint8_t txc = 1 + info->data_retries();
            tx_frames_ += txc;
            uint32_t tx_flags = info->tx_flags();
            if(tx_flags & TX_FLAGS_FAIL) {
               ++tx_success_;
            }
#endif
         }
      
      } else {

         if(udp->dst_port() == probe_port_) {
            // ToDo:
            // gather data for reverse delivery ratio - we should know our IP address by now
         }

      }
   }
}

etx_metric*
etx_metric::clone() const
{
   return new etx_metric(*this);
}

void
etx_metric::compute(uint32_t delta_us)
{
   double d_f = 1.0; // ToDo: fix me!
   double d_r = 1.0; // ToDo: fix me!
   // stash value(/* 1.0 / */ d_f * d_r);

   // advance the probe window
}

void
etx_metric::reset()
{
}

void
etx_metric::write(ostream& os) const
{
   // os << "ETX: " << value();
}
