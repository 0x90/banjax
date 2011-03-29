/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2010-2011 Steve Glass
 * 
 * This file is part of banjax.
 * 
 * Banjax is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * Banjax is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 */

#include <net/buffer_info.hpp>
#include <util/exceptions.hpp>
#include <iostream>
#include <iomanip>

using namespace net;
using namespace std;

buffer_info::buffer_info() :
   present_(0)
{
}

buffer_info::~buffer_info()
{
}

void
buffer_info::clear(property_t props)
{
   present_ &= ~props;
}

value_t
buffer_info::get(property_t prop) const
{
   return props_[bit(prop)];
}

bool
buffer_info::has(property_t props) const
{
   return((present_ & props) == props);
}

void
buffer_info::set(property_t prop, value_t value)
{
   uint8_t n = bit(prop);
   props_[n] = value;
   present_ |= prop;
}

void
buffer_info::write(ostream& os) const
{
   if(has(TIMESTAMP1))
      os << "TIMESTAMP1: " << get(TIMESTAMP1) << ", ";
   if(has(TIMESTAMP2))
      os << "TIMESTAMP2: " << get(TIMESTAMP2) << ", ";
   if(has(RATE_Kbs))
      os << "RATE_Kbs: " << get(RATE_Kbs) << ", ";
   if(has(FREQ_MHz))
      os << "FREQ_MHz: " << get(FREQ_MHz) << ", ";
   if(has(SIGNAL_dBm))
      os << "SIGNAL_dBm: " << static_cast<int16_t>(static_cast<int8_t>(get(SIGNAL_dBm))) << ", ";
   if(has(RETRIES))
      os << "RETRIES: " << get(RETRIES) << ", ";

   if(has(RXFLAGS)) {
      uint64_t flags = get(RXFLAGS);
      os << "FLAGS:";
      char sep = ' ';
      if(flags & RXFLAGS_PREAMBLE_LONG) {
         os << sep << "RXFLAGS_PREAMBLE_LONG";
         sep = '|';
      }
      if(flags & RXFLAGS_PREAMBLE_SHORT) {
         os << sep << "RXFLAGS_PREAMBLE_SHORT";
         sep = '|';
      }
      if(flags & RXFLAGS_CODING_DSSS) {
         os << sep << "RXFLAGS_CODING_DSSS";
         sep = '|';
      }
      if(flags & RXFLAGS_CODING_OFDM) {
         os << sep << "RXFLAGS_CODING_OFDM";
         sep = '|';
      }
      if(flags & RXFLAGS_CODING_FHSS) {
         os << sep << "RXFLAGS_CODING_FHSS";
         sep = '|';
      }
      if(flags & RXFLAGS_CODING_DYNAMIC) {
         os << sep << "RXFLAGS_CODING_DYNAMIC";
         sep = '|';
      }
      if(flags & RXFLAGS_RATE_FULL) {
         os << sep << "RXFLAGS_RATE_FULL";
         sep = '|';
      }
      if(flags & RXFLAGS_RATE_HALF) {
         os << sep << "RXFLAGS_RATE_HALF";
         sep = '|';
      }
      if(flags & RXFLAGS_RATE_QUARTER) {
         os << sep << "RXFLAGS_RATE_QUARTER";
         sep = '|';
      }
      if(flags & RXFLAGS_BAD_FCS) {
         os << sep << "RXFLAGS_BAD_FCS";
         sep = '|';
      }
   }
}

uint8_t
buffer_info::bit(property_t prop) const
{
   CHECK_NOT_EQUAL(prop, 0);
   CHECK_EQUAL((prop & (-prop)), prop);
   for(uint8_t i = 0; i < NOF_PROPS; ++i) {
      prop >>= 1;
      if(prop == 0)
         return i;
   }
}

ostream&
net::operator<<(ostream& os, const buffer_info& info)
{
   info.write(os);
   return os;
}
