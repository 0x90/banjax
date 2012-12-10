/* -*- Mode: C++; tab-width: 3; -*- */

/* 
 * Copyright NICTA, 2011
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <airtime_metric_kernel.hpp>
#include <airtime_metric_linux.hpp>
#include <airtime_metric_measured.hpp>
#include <airtime_metric_ns3.hpp>
#include <elc_metric.hpp>
#include <elc_mrr_metric.hpp>
#include <etx_metric.hpp>
#include <fdr_metric.hpp>
#include <goodput_metric.hpp>
#include <iperf_metric_wrapper.hpp>
#include <legacy_elc_metric.hpp>
#include <metric.hpp>
#include <metric_damper.hpp>
#include <metric_decimator.hpp>
#include <metric_demux.hpp>
#include <metric_group.hpp>
#include <pdr_metric.hpp>
#include <pktsz_metric.hpp>
#include <simple_elc_metric.hpp>
#include <tmt_metric.hpp>
#include <txc_metric.hpp>

#include <net/buffer_info.hpp>
#include <net/ofdm_encoding.hpp>
#include <net/wnic.hpp>
#include <net/wnic_encoding_fix.hpp>
#include <net/wnic_wallclock_fix.hpp>
#include <net/wnic_require_timestamps.hpp>

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

      bool debug, help;
      string enc_str, what;
      uint16_t acktimeout;
      uint32_t dead_time;
      uint16_t cw; 
      uint16_t damp;
      uint16_t mpdu_sz;
      uint16_t port_no;
      uint16_t rts_cts_threshold;
      uint32_t rate_Mbs;
      uint64_t runtime;
      size_t window_sz;
      bool show_ticks;

      options_description options("program options");
      options.add_options()
         ("acktimeout,a", value(&acktimeout)->default_value(UINT16_MAX), "specify ACKTimeout value")
         ("cw,c", value(&cw)->default_value(UINT16_MAX), "size of contention window in microseconds (default = compute average)")
         ("damping,d", value(&damp)->default_value(5), "size of damping window in seconds")
         ("dead,e", value(&dead_time)->default_value(0), "dead time (in microseconds) per tick")
         ("debug,g", value<bool>(&debug)->default_value(false)->zero_tokens(), "enable debug")
         ("encoding", value<string>(&enc_str)->default_value("OFDM"), "channel encoding")
         ("help,?", value(&help)->default_value(false)->zero_tokens(), "produce this help message")
         ("input,i", value<string>(&what)->default_value("mon0"), "input file/device name")
         ("linkrate,l", value<uint32_t>(&rate_Mbs)->default_value(6), "link rate in Mb/s")
         ("mpdu,m", value<uint16_t>(&mpdu_sz)->default_value(1024), "MPDU size used metric calculation")
         ("rts-threshold,r", value<uint16_t>(&rts_cts_threshold)->default_value(UINT16_MAX), "RTS threshold level")
         ("runtime,u", value<uint64_t>(&runtime)->default_value(0), "produce results after n seconds")
         ("ticks,t", value<bool>(&show_ticks)->default_value(false)->zero_tokens(), "show results for each second")
         ;

      variables_map vars;       
      store(parse_command_line(ac, av, options), vars);
      notify(vars);   

      if(help) {
         cout << options << endl;
         exit(EXIT_SUCCESS);
      }

      encoding_sptr enc(encoding::get(enc_str));
   	metric_group_sptr proto(new metric_group);
      proto->push_back(metric_sptr(new tmt_metric(enc, rate_Mbs * 1000, mpdu_sz, rts_cts_threshold)));
      proto->push_back(metric_sptr(new  goodput_metric));
      proto->push_back(metric_sptr(new airtime_metric_linux(enc)));

      proto->push_back(metric_sptr(new airtime_metric_kernel));
      proto->push_back(metric_sptr(new metric_decimator("Airtime-Kernel-50PC", metric_sptr(new airtime_metric_kernel), 2)));
      proto->push_back(metric_sptr(new metric_decimator("Airtime-Kernel-25PC", metric_sptr(new airtime_metric_kernel), 4)));
      proto->push_back(metric_sptr(new metric_decimator("Airtime-Kernel-10PC", metric_sptr(new airtime_metric_kernel), 10)));
      proto->push_back(metric_sptr(new metric_decimator("Airtime-Kernel-5PC", metric_sptr(new airtime_metric_kernel), 20)));

      proto->push_back(metric_sptr(new airtime_metric_measured));
      proto->push_back(metric_sptr(new metric_decimator("Airtime-Measured-50PC", metric_sptr(new airtime_metric_measured), 2)));
      proto->push_back(metric_sptr(new metric_decimator("Airtime-Measured-25PC", metric_sptr(new airtime_metric_measured), 4)));
      proto->push_back(metric_sptr(new metric_decimator("Airtime-Measured-10PC", metric_sptr(new airtime_metric_measured), 10)));
      proto->push_back(metric_sptr(new metric_decimator("Airtime-Measured-5PC", metric_sptr(new airtime_metric_measured), 20)));

      proto->push_back(metric_sptr(new airtime_metric_ns3(enc, rts_cts_threshold)));
      proto->push_back(metric_sptr(new metric_decimator("Airtime-NS3-50PC", metric_sptr(new airtime_metric_ns3(enc, rts_cts_threshold)), 2)));
      proto->push_back(metric_sptr(new metric_decimator("Airtime-NS3-25PC", metric_sptr(new airtime_metric_ns3(enc, rts_cts_threshold)), 4)));
      proto->push_back(metric_sptr(new metric_decimator("Airtime-NS3-10PC", metric_sptr(new airtime_metric_ns3(enc, rts_cts_threshold)), 10)));
      proto->push_back(metric_sptr(new metric_decimator("Airtime-NS3-5PC", metric_sptr(new airtime_metric_ns3(enc, rts_cts_threshold)), 20)));

      proto->push_back(metric_sptr(new fdr_metric));
      proto->push_back(metric_sptr(new txc_metric("TXC")));

/*
      proto->push_back(metric_sptr(new elc_metric("ELC", rts_cts_threshold, cw, 0, acktimeout)));
      if(dead_time)
         proto->push_back(metric_sptr(new elc_metric("ELC-adj", rts_cts_threshold, cw, dead_time, acktimeout)));
      proto->push_back(metric_sptr(new elc_mrr_metric("ELC-MRR", rts_cts_threshold, cw, 0, acktimeout)));
      proto->push_back(metric_sptr(new legacy_elc_metric(enc, rate_Mbs * 1000, mpdu_sz, rts_cts_threshold)));
      proto->push_back(metric_sptr(new metric_damper("ELC-Damped", metric_sptr(new elc_metric("", rts_cts_threshold, cw, 0, acktimeout)), damp)));
      proto->push_back(metric_sptr(new metric_decimator("ELC-10PC", metric_sptr(new elc_metric("", rts_cts_threshold, cw, 0, acktimeout)), 10)));
      proto->push_back(metric_sptr(new metric_decimator("ELC-1PC", metric_sptr(new elc_metric("", rts_cts_threshold, cw, 0, acktimeout)), 100)));
      proto->push_back(metric_sptr(new simple_elc_metric));
*/
      metric_sptr m(new iperf_metric_wrapper(metric_sptr(new metric_demux(proto))));

      wnic_sptr w(wnic::open(what));
      w = wnic_sptr(new wnic_wallclock_fix(w));
      w = wnic_sptr(new wnic_require_timestamps(w));
      if("OFDM" == enc_str) {
         w = wnic_sptr(new wnic_encoding_fix(w, CHANNEL_CODING_OFDM | CHANNEL_PREAMBLE_LONG));
      } else if("DSSS" == enc_str) {
         w = wnic_sptr(new wnic_encoding_fix(w, CHANNEL_CODING_DSSS | CHANNEL_PREAMBLE_LONG));
      }

      buffer_sptr first(w->read()), b(first), last;
      if(b) {
         buffer_info_sptr info(b->info());
         uint64_t tick_time = UINT64_C(1000000);
         uint64_t end_time = runtime ? info->timestamp_wallclock() + (runtime * tick_time) : UINT64_MAX;
         uint64_t tick = show_ticks ? info->timestamp_wallclock() + tick_time : UINT64_MAX;
         for(uint32_t n = 0; b && (info->timestamp_wallclock() <= end_time); ++n) {
            // is it time to print results yet?
            info = b->info();
            uint64_t timestamp = info->timestamp_wallclock();
            for(; tick <= timestamp; tick += tick_time) {
               m->compute(tick, tick_time);
               cout << "Time: " << tick / tick_time << endl;
               cout << *m << endl;
               m->reset();
            }
            if(debug) { 
               cout << n << " " << *info << endl;
            }
            m->add(b);
            last = b;
            b = w->read();
         }
         if(!show_ticks) {
            uint32_t elapsed = last->info()->timestamp_wallclock() - first->info()->timestamp_wallclock();
            m->compute(tick, elapsed);
            cout << "Time: " << static_cast<double>(elapsed) / tick_time << endl;
            cout << *m << endl;
         }
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
