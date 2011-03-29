/* -*- Mode: C++; tab-width: 3; -*- */

/* 
 * Copyright NICTA, 2011
 */

#define __STDC_LIMIT_MACROS
#include <net/wnic.hpp>
#include <net/wnic_timestamp_fix.hpp>
#include <net/wnic_timestamp_swizzle.hpp>
#include <dot11/frame.hpp>

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
//      w = wnic_sptr(new wnic_timestamp_swizzle(w));
//      w = wnic_sptr(new wnic_timestamp_fix(w));
      w->filter("wlan type data"); // ToDo: add test for outbound-only frames

      const uint16_t rts_cts_threshold = UINT16_MAX;

      typedef map<eui_48, metrics::link_sptr> linkmap;
      linkmap links;
      buffer_sptr b(w->read());
//      uint64_t tick = b->info()->get(TIMESTAMP2);
      for(b; b = w->read();){
         frame f(b);
         eui_48 ra(f.address1());
         if(!ra.is_special()) {
/*
            // find/create the link stats
            link_sptr l;
            linkmap::iterator i(links.find(ra));
            if(links.end() != i) {
               l = i->second;
            } else {
               eui_48 ta(f.address2());
               l = link_sptr(new metrics::link(ra, ta, rts_cts_threshold));
               links[ra] = l;
            }
            // add frame to link
            l->add(b);
*/
            cout << dec;
            cout << b->info()->get(RATE_Kbs) << ", ";
            cout << b->info()->get(RETRIES) << endl;
            cout << hex << *b << endl;
         }
/*
         if((b->info()->get(TIMESTAMP2) - tick) >= 1000000) {
            // write output
            for(linkmap::iterator i = links.begin(); i != links.end(); ++i) {
               cout << *(i->second) << endl;
            }
            cout << endl;
            // zero all counts
            links.clear();
            tick = b->info()->get(TIMESTAMP2);
         }
*/
      }
   } catch(const exception& x) {
      cerr << x.what() << endl;
   } catch(...) {
      cerr << "unhandled exception!" << endl;
   }
   exit(EXIT_FAILURE);
}
