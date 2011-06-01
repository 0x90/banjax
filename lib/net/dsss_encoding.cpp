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

using namespace net;

dsss_encoding::dsss_encoding()
{
}

dsss_encoding::~dsss_encoding()
{
}

uint16_t
dsss_encoding::SIFS() const
{
   return 16; // ToDo!!!
}

uint16_t
dsss_encoding::slot_time() const
{
   return 9; // ToDo!!!
}

uint16_t
dsss_encoding::txtime(uint16_t frame_sz, uint32_t rate_Kbs, bool has_short_preamble)
{
   float RATE_Mbs = rate_Kbs / 1000;
   const uint16_t PREAMBLE = (has_short_preamble ? 72 + 24 : 144 + 48);
   return PREAMBLE + ceill((frame_sz * 8) / RATE_Mbs);
}

void
dsss_encoding::write(ostream& os) const
{
   os << "DSSS";
}
