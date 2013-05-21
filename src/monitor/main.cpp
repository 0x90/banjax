/* -*- Mode: C++; tab-width: 3; -*- */

/* 
 * Copyright NICTA, 2013
 */

#define __STDC_LIMIT_MACROS
#include <net/buffer_info.hpp>
#include <net/wnic.hpp>
#include <net/wnic_encoding_fix.hpp>
#include <net/wnic_frame_aggregator.hpp>
#include <net/wnic_timestamp_fix.hpp>
#include <net/wnic_timestamp_swizzle.hpp>

#include <boost/program_options.hpp>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <unistd.h>

using namespace boost;
using namespace boost::program_options;
using namespace net;
using namespace std;

int
main(int ac, char **av)
{
   try {

      uint64_t runtime;
      string what, ta;
      bool debug, verbose;
      string enc_str;
      options_description options("program options");
      options.add_options()
         ("help,?", "produce this help message")
         ("debug,g", value<bool>(&debug)->default_value(false)->zero_tokens(), "enable debugging")
         ("encoding", value<string>(&enc_str)->default_value("OFDM"), "channel encoding")
         ("input,i", value<string>(&what)->default_value("mon0"), "input file/device name")
         ("runtime,u", value<uint64_t>(&runtime)->default_value(0), "finish after n seconds")
         ("ta,a", value<string>(&ta)->default_value("00:0b:6b:0a:82:34"), "transmitter address")
         ("verbose,v", value<bool>(&verbose)->default_value(false)->zero_tokens(), "enable verbose output")
         ;

      variables_map vars;       
      store(parse_command_line(ac, av, options), vars);
      notify(vars);   
      if(vars.count("help")) {
         cout << options << endl;
         exit(EXIT_SUCCESS);
      }

      wnic_sptr w(wnic::open(what));
      if("OFDM" == enc_str) {
         w = wnic_sptr(new wnic_encoding_fix(w, CHANNEL_CODING_OFDM | CHANNEL_PREAMBLE_LONG));
      } else if("DSSS" == enc_str) {
         w = wnic_sptr(new wnic_encoding_fix(w, CHANNEL_CODING_DSSS | CHANNEL_PREAMBLE_LONG));
      } else {
         cerr << enc_str << " is not a recognized encoding" << endl;
      }
      w = wnic_sptr(new wnic_timestamp_swizzle(w));
      w = wnic_sptr(new wnic_timestamp_fix(w));
      w = wnic_sptr(new wnic_frame_aggregator(w, eui_48(ta.c_str())));

      buffer_sptr b(w->read());
      if(b) {
         uint16_t seq_no = 0;
         buffer_sptr first, last;
         uint64_t tick_time = UINT64_C(1000000);
         uint64_t start_time = b->info()->timestamp1();
         uint64_t end_time = runtime ? b->info()->timestamp1() + (runtime * tick_time) : UINT64_MAX;
         for(uint32_t n = 2; (b = buffer_sptr(w->read())) && (b->info()->timestamp1() <= end_time); ++n){

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
