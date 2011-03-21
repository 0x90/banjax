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

#include <net/buffer.hpp>
#include <net/buffer_info.hpp>
#include <net/wnic_timestamp_fix.hpp>
#include <net/txtime.hpp>
#include <util/exceptions.hpp>e

using namespace net;
using namespace std;
using util::raise;

wnic_timestamp_fix::wnic_timestamp_fix(wnic_sptr wnic) :
   wnic_wrapper(wnic)
{
}

wnic_timestamp_fix::~wnic_timestamp_fix()
{
}

buffer_sptr
wnic_timestamp_fix::read()
{
   buffer_sptr buf(wnic_->read());
   if(buf) {
      buffer_info_sptr info(buf->info());
      if(info->has(RATE_Kbs | RXFLAGS | TIMESTAMP1) ^ (info->has(RATE_Kbs | RXFLAGS | TIMESTAMP2))) {

         // compute txtime
         uint32_t t = 0;
         const uint32_t CRC_SZ = 4;
         uint32_t rate = info->get(RATE_Kbs);
         uint64_t rxflags = info->get(RXFLAGS);
         bool short_preamble = rxflags & RXFLAGS_PREAMBLE_SHORT;
         if(rxflags & RXFLAGS_CODING_FHSS) {
            t = txtime_fhss(rate, buf->data_size() + CRC_SZ, short_preamble);
         } else if(rxflags & RXFLAGS_CODING_DSSS) {
            t = txtime_dsss(rate, buf->data_size() + CRC_SZ, short_preamble);
         } else if(rxflags & RXFLAGS_CODING_DYNAMIC) {
            t = txtime_dsss_ofdm(rate, buf->data_size() + CRC_SZ, short_preamble);
         } else if(rxflags & RXFLAGS_CODING_OFDM) {
            t = txtime_ofdm(rate, buf->data_size() + CRC_SZ);
         } else {
            ostringstream msg;
            msg << "unknown channel encoding (" << *info << ")" << endl;
            raise<logic_error>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
         }

         // adjust timestamp
         if(info->has(TIMESTAMP1)) {
            info->set(TIMESTAMP2, info->get(TIMESTAMP1) + t);
         } else if(info->has(TIMESTAMP2)) {
            info->set(TIMESTAMP1, info->get(TIMESTAMP2) - t);
         }

      } else {
         ostringstream msg;
         msg << "cannot fix frame (" << *info << ")" << endl;
         raise<runtime_error>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
      }
   }
   return buf;
}
