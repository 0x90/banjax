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

#include <dot11/data_frame.hpp>
#include <util/dump.hpp>
#include <util/exceptions.hpp>

using namespace dot11;
using namespace std;
using net::buffer_sptr;
using util::dump;
using util::raise;

data_frame::data_frame(buffer_sptr buf) :
   frame(buf)
{
   const size_t min_data_frame_sz = 24;
   if(buf->data_size() < min_data_frame_sz) {
      ostringstream msg;
      msg << "data frame too small (" << buf->data_size() << " <= " << min_data_frame_sz << " octets)";
      msg << hex << setw(2) << setfill('0') << dump(buf->data_size(), buf->data()) << endl;
      raise<invalid_argument>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
   }
}

data_frame::~data_frame()
{
}

#if 0

ccmp_mpdu_sptr
data_frame::ccmp_data()
{
   ccmp_mpdu_sptr data;
   if(fc().protected_frame()) {
      ccmp_mpdu_sptr mpdu(new ccmp_mpdu(*this, mpdu_offset(), size()));
      if(mpdu->ext_iv()) {
         // ToDo: TKIP/CCMP disambiguation
         // mpdu_hdr[2] == 0?
         data = mpdu;
      }
   }
   return data;
}

llc_pdu_sptr
data_frame::llc_data()
{
   llc_pdu_sptr data;
   if(!fc().protected_frame()) {
      llc_pdu_sptr mpdu(new llc_pdu(*this, mpdu_offset(), size()));
      data = mpdu;
   }
   return data;
}

tkip_mpdu_sptr
data_frame::tkip_data()
{
   tkip_mpdu_sptr data;
   if(fc().protected_frame()) {
      tkip_mpdu_sptr mpdu(new tkip_mpdu(*this, mpdu_offset(), size()));
      if(mpdu->ext_iv() && mpdu->seed() == ((mpdu->tsc_octet(1) | 0x20) & 0x7f)) {
         data = mpdu;
      }
   }
   return data;
}

wep_mpdu_sptr
data_frame::wep_data()
{
   wep_mpdu_sptr data;
   if(fc().protected_frame()) {
      wep_mpdu_sptr mpdu(new wep_mpdu(*this, mpdu_offset(), size()));
      if(!mpdu->ext_iv()) {
         data = mpdu;
      }
   }
   return data;
}

#endif

size_t
data_frame::mpdu_offset() const
{
   // ToDo: account for 802.11e QoS field, if present
   size_t data_ofs = 24;
   if(has_address4())
      data_ofs += 6;
   return data_ofs;
}
