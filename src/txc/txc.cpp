/* -*- Mode: C++; tab-width: 3; -*- */

/* 
 * Copyright NICTA, 2011
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <dot11/data_frame.hpp>
#include <dot11/frame.hpp>
#include <dot11/ip_hdr.hpp>
#include <dot11/llc_hdr.hpp>
#include <dot11/udp_hdr.hpp>
#include <net/buffer_info.hpp>
#include <net/wnic.hpp>
#include <net/wnic_encoding_fix.hpp>
#include <net/wnic_timestamp_fix.hpp>
#include <net/wnic_timestamp_swizzle.hpp>

#include <boost/program_options.hpp>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <vector>

using namespace boost;
using namespace boost::program_options;
using namespace dot11;
using namespace net;
using namespace std;


void
update(uint16_t n, double v, vector<double>& cw)
{
   if(0 == n) {
      cw[n] += v;
   } else if(cw.size() - 1 < n) {
      update(n - 1, v, cw);  
   } else {
      v /= 2.0;
      cw[n] += v;
      update(n - 1, v, cw);
   }
}


int
main(int ac, char **av)
{
   try {
      uint64_t runtime;
      string what, enc_str;
      bool debug, dist, stats, use_sexprs, verbose;
      options_description options("program options");
      options.add_options()
         ("help,?", "produce this help message")
         ("debug,g", value<bool>(&debug)->default_value(false)->zero_tokens(), "enable debug")
         ("dist,d", value<bool>(&dist)->default_value(false)->zero_tokens(), "show tx distribution")
         ("encoding,e", value<string>(&enc_str)->default_value("OFDM"), "channel encoding")
         ("input,i", value<string>(&what)->default_value("mon0"), "input file/device name")
         ("runtime,u", value<uint64_t>(&runtime)->default_value(0), "produce results after n seconds")
         ("stats,s", value<bool>(&stats)->default_value(false)->zero_tokens(), "show txc stats")
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
      vector<double> cw(6);
      buffer_sptr b(w->read());
      if(b) {
         uint64_t tick_time = UINT64_C(1000000);
         uint64_t end_time = runtime ? b->info()->timestamp_wallclock() + (runtime * tick_time) : UINT64_MAX;
         uint_least32_t nof_txs = 0, nof_pkts = 0, max_txc = 0, min_txc = UINT32_MAX;
         for(uint32_t n = 1; b && (b->info()->timestamp_wallclock() <= end_time); b = w->read(), ++n) {
            frame f(b);
            buffer_info_sptr info(b->info());

            // use only iperf traffic!
            data_frame_sptr df(f.as_data_frame());
            if(!df)
               continue;

            llc_hdr_sptr llc(df->get_llc_hdr());
            if(!llc)
               continue;

            ip_hdr_sptr ip(llc->get_ip_hdr());
            if(!ip)
               continue;

            udp_hdr_sptr udp(ip->get_udp_hdr());
            if(!udp)
               continue;

            if(udp->dst_port() != 5001)
               continue;

            if(info->has(TX_FLAGS) && info->has(DATA_RETRIES)) {
               uint txc = 1 + info->data_retries();
               max_txc = max(max_txc, txc);
               min_txc = min(min_txc, txc);
               nof_txs += txc;
               ++nof_pkts;
               for(size_t i = 0; i < txc; ++i)
                  update(i, 1.0, cw);
            }
            if(debug)
               cout << n << " " << *info << endl;
         }
         if(dist) {
            uint16_t lo = 0;
            uint16_t hi = 0;
            for(size_t i = min_txc; i < min(static_cast<uint_least32_t>(cw.size()), max_txc); ++i) {
               lo = hi;
               hi = (1 << (3 + i));
               cout << lo << " " << cw[i] << endl;
               cout << hi - 1 << " " << cw[i] << endl;
            }
         }
         if(stats) {
            cout << "txc: " << nof_txs / static_cast<double>(nof_pkts) << ", ";
            cout << "min txc: " << min_txc << ", ";
            cout << "max txc: " << max_txc << endl;
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
