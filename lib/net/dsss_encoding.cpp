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

#include <net/dsss_encoding.hpp>

#include <math.h>
#include <iostream>

using namespace net;
using namespace std;

encoding_sptr
dsss_encoding::get()
{
   static encoding_sptr enc(new dsss_encoding);
   return enc;
}

dsss_encoding::~dsss_encoding()
{
}

uint16_t
dsss_encoding::CWMIN() const
{
   return 31;
}

uint16_t
dsss_encoding::CWMAX() const
{
   return 1023;
}

uint16_t
dsss_encoding::SIFS() const
{
   return 10;
}

uint16_t
dsss_encoding::slot_time() const
{
   return 20;
}

uint16_t
dsss_encoding::txtime(uint16_t frame_sz, uint32_t rate_Kbs, bool has_short_preamble) const
{
   float RATE_Mbs = rate_Kbs / 1000;
   const uint16_t PREAMBLE = has_short_preamble ? 72 : 144;
   const uint16_t PLCP = has_short_preamble ? 24 : 48;
   return PREAMBLE + PLCP + ceill((frame_sz * 8) / RATE_Mbs);
}

void
dsss_encoding::write(ostream& os) const
{
   os << "DSSS";
}

dsss_encoding::dsss_encoding()
{
}
