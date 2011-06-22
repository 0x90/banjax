/* -*- Mode: C++; tab-width: 3; -*- */

/* 
 * Copyright NICTA, 2011
 */

#define __STDC_LIMIT_MACROS
#include <elc_metric.hpp>
#include <elc_mrr_metric.hpp>
#include <ett_metric.hpp>
#include <metric_demux.hpp>
#include <metric_group.hpp>
#include <metric.hpp>
#include <net/buffer_info.hpp>
#include <net/wnic.hpp>
#include <net/wnic_encoding_fix.hpp>
#include <net/wnic_wallclock_fix.hpp>
#include <dot11/frame.hpp>

#include <boost/program_options.hpp>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <unistd.h>

using namespace boost::program_options;
using namespace dot11;
using namespace net;
using namespace metrics;
using namespace std;

int
main(int ac, char **av)
{
   try {

#if 0
   const char *what = NULL;
   uint16_t rts_cts_threshold = UINT16_MAX;

   options_description options("program options");
   desc.add_options()
      ("help,h", "produce this help message")
      ("input,i", value<string>()->default_value("mon0"), "input file/device name")
      ("rts-threshold,r", value<uint16_t>()->default_value(UINT16_MAX), "RTS threshold level")
      ;

   variables_map vars;       
   store(parse_command_line(ac, av, options), vars);
   notify(vars);   

   if(vars.count("help")) {
      cout << options << "\n";
      return EXIT_SUCCESS;
   }
#else
   opterr = 0;
   int opt, errs = 0;
   uint16_t metric = 0;
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
#endif

   	metric_group_sptr proto(new metric_group);
      proto->push_back(metric_sptr(new elc_metric(rts_cts_threshold)));
      proto->push_back(metric_sptr(new elc_mrr_metric(rts_cts_threshold)));
      proto->push_back(metric_sptr(new ett_metric));
      metric_sptr m(metric_sptr(new metric_demux(proto)));

      wnic_sptr w(wnic::open(what));
      w = wnic_sptr(new wnic_wallclock_fix(w));
      w = wnic_sptr(new wnic_encoding_fix(w, CHANNEL_CODING_OFDM | CHANNEL_PREAMBLE_LONG));
#if 0
      // ToDo: find out why this is garbaging inbound data!
      w->filter("wlan type data"); // ToDo: add BPF test for outbound-only frames
#endif
      buffer_sptr b(w->read());
      buffer_info_sptr info(b->info());
      uint64_t tick = info->timestamp_wallclock();
      for(b; b = w->read();){
         // update metrics with frame
         frame f(b);
         info = b->info();
         eui_48 ra(f.address1());
         frame_control fc(f.fc());
         if(info->has(TX_FLAGS) && fc.type() == DATA_FRAME && !ra.is_special()) {
            m->add(b);
         }
         // is it time to print results yet?
         uint64_t timestamp = info->timestamp_wallclock();
         uint64_t delta = timestamp - tick;
         if(1000000 <= delta) {
            cout << "TIME: " << fixed << setprecision(0) << timestamp /1000000.0 << ", ";
            cout << "DELTA: " << setprecision(3) << delta / 1000000.0 << endl;
            cout << *m << endl;
            m->reset();
            tick = timestamp;
         }
      }
   } catch(const error& x) {
      // ToDo: generate a usage message?
      cerr << x.what() << endl;
   } catch(const exception& x) {
      cerr << x.what() << endl;
   } catch(...) {
      cerr << "unhandled exception!" << endl;
   }
   exit(EXIT_FAILURE);
}
