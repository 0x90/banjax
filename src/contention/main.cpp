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

      string what;
      options_description options("program options");
      options.add_options()
         ("help,?", "produce this help message")
         ("input,i", value<string>(&what)->default_value("mon0"), "input file/device name")
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

      
      bool contending = false;
      uint_least32_t n_ifs = 0, t_ifs = 0;
      buffer_sptr b(w->read()), p;
      uint64_t adj =  b->info()->timestamp1();
      for(uint32_t n = 1; b; ++n){
         frame f(b);
         frame_control fc(f.fc());
         if((fc.subtype() == DATA_QOS || fc.subtype() == DATA) && contending) {
            int32_t ifs = b->info()->timestamp1() - p->info()->timestamp2();
            cout << n << " " << b->info()->timestamp1() << " " << ifs << endl;
            contending = false;
            ++n_ifs;
            t_ifs += ifs;
         } else if(fc.subtype() == CTRL_ACK) {
            p = b;
            contending = true;
         } else {
            contending = false;
         }
         b = w->read();
      }
      cerr << "AVG CONTENTION TIME = " << (t_ifs / static_cast<double>(n_ifs)) << endl;
   } catch(const error& x) {
      cerr << x.what() << endl;
   } catch(const std::exception& x) {
      cerr << x.what() << endl;
   } catch(...) {
      cerr << "unhandled exception!" << endl;
   }
   exit(EXIT_FAILURE);
}
