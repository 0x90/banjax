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
#include <dot11/data_frame.hpp>
#include <dot11/frame.hpp>
#include <dot11/ip_hdr.hpp>

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
      bool use_sexprs, verbose;
      options_description options("program options");
      options.add_options()
         ("help,?", "produce this help message")
         ("input,i", value<string>(&what)->default_value("mon0"), "input file/device name")
         ("verbose,v", value<bool>(&verbose)->default_value(false)->zero_tokens(), "show TXC per packet")
         ;

      variables_map vars;       
      store(parse_command_line(ac, av, options), vars);
      notify(vars);   

      if(vars.count("help")) {
         cout << options << endl;
         exit(EXIT_SUCCESS);
      }

      wnic_sptr w(wnic::open(what));
#if 0
      double w[W_MAX];
      fill(&w[0], &w[W_MAX], 0.0);
#endif
      buffer_sptr b;
      uint_least32_t packets = 0, txs = 0, max_txc = 0, min_txc = UINT32_MAX;
      for(uint32_t n = 1; b = w->read(); ++n){
         frame f(b);
         buffer_info_sptr info(b->info());
         if(info->has(TX_FLAGS) && info->has(DATA_RETRIES)) {
            uint txc = 1 + info->data_retries();
            max_txc = max(max_txc, txc);
            min_txc = min(min_txc, txc);
            txs += txc;
            ++packets;
            
#if 0
            for(uint16_t i = 0; i < txc; ++i) {
               w[i] += 1 / static_cast<double>(1 << (txc - i));
            }
#endif
            if(verbose)
               cout << n << " " << txc << endl;
         }
      }

      cerr << "txc: " << txs / static_cast<double>(packets) << ", ";
      cerr << "min txc: " << min_txc << ", ";
      cerr << "max txc: " << max_txc << endl;

   } catch(const error& x) {
      cerr << x.what() << endl;
   } catch(const std::exception& x) {
      cerr << x.what() << endl;
   } catch(...) {
      cerr << "unhandled exception!" << endl;
   }
   exit(EXIT_FAILURE);
}
