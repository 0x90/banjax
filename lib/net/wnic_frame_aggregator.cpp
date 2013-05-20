/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2013 NICTA
 * 
 */

#include <net/wnic_frame_aggregator.hpp>
#include <dot11/data_frame.hpp>
#include <dot11/frame.hpp>
#include <dot11/frame_subtype.hpp>
#include <dot11/sequence_control.hpp>

#include <iostream>

using namespace net;
using net::buffer_sptr;

wnic_frame_aggregator::wnic_frame_aggregator(wnic_sptr w, const eui_48& ta, bool greedy) :
   wnic_wrapper(w),
   ta_(ta),
   greedy_(greedy),
   seq_no_(0),
   first_(),
   last_()
{
}

wnic_frame_aggregator::~wnic_frame_aggregator()
{
}

buffer_sptr
wnic_frame_aggregator::read()
{
   buffer_sptr b;
/*
   while(b = wnic_wrapper::read()) {
      frame f(b);
      if((f.fc().type() == DATA_FRAME) && (f.address2() == ta) && is_iperf_frame(f)) {
         sequence_control sc = f.sc();
         if(first) {
            if(sc.sequence_no() == seq_no) {
               last = b;
            } else {
               uint64_t ts = first->info()->timestamp1();
               first = last = b;
               seq_no = sc.sequence_no();
            }
         } else {
            first = last = b;
            seq_no = sc.sequence_no();
         }
      } else if(greedy || (f.subtype() == CTRL_ACK && f.address1() == ta_)) {
         last = b;
      } else {
         b = first;
         first.reset();
         last.reset();
      }
   }
*/
   return b;
}
/*
bool
wnic_frame_aggregator::is_iperf_frame(frame& f)
{
   data_frame_sptr df(f.as_data_frame());
   if(!df)
      return false;
   llc_hdr_sptr llc(df->get_llc_hdr());
   if(!llc)
      return false;
   ip_hdr_sptr ip(llc->get_ip_hdr());
   if(!ip)
      return false;
   udp_hdr_sptr udp(ip->get_udp_hdr());
   if(!udp)
      return false;
   if(udp->dst_port() != 5001)
      return false;
   return true;
}
*/
