/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2009,2010 Steve Glass
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

#include <net/offline_wnic.hpp>
#include <net/linux_wnic.hpp>
#include <net/pcap_wnic.hpp>
#include <net/wnic_read_logger.hpp>
#include <net/wnic_write_logger.hpp>
#include <net/wnic.hpp>

#include <cstring>

using namespace net;
using std::string;

bool
check_ext(const string& s, const string& ext)
{
   bool match = false;
   const size_t s_sz = s.size();
   const size_t ext_sz = ext.size();
   if(s_sz >= ext_sz) {
      if(s.substr(s_sz - ext_sz) == ext) {
         match = true;
      }
   }
   return match;
}

bool
strip_ext(string& s, const string& ext)
{
   bool match = false;
   const size_t s_sz = s.size();
   const size_t ext_sz = ext.size();
   if(s_sz >= ext_sz) {
      if(s.substr(s_sz - ext_sz) == ext) {
         s.erase(s_sz - ext_sz);
         match = true;
      }
   }
   return match;
}

wnic*
new_wnic(const string& dev_name) 
{
   wnic *dev;
#if __linux__
   const string linux_wireless_dev("/sys/class/net/" + dev_name);
   if(0 == access(linux_wireless_dev.c_str(), 0500)) {
      dev = new linux_wnic(dev_name);
   } else {
#endif
      dev = new pcap_wnic(dev_name);
#if __linux__
   }
#endif
   return dev;
}

wnic_sptr
wnic::open(string dev_name)
{
   wnic *dev = NULL;
   if(check_ext(dev_name, ".pcap")) {
      dev = new offline_wnic(dev_name);
   } else  {
      if(strip_ext(dev_name, "+r")) {
         dev = new_wnic(dev_name);
         dev = new wnic_read_logger(wnic_sptr(dev));
      } else if(strip_ext(dev_name, "+w")) {
         dev = new_wnic(dev_name);
         dev = new wnic_write_logger(wnic_sptr(dev));
      } else if(strip_ext(dev_name, "+rw")) {
         dev = new_wnic(dev_name);
         dev = new wnic_read_logger(wnic_sptr(dev));
         dev = new wnic_write_logger(wnic_sptr(dev));
      } else {
         dev = new_wnic(dev_name);
      }
   }
   return wnic_sptr(dev);
}
