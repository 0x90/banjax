/* -*- Mode: C++; tab-width: 3; -*- */

/* 
 * Copyright NICTA, 2011
 */

#define __STDC_LIMIT_MACROS
#include <net/buffer_info.hpp>
#include <net/wnic.hpp>
#include <net/wnic_encoding_fix.hpp>
#include <net/wnic_timestamp_fix.hpp>
#include <net/wnic_timestamp_swizzle.hpp>
#include <dot11/frame.hpp>

#include <boost/program_options.hpp>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <unistd.h>

using namespace boost;
using namespace boost::program_options;
using namespace dot11;
using namespace net;
using namespace std;

int
main(int ac, char **av)
{
   try {

      bool debug;
      uint64_t runtime;
      string what, ta_str;
      options_description options("program options");
      options.add_options()
         ("help,?", "produce this help message")
         ("debug,g", value<bool>(&debug)->default_value(false)->zero_tokens(), "enable debug")
         ("input,i", value<string>(&what)->default_value("mon0"), "input file/device name")
         ("runtime,u", value<uint64_t>(&runtime)->default_value(0), "finish after n seconds")
         ("ta,a", value<string>(&ta_str)->default_value("00:0b:6b:0a:82:34"), "transmitter address")
         ;

      variables_map vars;       
      store(parse_command_line(ac, av, options), vars);
      notify(vars);   

      if(vars.count("help")) {
         cout << options << endl;
         exit(EXIT_SUCCESS);
      }

      wnic_sptr w(wnic::open(what));
      w = wnic_sptr(new wnic_encoding_fix(w, CHANNEL_CODING_OFDM | CHANNEL_PREAMBLE_LONG));
      w = wnic_sptr(new wnic_timestamp_swizzle(w));
      w = wnic_sptr(new wnic_timestamp_fix(w));

      eui_48 ta(ta_str.data());
      buffer_sptr b(w->read());
      if(b) {
         buffer_sptr p = b;
         uint64_t tick_time = UINT64_C(1000000);
         uint64_t end_time = runtime ? b->info()->timestamp_wallclock() + (runtime * tick_time) : UINT64_MAX;
         for(uint32_t n = 2; (b = buffer_sptr(w->read())) && (b->info()->timestamp_wallclock() <= end_time); p = b, ++n){
            frame curr(b);
            frame prev(p);
            if((curr.fc().subtype() == MGMT_BEACON) && (curr.address2() == ta)) {
               cout << n << " B " << b->info()->timestamp1() - p->info()->timestamp2() << endl;
            }
            if((prev.fc().subtype() == MGMT_BEACON) && (prev.address2() == ta)) {
               cout << n - 1 << " A " << b->info()->timestamp1() - p->info()->timestamp2() << endl;
            }
            if(debug) {
               cout << n << " " << *(b->info()) << endl;
            }
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
