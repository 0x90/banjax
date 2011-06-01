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

#include <net/fhss_encoding.hpp>

#include <math.h>
#include <iostream>

using namespace net;
using namespace std;

encoding_sptr
fhss_encoding::get()
{
   static encoding_sptr enc(new fhss_encoding);
   return enc;
}

fhss_encoding::~fhss_encoding()
{
}

uint16_t
fhss_encoding::CWMIN() const
{
   return 15;
}

uint16_t
fhss_encoding::CWMAX() const
{
   return 1023;
}

uint16_t
fhss_encoding::SIFS() const
{
   return 28;
}

uint16_t
fhss_encoding::slot_time() const
{
   return 50;
}

uint16_t
fhss_encoding::txtime(uint16_t frame_sz, uint32_t rate_Kbs, bool has_short_preamble) const
{
   float RATE_Mbs = rate_Kbs / 1000;
   const uint16_t PREAMBLE = 96;
   const uint16_t PLCP = 32;
   return PREAMBLE + PLCP + ceill((frame_sz * 8) / RATE_Mbs);
}

void
fhss_encoding::write(ostream& os) const
{
   os << "FHSS";
}

fhss_encoding::fhss_encoding()
{
}
