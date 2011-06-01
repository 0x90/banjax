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

#include <net/ofdm_encoding.hpp>

using namespace net;

ofdm_encoding::ofdm_encoding()
{
}

ofdm_encoding::~ofdm_encoding()
{
}

uint16_t
ofdm_encoding::SIFS() const
{
   return 16;
}

uint16_t
ofdm_encoding::slot_time() const
{
   return 9;
}

uint16_t
ofdm_encoding::txtime(uint16_t frame_sz, uint32_t rate_Kbs, bool ignored) const
{
   const float NDBPS = ndbps(rate_Kbs);
   const uint16_t PREAMBLE = 16;
   const uint16_t SIGNAL = 4;
   const uint16_t SYM = 4;
   return PREAMBLE + SIGNAL + SYM * ceill((16 + (8 * frame_sz) + 6) / NDBPS);
}

void
ofdm_encoding::write(ostream& os) const
{
   os << "OFDM";
}

uint16_t
ofdm_encoding::ndbps(uint32_t rate_Kbs) const
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
