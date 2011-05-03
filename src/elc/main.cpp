/* -*- Mode: C++; tab-width: 3; -*- */

/* 
 * Copyright NICTA, 2011
 */

#define __STDC_LIMIT_MACROS
#include <elc_link_metric.hpp>
#include <link_metric.hpp>
#include <net/wnic.hpp>
#include <dot11/frame.hpp>
#include <net/wnic_wallclock_fix.hpp>

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <string.h>
#include <unistd.h>

using namespace dot11;
using namespace net;
using namespace metrics;
using namespace std;

int
main(int ac, char **av)
{
   opterr = 0;
   int opt, errs = 0;
   const char *what = NULL;
   uint16_t rts_cts_threshold = UINT16_MAX;
   while((opt = getopt(ac, av, "i:r:")) != -1) {
      switch(opt) {
      case 'i':
         what = strdupa(optarg);
         break;
      case 'r':
         rts_cts_threshold = atoi(optarg);
         break;
      default:
         ++errs;
         break;
      }
   }
   if(errs || NULL == what) {
      cerr << "usage: elc [-r rts_cts_threshold] -i input" << endl;
      exit(EXIT_FAILURE);
   }

   try {
      wnic_sptr w(wnic::open(what));
      w = wnic_sptr(new wnic_wallclock_fix(w));
      w->filter("wlan type data"); // ToDo: add test for outbound-only frames
      typedef map<eui_48, metrics::link_metric_sptr> linkmap;
      linkmap links;
      buffer_sptr b(w->read());
      buffer_info_sptr info(b->info());
      uint64_t tick = info->timestamp_wallclock();
      for(b; b = w->read();){
         frame f(b);
         info = b->info();
         eui_48 ra(f.address1());
         frame_control fc(f.fc());
         // find/create the link stats + update with packet
         if(info->has(TX_FLAGS) && fc.type() == DATA_FRAME && !ra.is_special()) {
            link_metric_sptr l;
            linkmap::iterator i(links.find(ra));
            if(links.end() != i) {
               l = i->second;
            } else {
               eui_48 ta(f.address2());
               l = link_metric_sptr(new elc_link_metric(ra, ta, rts_cts_threshold));
               links[ra] = l;
            }
            l->add(b);
         }
         // time to print results?
         uint64_t timestamp = info->timestamp_wallclock();
         uint64_t delta = timestamp - tick;
         if(1000000 <= delta) {
            // write output
            for(linkmap::iterator i = links.begin(); i != links.end(); ++i) {
               cout << fixed << setprecision(6) << timestamp /1000000.0 << " " << delta / 1000000.0 << " ";
               cout << *(i->second) << endl;
            }
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
