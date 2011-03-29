/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#include <link.hpp>
#include <net/txtime.hpp>
#include <util/exceptions.hpp>

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <math.h>

using namespace net;
using namespace std;
using metrics::link;
using util::raise;

link::link(const eui_48& to, const eui_48& from, uint16_t rts_cts_threshold) :
   to_(to),
   from_(from),
   rts_cts_threshold_(rts_cts_threshold),
   n_pkt_succ_(0),
   t_pkt_succ_(0.0),
   t_pkt_fail_(0.0),
   packet_octets_(0),
   packet_count_(0)
{
}

link::link(const link& other) :
   to_(other.to_),
   from_(other.from_),
   rts_cts_threshold_(other.rts_cts_threshold_),
   n_pkt_succ_(other.n_pkt_succ_),
   t_pkt_succ_(other.t_pkt_succ_),
   t_pkt_fail_(other.t_pkt_fail_),
   packet_octets_(other.packet_octets_),
   packet_count_(other.packet_count_)
{
}

metrics::link&
link::operator=(const link& other)
{
   if(this != &other) {
      to_ = other.to_;
      from_ = other.from_;
      rts_cts_threshold_ = other.rts_cts_threshold_;     
      n_pkt_succ_ = other.n_pkt_succ_;
      t_pkt_succ_ = other.t_pkt_succ_;
      t_pkt_fail_ = other.t_pkt_fail_;
      packet_octets_ = other.packet_octets_;
      packet_count_ = other.packet_count_;
   }
   return *this;
}

link::~link()
{
}

bool
link::operator==(const link& other) const
{
   return to_ == other.to_ ;
}

bool
link::operator<(const link& other) const
{
   return to_ < other.to_;
}

void
link::add(buffer_sptr b)
{
   // update totals for packet size and count
   const uint32_t LLC_HDR_SZ = 8;
   const uint32_t IEEE80211_HDR_SZ = 24;
   const uint32_t IP_HDR_SZ = 20;
   const uint32_t UDP_HDR_SZ = 8;
   packet_octets_ += b->data_size() - IEEE80211_HDR_SZ - LLC_HDR_SZ - IP_HDR_SZ - UDP_HDR_SZ;
   ++packet_count_;

   // compute the t_pktime taken to send this packet - whether good or bad
   buffer_info_sptr info(b->info());
   uint32_t tx_flags = info->get(TXFLAGS);
   if(tx_flags & TXFLAGS_FAIL) {
      t_pkt_fail_ += packet_fail_time(b);
   } else {
      ++n_pkt_succ_;
      t_pkt_succ_ += packet_succ_time(b);
   }

}

size_t
link::hash() const
{
   return to_.hash();
}

void
link::write(ostream& os) const
{
//   os << to_ << " ";
   os << to_ << endl;
   os << n_pkt_succ_ << endl;
   os << packet_octets_ << endl;
   os << packet_count_ << endl;
   os << t_pkt_succ_ << endl;
   os << t_pkt_fail_ << endl;
   os << metric() << endl;
}

ostream&
metrics::operator<<(ostream& os, const link& addr)
{
   addr.write(os);
   return os;
}

double
link::metric() const
{
   const double AVG_PKT_SZ = packet_octets_ / static_cast<double>(packet_count_);
   return (n_pkt_succ_ * AVG_PKT_SZ) / (t_pkt_succ_ + t_pkt_fail_);
}

double
link::packet_succ_time(buffer_sptr b) const
{
   buffer_info_sptr info(b->info());
   double usecs = 0.0;
   uint8_t txc = 1 + info->get(RETRIES);
   for(uint8_t i = 0; i < txc - 1; ++i) {
      usecs += avg_contention_time(i) + frame_fail_time(b);
   }
   return usecs + avg_contention_time(txc) + frame_succ_time(b);
}

double
link::packet_fail_time(buffer_sptr b) const
{
   buffer_info_sptr info(b->info());
   double usecs = 0.0;
   uint8_t txc = 1 + info->get(RETRIES);
   for(uint8_t i = 0; i < txc; ++i) {
      usecs += avg_contention_time(i) + frame_fail_time(b);
   }
   return usecs;
}

double
link::avg_contention_time(uint8_t txc) const
{
   return max_contention_time(txc) / 2.0;
}

double
link::max_contention_time(uint8_t txc) const
{
  /* ath5k hack: collapse contention window after 10 attempts */
  if(txc >= 10) {
    txc %= 10;
  }
  /* end hack */
  
  const uint32_t T_SLOT = 9;
  const uint32_t CW = pow(2, txc+3) - 1;
  return min(CW, 1023u) * T_SLOT;
}

const uint32_t T_SIFS = 16;
const uint32_t T_SLOT = 9;
const uint32_t T_DIFS = T_SIFS + 2 * T_SLOT;

double 
link::frame_succ_time(buffer_sptr b) const
{
   buffer_info_sptr info(b->info());

   const uint32_t CRC_SZ = 4;
   const uint32_t FRAME_SZ = b->data_size() + CRC_SZ;
   const uint32_t T_RTS_CTS = rts_cts_time(FRAME_SZ);
   const uint32_t DATA_RATE = info->get(RATE_Kbs);
   const uint32_t T_DATA = txtime_ofdm(DATA_RATE, FRAME_SZ);
   const uint32_t ACK_RATE = ack_rate(DATA_RATE);
   const uint32_t ACK_SZ = 14;
   const uint32_t T_ACK = txtime_ofdm(ACK_RATE, ACK_SZ);

   return T_RTS_CTS + T_DATA + T_SIFS + T_ACK + T_DIFS;
}

double
link::frame_fail_time(buffer_sptr b) const
{
   buffer_info_sptr info(b->info());

   const uint32_t CRC_SZ = 4;
   const uint32_t FRAME_SZ = b->data_size() + CRC_SZ;
   const uint32_t T_RTS_CTS = rts_cts_time(FRAME_SZ);
   const uint32_t DATA_RATE = info->get(RATE_Kbs);
   const uint32_t T_DATA = txtime_ofdm(DATA_RATE, FRAME_SZ);
   const uint32_t T_ACKTIMEOUT = 50;

   return T_RTS_CTS + T_DATA + T_SIFS + T_ACKTIMEOUT + T_DIFS;
}

double
link::rts_cts_time(uint32_t frame_sz) const
{
   double usecs = 0.0;
   if(rts_cts_threshold_ <= frame_sz) {
      const uint32_t RTS_SZ = 20;
      const uint32_t CTS_SZ = 14;
      usecs = txtime_ofdm(6000, RTS_SZ) + T_SIFS + txtime_ofdm(6000, CTS_SZ) + T_SIFS;
   }
   return usecs;
}

uint32_t
link:: ack_rate(uint32_t rate_Kbs) const
{
   static const uint32_t RATES[][2] = {
      {  6000,  6000 },
      {  9000,  6000 },
      { 12000, 12000 },
      { 18000, 12000 },
      { 24000, 24000 },
      { 36000, 24000 },
      { 48000, 24000 },
      { 54000, 24000 }
   };
   static const size_t NOF_RATES = sizeof(RATES) / sizeof(RATES[0]);
   for(size_t i = 0; i < NOF_RATES; ++i) {
      if(RATES[i][0] == rate_Kbs) {
         return RATES[i][1];
      }
   }
   ostringstream msg;
   msg << rate_Kbs << " is not a recognized OFDM data rate!" << endl;
   raise<invalid_argument>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
}
