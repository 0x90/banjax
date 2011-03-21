/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2009-2011 Steve Glass
 * 
 * This file is part of banjax.
 * 
 * Banjax is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * Banjax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 */

#include <dot11/frame.hpp>
#include <dot11/control_frame.hpp>
#include <dot11/data_frame.hpp>
#include <dot11/mgmt_frame.hpp>
#include <util/dump.hpp>
#include <util/exceptions.hpp>

// #include <cstring>

using namespace dot11;
using namespace std;
using net::buffer_sptr;
using net::buffer_info_sptr;
using net::eui_48;
using std::string;
using util::dump;
using util::raise;

frame::frame(buffer_sptr buf) :
   buf_(buf)
{
   const size_t min_frame_sz = 10; // sizeof(ACK_frame)
   if(buf->data_size() < min_frame_sz) {
      ostringstream msg;
      msg << "frame too small (" << buf->data_size() << " <= " << min_frame_sz << " octets)";
      msg << hex << setw(2) << setfill('0') << dump(buf->data_size(), buf->data()) << endl;
      raise<length_error>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
   }
}

frame::~frame()
{
}

frame_control
frame::fc() const
{
   return frame_control(buf_->read_u16_le(0));
}

void
frame::fc(frame_control fc)
{
   buf_->write_u16_le(0, fc);
}

uint16_t
frame::duration() const
{
   return buf_->read_u16_le(2);
}

void
frame::duration(uint16_t d)
{
   buf_->write_u16_le(2, d);
}


eui_48
frame::address1() const 
{
   return buf_->read_mac(4);
}

void
frame::address1(eui_48 mac)
{
   buf_->write_mac(4, mac);
}

bool
frame::has_address2() const
{
   return(16 <= buf_->data_size());
}

eui_48
frame::address2() const 
{
   return buf_->read_mac(10);
}

void
frame::address2(eui_48 mac)
{
   buf_->write_mac(10, mac);
}

bool
frame::has_address3() const
{
   return(24 <= buf_->data_size());
}

eui_48
frame::address3() const 
{
   return buf_->read_mac(16);
}

void
frame::address3(eui_48 mac)
{
   buf_->write_mac(16, mac);
}

sequence_control
frame::sc() const
{
   return sequence_control(buf_->read_u16_le(22));
}

void
frame::sc(sequence_control s)
{
   buf_->write_u16(22, s);
}

bool
frame::has_address4() const
{
   return fc().to_ds() && fc().from_ds();
}

eui_48
frame::address4() const 
{
   PRECONDITION(has_address4())
   return buf_->read_mac(24);
}

void
frame::address4(eui_48 mac)
{
   PRECONDITION(has_address4());
   buf_->write_mac(24, mac);
}

control_frame_sptr
frame::as_control_frame()
{
   control_frame_sptr ctrl;
   if(CTRL_FRAME == fc().type()) {
      ctrl = control_frame_sptr(new control_frame(buf_));
   }
   return ctrl;
}

data_frame_sptr
frame::as_data_frame()
{
   data_frame_sptr data;
   if(DATA_FRAME == fc().type()) {
      data = data_frame_sptr(new data_frame(buf_));
   }
   return data;
}

mgmt_frame_sptr
frame::as_mgmt_frame()
{
   mgmt_frame_sptr mgmt;
   if(MGMT_FRAME == fc().type()) {
      mgmt = mgmt_frame_sptr(new mgmt_frame(buf_));
   }
   return mgmt;
}
