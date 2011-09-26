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
      bool use_sexprs;
      options_description options("program options");
      options.add_options()
         ("help,?", "produce this help message")
         ("input,i", value<string>(&what)->default_value("mon0"), "input file/device name")
         ("scheme,s", value<bool>(&use_sexprs)->zero_tokens(), "write output as scheme definitions")
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

      uint_least32_t t_ctrl = 0, n_ctrl = 0;
      uint_least32_t t_data = 0, n_data = 0;
      uint_least32_t t_mgmt = 0, n_mgmt = 0;
      uint_least32_t t_bad = 0, n_bad = 0;
      uint_least32_t t_ifs = 0, n_ifs = 0;
      uint_least32_t t_cw = 0, n_cw = 0;
      uint_least32_t t_delta = 0, n_delta = 0;
      uint_least32_t t_iperf = 0, n_iperf = 0, sz_iperf = 0;
      uint_least32_t sz_data = 0;

      const uint32_t CRC_SZ = 4;
      map<eui_48, uint16_t> seq_nos;
      map<eui_48, uint16_t>::iterator seq_no;
      buffer_sptr b(w->read());
      if(b) {
         uint64_t t1 = b->info()->timestamp1();
         uint64_t t2 = b->info()->timestamp2();
         for(uint32_t n = 1; b; ++n){
            frame f(b);
            uint16_t ifs = 0;
            data_frame_sptr df;
            buffer_info_sptr info(b->info()); 
            uint16_t txtime = info->timestamp2() - info->timestamp1();
            frame_control fc(f.fc());
            switch(fc.type()) {
            case CTRL_FRAME:
               ++n_ifs;
               t_ifs += info->channel_encoding()->SIFS();
               ++n_delta;
               t_delta += info->timestamp1() - t2 - info->channel_encoding()->SIFS();
               ++n_ctrl;
               t_ctrl += txtime;
               break;
            case DATA_FRAME:
               ++n_ifs;
               t_ifs += info->channel_encoding()->DIFS();
               ++n_cw;
               t_cw += info->timestamp1() - t2 - info->channel_encoding()->DIFS();
               ++n_data;
               t_data += txtime;
               sz_data += b->data_size() + CRC_SZ;
               if(df = f.as_data_frame()) {
                  // iperf?
                  llc_hdr_sptr llc(df->get_llc_hdr());
                  if(!llc)
                     break;
                  ip_hdr_sptr ip(llc->get_ip_hdr());
                  if(!ip)
                     break;
                  udp_hdr_sptr udp(ip->get_udp_hdr());
                  if(!udp)
                     break;
                  if(udp->dst_port() != 5001)
                     break;
                  // increment iperf counts
                  ++n_iperf;
                  t_iperf += txtime;
                  sz_iperf += b->data_size() + CRC_SZ;
               }
               break;
            case MGMT_FRAME:
               ++n_ifs;
               t_ifs += info->channel_encoding()->DIFS();
               ++n_cw;
               t_cw += info->timestamp1() - t2 - info->channel_encoding()->DIFS();
               ++n_mgmt;
               t_mgmt += txtime;
               break;
            default:
               ++n_bad;
               t_bad += 0;
               // NB: even "bad" frames would have some IFS and maybe CW time!
               break;
            }
            t2 = b->info()->timestamp2();
            b = w->read();
         }

         double dur = t2 - t1;
         if(use_sexprs) {
            cout << "(define t-dur " << t2 - t1 << ")" << endl;
            cout << "(define t-ctrl " << t_ctrl  << ")" << endl;
            cout << "(define n-ctrl " << n_ctrl <<  ")" << endl;
            cout << "(define t-data " << t_data  << ")" << endl;
            cout << "(define n-data " << n_data <<  ")" << endl;
            cout << "(define t-mgmt " << t_mgmt  << ")" << endl;
            cout << "(define n-mgmt " << n_mgmt <<  ")" << endl;
            cout << "(define t-ifs " << t_ifs << ")" << endl;
            cout << "(define n-ifs " << n_ifs << ")" << endl;
            cout << "(define t-cw " << t_cw << ")" << endl;
            cout << "(define n-cw " << n_cw << ")" << endl;
            cout << "(define t-delta " << t_delta << ")" << endl;
            cout << "(define n-delta " << n_delta << ")" << endl;
         } else {
            cout << "T_DUR: " << t2 - t1 << ", ";
            cout << "T_CTRL: " << t_ctrl  << ", N_CTRL: " << n_ctrl <<  ", ";
            cout << "T_DATA: " << t_data  << ", N_DATA: " << n_data <<  ", ";
            cout << "T_MGMT: " << t_mgmt  << ", N_MGMT: " << n_mgmt <<  ", ";
            cout << "T_IFS: " << t_ifs << ", N_IFS: " << n_ifs << ", ";
            cout << "T_CW: " << t_cw << ", N_CW: " << n_cw << ", ";
            cout << "T_DELTA: " << t_delta << ", N_DELTA: " << n_delta << ", ";
            cout << "T_TOTAL: " << t_ctrl + t_data + t_mgmt + t_bad + t_ifs + t_cw + t_delta << ", N_TOTAL: " << n_ctrl + n_data + n_mgmt + n_bad << endl;
            if(n_bad) {
               cerr << "unknown frames found in capture!" << endl;
               cerr << " T_BAD: "  <<  t_bad  << ", N_BAD: "  <<  n_bad << ", ";
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
