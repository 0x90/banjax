/* -*- Mode: C++; tab-width: 3; -*- */

/* 
 * Copyright NICTA, 2011
 */

#define __STDC_LIMIT_MACROS
#include <net/wnic.hpp>
#include <dot11/frame.hpp>
#include <net/wnic_wallclock_fix.hpp>

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <link.hpp>

using namespace net;
using namespace metrics;
using namespace std;
using dot11::frame;

int
main(int ac, char **av)
{
   if(2 != ac) {
      cerr << "usage: elc device" << endl;
      exit(EXIT_FAILURE);
   }

   try {
      const char *what = *++av;
      wnic_sptr w(wnic::open(what));
      w = wnic_sptr(new wnic_wallclock_fix(w));
      w->filter("wlan type data"); // ToDo: add test for outbound-only frames

      const uint16_t rts_cts_threshold = UINT16_MAX;
      typedef map<eui_48, metrics::link_sptr> linkmap;
      linkmap links;
      buffer_sptr b(w->read());
      buffer_info_sptr info(b->info());
      uint64_t tick = info->get(TIMESTAMP_WALLCLOCK);
      for(b; b = w->read();){
         frame f(b);
         info = b->info();
         eui_48 ra(f.address1());
         // find/create the link stats + update with packet
         if(info->has(TXFLAGS) && !ra.is_special()) {
            link_sptr l;
            linkmap::iterator i(links.find(ra));
            if(links.end() != i) {
               l = i->second;
            } else {
               eui_48 ta(f.address2());
               l = link_sptr(new metrics::link(ra, ta, rts_cts_threshold));
               links[ra] = l;
            }
            l->add(b);
         }
         // time to print results?
         uint64_t timestamp = info->get(TIMESTAMP_WALLCLOCK);
         uint64_t delta = timestamp - tick;
         if(1000000 <= delta) {
            // write output
            cout << timestamp / 1000000.0 << " " << delta / 1000000.0 << endl;
            for(linkmap::iterator i = links.begin(); i != links.end(); ++i) {
               cout << *(i->second) << endl;
            }
            cout << endl;
            // zero all counts
            links.clear();
            tick = timestamp;
         }
      }
   } catch(const exception& x) {
      cerr << x.what() << endl;
   } catch(...) {
      cerr << "unhandled exception!" << endl;
   }
   exit(EXIT_FAILURE);
}
