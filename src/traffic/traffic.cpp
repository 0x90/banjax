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
#include <dot11/data_frame.hpp>

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

      uint_least32_t ctrl_time = 0;
      uint_least32_t data_time = 0;
      uint_least32_t mgmt_time = 0;
      uint_least32_t bad_time = 0;
      
      buffer_sptr b;
      for(uint32_t n = 1; b = w->read(); ++n){
         frame f(b);
         data_frame_sptr data;
         buffer_info_sptr info(b->info()); 
         uint16_t txtime = info->timestamp2() - info->timestamp1();
         frame_control fc(f.fc());
         switch(fc.type()) {
         case CTRL_FRAME:
            ctrl_time += txtime;
            ctrl_time += info->channel_encoding()->SIFS();
            break;
         case DATA_FRAME:
            data_time += txtime;
            data_time += info->channel_encoding()->DIFS();
            data = f.as_data_frame();
            
            break;
         case MGMT_FRAME:
            mgmt_time += txtime;
            mgmt_time += info->channel_encoding()->SIFS();
            break;
         default:
            bad_time += 0;
            break;
         }
      }

      cout << " CTRL: " << ctrl_time << endl;
      cout << " DATA: " << data_time << endl;
      cout << " MGMT: " << mgmt_time << endl;
      cout << "  BAD: " <<  bad_time << endl;
      cout << endl;
      cout << "TOTAL: " << ctrl_time + data_time + mgmt_time + bad_time << endl;
      cout << endl;

   } catch(const error& x) {
      cerr << x.what() << endl;
   } catch(const std::exception& x) {
      cerr << x.what() << endl;
   } catch(...) {
      cerr << "unhandled exception!" << endl;
   }
   exit(EXIT_FAILURE);
}
