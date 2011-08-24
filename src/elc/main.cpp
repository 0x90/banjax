/* -*- Mode: C++; tab-width: 3; -*- */

/* 
 * Copyright NICTA, 2011
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <elc_metric.hpp>
#include <elc_mrr_metric.hpp>
// #include <ett_metric.hpp>
#include <etx_metric.hpp>
#include <goodput_metric.hpp>
#include <legacy_elc_metric.hpp>
#include <metric_demux.hpp>
#include <metric_group.hpp>
#include <metric.hpp>
#include <pdr_metric.hpp>
#include <pktsz_metric.hpp>
#include <txc_metric.hpp>

#include <net/buffer_info.hpp>
#include <net/ofdm_encoding.hpp>
#include <net/wnic.hpp>
#include <net/wnic_encoding_fix.hpp>
#include <net/wnic_wallclock_fix.hpp>

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

      bool help;
      uint16_t cw;
      string enc_str, what;
      uint16_t port_no;
      uint16_t rts_cts_threshold;

      options_description options("program options");
      options.add_options()
         ("help,?", value(&help)->default_value(false)->zero_tokens(), "produce this help message")
         ("cw,c", value(&cw)->default_value(0), "size of contention window in microseconds (0 = compute average)")
         ("encoding,e", value<string>(&enc_str)->default_value("OFDM"), "channel encoding")
         ("input,i", value<string>(&what)->default_value("mon0"), "input file/device name")
         ("port,p", value<uint16_t>(&port_no)->default_value(50000), "port number used for ETX probes")
         ("rts-threshold,r", value<uint16_t>(&rts_cts_threshold)->default_value(UINT16_MAX), "RTS threshold level")
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
      proto->push_back(metric_sptr(new goodput_metric));
      proto->push_back(metric_sptr(new elc_metric(rts_cts_threshold, cw)));
      proto->push_back(metric_sptr(new elc_mrr_metric(rts_cts_threshold)));
      proto->push_back(metric_sptr(new legacy_elc_metric(enc)));
      proto->push_back(metric_sptr(new txc_metric));
      metric_sptr m(metric_sptr(new metric_demux(proto)));

      wnic_sptr w(wnic::open(what));
      w = wnic_sptr(new wnic_wallclock_fix(w));
      w = wnic_sptr(new wnic_encoding_fix(w, CHANNEL_CODING_OFDM | CHANNEL_PREAMBLE_LONG)); // ToDo: use cmd line opt to choose default coding!
      buffer_sptr b(w->read());
      buffer_info_sptr info(b->info());
      const uint64_t uS_PER_TICK = UINT64_C(1000000);
      uint64_t tick = info->timestamp_wallclock() + uS_PER_TICK;
      for(b; b = w->read();){
         // is it time to print results yet?
         info = b->info();
         uint64_t timestamp = info->timestamp_wallclock();
         for(; tick <= timestamp; tick += uS_PER_TICK) {
            m->compute(uS_PER_TICK);
            cout << "TIME: " << tick / uS_PER_TICK << endl;
            cout << *m << endl;
            m->reset();
         }
         // update metric with frame
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
