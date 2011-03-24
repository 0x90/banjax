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
      map<eui_48, metrics::link> stats;
      const char *what = *++av;
      wnic_sptr w(wnic::open(what));
      w = wnic_sptr(new wnic_timestamp_swizzle(w));
      w = wnic_sptr(new wnic_timestamp_fix(w));
      // ToDo: install outbound-only filter
      const uint16_t rts_cts_threshold = UINT16_MAX;
      for(buffer_sptr b; b = w->read();){
         frame f(b);
         eui_48 ra(f.address1());
         if(! ra.is_special()) {

         // ToDo:
         // lookup the station in the map, create if not there
         // add txtime + stuff to it (use duration?)

         }


         // ToDo: if clock ticked then compute ELC, zero totals
      }

   } catch(const exception& x) {
      cerr << x.what() << endl;
   } catch(...) {
      cerr << "unhandled exception!" << endl;
   }
   exit(EXIT_FAILURE);
}
