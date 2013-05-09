/* -*- Mode: C++; tab-width: 3; -*- */

/* 
 * Copyright NICTA, 2013
 */

#define __STDC_LIMIT_MACROS
#include <net/buffer_info.hpp>
#include <net/wnic.hpp>
#include <net/wnic_encoding_fix.hpp>
#include <net/wnic_timestamp_fix.hpp>
#include <net/wnic_timestamp_swizzle.hpp>
#include <dot11/data_frame.hpp>
#include <dot11/frame.hpp>
#include <dot11/sequence_control.hpp>

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

bool
is_iperf_frame(frame& f)
{
   data_frame_sptr df(f.as_data_frame());
   if(!df)
      return false;
   llc_hdr_sptr llc(df->get_llc_hdr());
   if(!llc)
      return false;
   ip_hdr_sptr ip(llc->get_ip_hdr());
   if(!ip)
      return false;
   udp_hdr_sptr udp(ip->get_udp_hdr());
   if(!udp)
      return false;
   if(udp->dst_port() != 5001)
      return false;
   return true;
}

int
main(int ac, char **av)
{
   try {

      uint64_t runtime;
      string what, ta_str;
      bool debug, relax, verbose;
      string enc_str;
      options_description options("program options");
      options.add_options()
         ("help,?", "produce this help message")
         ("debug,d", value<bool>(&debug)->default_value(false)->zero_tokens(), "enable debugging")
         ("encoding", value<string>(&enc_str)->default_value("OFDM"), "channel encoding")
         ("input,i", value<string>(&what)->default_value("mon0"), "input file/device name")
         ("relax,r", value<bool>(&relax)->default_value(false)->zero_tokens(), "relax packet membership test")
         ("runtime,u", value<uint64_t>(&runtime)->default_value(0), "finish after n seconds")
         ("ta,a", value<string>(&ta_str)->default_value("00:0b:6b:0a:82:34"), "transmitter address")
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
      w = wnic_sptr(new wnic_encoding_fix(w, CHANNEL_CODING_OFDM | CHANNEL_PREAMBLE_LONG));
      w = wnic_sptr(new wnic_timestamp_swizzle(w));
      w = wnic_sptr(new wnic_timestamp_fix(w));
      if("OFDM" == enc_str) {
         w = wnic_sptr(new wnic_encoding_fix(w, CHANNEL_CODING_OFDM | CHANNEL_PREAMBLE_LONG));
      } else if("DSSS" == enc_str) {
         w = wnic_sptr(new wnic_encoding_fix(w, CHANNEL_CODING_DSSS | CHANNEL_PREAMBLE_LONG));
      }

      buffer_sptr b(w->read());
      if(b) {
         eui_48 ta(ta_str.c_str());
         uint16_t seq_no = 0;
         buffer_sptr first, last;
         uint64_t tick_time = UINT64_C(1000000);
         uint64_t start_time = b->info()->timestamp1();
         uint64_t end_time = runtime ? b->info()->timestamp1() + (runtime * tick_time) : UINT64_MAX;
         for(uint32_t n = 2; (b = buffer_sptr(w->read())) && (b->info()->timestamp1() <= end_time); ++n){
            frame f(b);

            if((f.fc().type() == DATA_FRAME) && (f.address2() == ta) && is_iperf_frame(f)) {
               sequence_control sc = f.sc();
               if(first) {
                  if(sc.sequence_no() == seq_no) {
                     last = b;
                  } else {
                     uint64_t ts = first->info()->timestamp1();
                     for(; tick <= ts; tick += tick_time) {
                        // compute per-second counters
                        cout << "Time: " << (tick - start_time) / tick_time << ", " << *metrics << endl;
                        // reset per-second counters
                     }
                     if(verbose) {
                        uint32_t pkttime = last->info()->timestamp2() - first->info()->timestamp1();
                        uint32_t AIFS = 43;
                        float CW = 67.5;
                        cout << n << " " << CW + pkttime + AIFS << endl;
                     }
                     first = last = b;
                     seq_no = sc.sequence_no();
                  }
               } else {
                  first = last = b;
                  seq_no = sc.sequence_no();
               }
            } else if(relax || (f.fc().subtype() == CTRL_ACK && f.address1() == ta)) {
               last = b;
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
