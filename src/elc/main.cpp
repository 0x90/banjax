/* -*- Mode: C++; tab-width: 3; -*- */

/* 
 * Copyright NICTA, 2011
 */

#define __STDC_LIMIT_MACROS
#include <elc_metric.hpp>
#include <elc_mrr_metric.hpp>
// #include <ett_metric.hpp>
#include <etx_metric.hpp>
#include <legacy_elc_metric.hpp>
#include <metric_demux.hpp>
#include <metric_group.hpp>
#include <metric.hpp>
#include <net/buffer_info.hpp>
#include <net/wnic.hpp>
#include <net/wnic_encoding_fix.hpp>
#include <net/wnic_wallclock_fix.hpp>
#include <utilization_metric.hpp>

#include <boost/program_options.hpp>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <unistd.h>

using namespace boost;
using namespace boost::program_options;
using namespace net;
using namespace metrics;
using namespace std;

int
main(int ac, char **av)
{
   try {

      string what;
      uint16_t rts_cts_threshold = UINT16_MAX;

      options_description options("program options");
      options.add_options()
         ("help,?", "produce this help message")
         ("input,i", value<string>(&what)->default_value("mon0"), "input file/device name")
         ("rts-threshold,r", value<uint16_t>(&rts_cts_threshold)->default_value(UINT16_MAX), "RTS threshold level")
         ;

      variables_map vars;       
      store(parse_command_line(ac, av, options), vars);
      notify(vars);   

      if(vars.count("help")) {
         cout << options << endl;
         exit(EXIT_SUCCESS);
      }

   	metric_group_sptr proto(new metric_group);
      proto->push_back(metric_sptr(new utilization_metric));
      proto->push_back(metric_sptr(new elc_metric(rts_cts_threshold)));
      proto->push_back(metric_sptr(new elc_mrr_metric(rts_cts_threshold)));
      proto->push_back(metric_sptr(new legacy_elc_metric));
//      proto->push_back(metric_sptr(new etx_metric));

      metric_sptr m(metric_sptr(new metric_demux(proto)));

      wnic_sptr w(wnic::open(what));
      w = wnic_sptr(new wnic_wallclock_fix(w));
      w = wnic_sptr(new wnic_encoding_fix(w, CHANNEL_CODING_OFDM | CHANNEL_PREAMBLE_LONG)); // ToDo: add cmd line opt to choose default coding!
#if 0
      // ToDo: figure out why this is garbaging inbound data!
      w->filter("wlan type data"); // ToDo: add BPF test for outbound-only frames
#endif
      buffer_sptr b(w->read());
      buffer_info_sptr info(b->info());
      const uint64_t uS_PER_TICK = UINT64_C(1000000);
      uint64_t tick = info->timestamp_wallclock() + uS_PER_TICK;
      for(b; b = w->read();){
         // is it time to print results yet?
         info = b->info();
         uint64_t timestamp = info->timestamp_wallclock();
         for(; tick <= timestamp; tick += uS_PER_TICK) {
            cout << "TIME: " << tick / uS_PER_TICK << endl;
            cout << *m << endl;
            m->reset();
         }
         // update metrics with frame
         m->add(b);
      }
   } catch(const error& x) {
      cerr << x.what() << endl;
   } catch(const std::exception& x) {
      cerr << x.what() << endl;
   } catch(...) {
      cerr << "unhandled exception!" << endl;
   }
   exit(EXIT_FAILURE);
}
