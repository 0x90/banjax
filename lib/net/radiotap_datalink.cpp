/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2010-2011 Steve Glass
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

#define __STDC_CONSTANT_MACROS

#include <net/buffer_body.hpp>
#include <net/radiotap_datalink.hpp>
#include <util/byteswab.hpp>
#include <util/dump.hpp>
#include <util/exceptions.hpp>

#include <algorithm>
#include <cstddef>
#include <pcap.h>
#include <vector>

using namespace std;
using namespace net;
using util::dump;
using util::le_to_cpu;
using util::raise;

const uint32_t RADIOTAP_TSFT              = 0x0001;
const uint32_t RADIOTAP_FLAGS             = 0x0002;
const uint32_t RADIOTAP_RATE              = 0x0004;
const uint32_t RADIOTAP_CHANNEL           = 0x0008;
const uint32_t RADIOTAP_FHSS              = 0x0010;
const uint32_t RADIOTAP_DBM_ANTSIGNAL     = 0x0020;
const uint32_t RADIOTAP_DBM_ANTNOISE      = 0x0040;
const uint32_t RADIOTAP_LOCK_QUALITY      = 0x0080;
const uint32_t RADIOTAP_TX_ATTENUATION    = 0x0100;
const uint32_t RADIOTAP_DB_TX_ATTENUATION = 0x0200;
const uint32_t RADIOTAP_DBM_TX_POWER      = 0x0400;
const uint32_t RADIOTAP_ANTENNA           = 0x0800;
const uint32_t RADIOTAP_DB_ANTSIGNAL      = 0x1000;
const uint32_t RADIOTAP_DB_ANTNOISE       = 0x2000;
const uint32_t RADIOTAP_RXFLAGS           = 0x4000;
const uint32_t RADIOTAP_TXFLAGS           = 0x8000;
const uint32_t RADIOTAP_RTS_RETRIES       = 0x10000;
const uint32_t RADIOTAP_DATA_RETRIES      = 0x20000;
const uint32_t RADIOTAP_RATE_TUPLES       = 0x10000000;
const uint32_t RADIOTAP_EXT               = 0x80000000;

const uint8_t  RADIOTAP_FLAGS_CFP         = 0x01;
const uint8_t  RADIOTAP_FLAGS_SHORTPRE    = 0x02;
const uint8_t  RADIOTAP_FLAGS_WEP         = 0x04;
const uint8_t  RADIOTAP_FLAGS_FRAG        = 0x08;
const uint8_t  RADIOTAP_FLAGS_FCS         = 0x10;
const uint8_t  RADIOTAP_FLAGS_PAD         = 0x20;
const uint8_t  RADIOTAP_FLAGS_BAD_FCS     = 0x40;
const uint8_t  RADIOTAP_FLAGS_SHORTGRD    = 0x80;

const uint16_t RADIOTAP_CHAN_TURBO        = 0x0010;
const uint16_t RADIOTAP_CHAN_CCK          = 0x0020;
const uint16_t RADIOTAP_CHAN_OFDM         = 0x0040;
const uint16_t RADIOTAP_CHAN_2GHZ         = 0x0080;
const uint16_t RADIOTAP_CHAN_5GHZ         = 0x0100;
const uint16_t RADIOTAP_CHAN_PASSIVE      = 0x0200;
const uint16_t RADIOTAP_CHAN_DYN          = 0x0400;
const uint16_t RADIOTAP_CHAN_GFSK         = 0x0800;
const uint16_t RADIOTAP_CHAN_900MHZ       = 0x1000;
const uint16_t RADIOTAP_CHAN_STURBO       = 0x2000;
const uint16_t RADIOTAP_CHAN_HALF_RATE    = 0x4000;
const uint16_t RADIOTAP_CHAN_QUARTER_RATE = 0x8000;

const uint16_t RADIOTAP_TXFLAGS_FAIL      = 0x0001;
const uint16_t RADIOTAP_TXFLAGS_CTS       = 0x0002;
const uint16_t RADIOTAP_TXFLAGS_RTS_CTS   = 0x0004;
const uint16_t RADIOTAP_TXFLAGS_NO_ACK    = 0x0008;

const uint16_t RADIOTAP_RXFLAGS_BAD_FCS   = 0x0001;
const uint16_t RADIOTAP_RXFLAGS_BAD_PLCP  = 0x0002;

radiotap_datalink::radiotap_datalink()
{
}

radiotap_datalink::~radiotap_datalink()
{
}

size_t
radiotap_datalink::format(const buffer& b, size_t frame_sz, uint8_t *frame)
{
   CHECK_NOT_NULL(frame);
   // ToDo: add the datalink header
   size_t n = std::min(frame_sz, b.data_size());
   const uint8_t *buf = b.data();
   copy(&buf[0], &buf[n], frame);
   return n;
}

const char*
radiotap_datalink::name() const
{
   return "IEEE 80211 (RADIOTAP)";
}

/**
 * Template helper function to exract a field from a radiotap header
 * taking care of alignment, byte-ordering and buffer-overflow issues.
 *
 * \param ofs The offset of the field from the frame start.
 * \param field The field to extract.
 * \param hdr_sz The size of the radiotap_header.
 * \param frame_sz The size of the frame (including radiotap_header).
 * \param frame A pointer to the frame.
 * \throws length_error When reading would ovaerflow the buffer.
 */
template<typename T> void
extract(size_t& ofs, T& field, size_t hdr_sz, size_t frame_sz, const uint8_t *frame)
{
   const size_t field_sz = sizeof(field);
   const size_t align = field_sz - 1;
   ofs = (ofs + align) & ~align;
   if(!(ofs < hdr_sz)) {
      ostringstream msg;
      msg << "short read of argument from radiotap header at offset " << ofs << endl;
      msg << hex << dump(frame_sz, frame) << endl;
      raise<length_error>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
   }
   le_to_cpu(frame + ofs, field);
   ofs += field_sz;
}

buffer_sptr
radiotap_datalink::parse(size_t frame_sz, const uint8_t *frame)
{
   CHECK_NOT_NULL(frame);

   const radiotap_header *hdr = reinterpret_cast<const radiotap_header*>(frame);
   if(0 != hdr->version_) {
      ostringstream msg;
      msg << "unsupported radiotap version (version=" << hex << showbase << setw(2) << setfill('0') << static_cast<uint16_t>(hdr->version_) << ")" << endl;
      msg << hex << dump(frame_sz, frame) << endl;
      raise<invalid_argument>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
   }

   uint16_t hdr_sz = 0;
   le_to_cpu(reinterpret_cast<const uint8_t*>(&hdr->size_), hdr_sz);
   const size_t MIN_HDR_SZ = 8;
   if(!(MIN_HDR_SZ <= hdr_sz && hdr_sz <= frame_sz)) {
      ostringstream msg;
      msg << "partial read of radiotap header (expected " << MIN_HDR_SZ << " <= size <= " << frame_sz << ", actual size=" << hdr_sz << ")" << endl;
      msg << hex << dump(frame_sz, frame) << endl;
      raise<invalid_argument>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
   }

   uint32_t bitmap = 0;
   const uint8_t *bitmaps = frame + offsetof(radiotap_header, bitmaps_);
   const uint8_t *limit = frame + hdr_sz;
   le_to_cpu(reinterpret_cast<const uint8_t*>(bitmaps), bitmap);
   bitmaps += sizeof(uint32_t);
   uint32_t ext_bitmap = bitmap;
   while(ext_bitmap & RADIOTAP_EXT) {
      if(!(bitmaps < limit)) {
         ostringstream msg;
         msg << "bad radiotap header (expected " << MIN_HDR_SZ << " <= size <= " << frame_sz << ", actual size=" << hdr_sz << ")"<< endl;
         msg << hex << dump(frame_sz, frame) << endl;
         raise<invalid_argument>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
      }
      le_to_cpu(reinterpret_cast<const uint8_t*>(bitmaps), ext_bitmap);
      bitmaps += sizeof(uint32_t);
   }

   flags_t rx_flags = 0;
   flags_t chan_flags = 0;
   size_t ofs = bitmaps - frame;
   buffer_info_sptr info(new buffer_info);
   for(uint32_t i = RADIOTAP_TSFT; i < RADIOTAP_EXT; i <<= 1) {
      int8_t junk_s8;
      uint8_t junk_u8;
      uint16_t junk_u16;
      uint64_t junk_u64;
      uint32_t bit = bitmap & i;
      switch(bit) {
      case RADIOTAP_TSFT:
         extract(ofs, junk_u64, hdr_sz, frame_sz, frame);
         info->timestamp1(junk_u64);
         break;
      case RADIOTAP_FLAGS:
         extract(ofs, junk_u8, hdr_sz, frame_sz, frame);
         if(junk_u8 & RADIOTAP_FLAGS_FCS) {
            frame_sz -= sizeof(uint32_t); // remove FCS - banjax *never* provides FCS field
         }
         chan_flags |= (junk_u8 & RADIOTAP_FLAGS_SHORTPRE) ? CHANNEL_PREAMBLE_SHORT : CHANNEL_PREAMBLE_LONG;
         info->channel_flags(chan_flags);
         rx_flags |= (junk_u8 & RADIOTAP_FLAGS_BAD_FCS) ? RX_FLAGS_BAD_FCS : 0;
         info->rx_flags(rx_flags);
         break;
      case RADIOTAP_RATE:
         extract(ofs, junk_u8, hdr_sz, frame_sz, frame);
         info->rate_Kbs(junk_u8 * 500);
         break;
      case RADIOTAP_CHANNEL:
         extract(ofs, junk_u16, hdr_sz, frame_sz, frame);
         info->freq_MHz(junk_u16);
         extract(ofs, junk_u16, hdr_sz, frame_sz, frame); 
         if(junk_u16 & (RADIOTAP_CHAN_TURBO | RADIOTAP_CHAN_STURBO)) {
            ostringstream msg;
            msg << "unsupported channel type in radiotap header (";
            msg << "channel flags =" << hex << showbase << setw(4) << junk_u16 << ")" << endl;
            msg << hex << dump(frame_sz, frame) << endl;
            raise<runtime_error>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
         }
         chan_flags |= (junk_u16 & RADIOTAP_CHAN_CCK)  ? CHANNEL_CODING_DSSS : 0;
         chan_flags |= (junk_u16 & RADIOTAP_CHAN_OFDM) ? CHANNEL_CODING_OFDM : 0;
         chan_flags |= (junk_u16 & RADIOTAP_CHAN_GFSK) ? CHANNEL_CODING_FHSS : 0;
         chan_flags |= (junk_u16 & RADIOTAP_CHAN_DYN)  ? CHANNEL_CODING_DYNAMIC : 0;
         if(junk_u16 & RADIOTAP_CHAN_QUARTER_RATE)
            chan_flags |= CHANNEL_RATE_QUARTER;
         else if(junk_u16 & RADIOTAP_CHAN_HALF_RATE)
            chan_flags |= CHANNEL_RATE_HALF;
         else
            chan_flags |= CHANNEL_RATE_FULL;
         info->channel_flags(chan_flags);
         break;
      case RADIOTAP_FHSS:
         extract(ofs, junk_u8, hdr_sz, frame_sz, frame);
         extract(ofs, junk_u8, hdr_sz, frame_sz, frame);
         break;
      case RADIOTAP_DBM_ANTSIGNAL:
         extract(ofs, junk_s8, hdr_sz, frame_sz, frame);
         info->signal_dBm(junk_s8);
         break;
      case RADIOTAP_DBM_ANTNOISE:
         extract(ofs, junk_s8, hdr_sz, frame_sz, frame);
         break;
      case RADIOTAP_LOCK_QUALITY:
         extract(ofs, junk_u8, hdr_sz, frame_sz, frame);
         extract(ofs, junk_u8, hdr_sz, frame_sz, frame);
         break;
      case RADIOTAP_TX_ATTENUATION:
         extract(ofs, junk_u8, hdr_sz, frame_sz, frame);
         extract(ofs, junk_u8, hdr_sz, frame_sz, frame);
         break;
      case RADIOTAP_DB_TX_ATTENUATION:
         extract(ofs, junk_u8, hdr_sz, frame_sz, frame);
         extract(ofs, junk_u8, hdr_sz, frame_sz, frame);
         break;
      case RADIOTAP_DBM_TX_POWER:
         extract(ofs, junk_s8, hdr_sz, frame_sz, frame);
         break;
      case RADIOTAP_ANTENNA:
         extract(ofs, junk_u8, hdr_sz, frame_sz, frame);
         break;
      case RADIOTAP_DB_ANTSIGNAL:
         extract(ofs, junk_u8, hdr_sz, frame_sz, frame);
         info->signal_dBm(junk_u8);
         break;
      case RADIOTAP_DB_ANTNOISE:
         extract(ofs, junk_u8, hdr_sz, frame_sz, frame);
         break;
      case RADIOTAP_RXFLAGS:
         extract(ofs, junk_u16, hdr_sz, frame_sz, frame);
         break;
         // from here onwards are "suggested fields"
      case RADIOTAP_TXFLAGS:
         extract(ofs, junk_u16, hdr_sz, frame_sz, frame);
         info->tx_flags((junk_u16 & RADIOTAP_TXFLAGS_FAIL) ? TX_FLAGS_FAIL : 0);
         break;
      case RADIOTAP_RTS_RETRIES:
         extract(ofs, junk_u8, hdr_sz, frame_sz, frame);
         info->rts_retries(junk_u8);
         break;
      case RADIOTAP_DATA_RETRIES:
         extract(ofs, junk_u8, hdr_sz, frame_sz, frame);
         info->data_retries(junk_u8);
         break;
         //  Warning: proprietary extension!
      case RADIOTAP_RATE_TUPLES:
	      {
            vector<uint32_t> rates;
            for(size_t i = 0; i < 5; ++i) {
               uint8_t rate, flags, tries;
               extract(ofs, rate, hdr_sz, frame_sz, frame);
               extract(ofs, flags, hdr_sz, frame_sz, frame);
               extract(ofs, tries, hdr_sz, frame_sz, frame);
               for(uint16_t i = 0; i < tries; ++i) {
                  rates.push_back(rate * UINT32_C(500));
               }
            }
            info->rates(rates);
         }
         break;
      default:
         break;
      }
   }
   frame += hdr_sz;
   frame_sz -= hdr_sz;
   return buffer_sptr(new buffer_body(frame_sz, frame, info));
}

int
radiotap_datalink::type() const
{
   return DLT_IEEE802_11_RADIO;
}
