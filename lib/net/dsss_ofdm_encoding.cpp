/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 Steve Glass
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

#include <net/dsss_ofdm_encoding.hpp>
#include <util/exceptions.hpp>

#include <iostream>
#include <math.h>
#include <sstream>

using namespace net;
using namespace std;
using util::raise;

encoding_sptr
dsss_ofdm_encoding::get()
{
   static encoding_sptr enc(new dsss_ofdm_encoding);
   return enc;
}

dsss_ofdm_encoding::~dsss_ofdm_encoding()
{
}

uint16_t
dsss_ofdm_encoding::CWMIN() const
{
   return 31;
}

uint16_t
dsss_ofdm_encoding::CWMAX() const
{
   return 1023;
}

uint16_t
dsss_ofdm_encoding::SIFS() const
{
   return 10;
}

uint16_t
dsss_ofdm_encoding::slot_time() const
{
   return 20; // Note: could also be 9 for short slot times
}

uint16_t
dsss_ofdm_encoding::txtime(uint16_t frame_sz, uint32_t rate_Kbs, bool has_short_preamble) const
{
   const float NDBPS = ndbps(rate_Kbs);
   const uint16_t PREAMBLE_DSSS = has_short_preamble ? 72 : 144;
   const uint16_t PLCP_DSSS = has_short_preamble ? 24 : 48;
   const uint16_t PREAMBLE_OFDM = 8;
   const uint16_t PLCP_OFDM = 4;
   const uint8_t PLCP_SVC_BITS = 16;
   const uint8_t PAD_BITS = 6;
   const uint16_t SIGNAL_EXT = 6;
   return PREAMBLE_DSSS + PLCP_DSSS + PREAMBLE_OFDM + PLCP_OFDM + 4 * ceill((PLCP_SVC_BITS + (8 * frame_sz) + PAD_BITS) / NDBPS) + SIGNAL_EXT;
}

void
dsss_ofdm_encoding::write(ostream& os) const
{
   os << "DSSS/OFDM";
}

dsss_ofdm_encoding::dsss_ofdm_encoding()
{
}

uint16_t
dsss_ofdm_encoding::ndbps(uint32_t rate_Kbs) const
{
   const uint32_t RATE_Mbs = rate_Kbs / 1000;
   static const uint8_t RATE_NSYMS[][2] = {
      {  6,  24 },
      {  9,  36 },
      { 12,  48 },
      { 18,  72 },
      { 24,  96 },
      { 36, 144 },
      { 48, 192 },
      { 54, 216 }
   };
   static const size_t NOF_RATE_NSYMS = sizeof(RATE_NSYMS) / sizeof(RATE_NSYMS[0]);
   for(size_t i = 0; i < NOF_RATE_NSYMS; ++i) {
      if(RATE_Mbs == RATE_NSYMS[i][0]) {
         return RATE_NSYMS[i][1];
      }
   }
   ostringstream msg;
   msg << "invalid data rate (rate = " << rate_Kbs << ")" << endl;
   raise<invalid_argument>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
}
