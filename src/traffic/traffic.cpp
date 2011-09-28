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

      uint_least32_t n_ctrl = 0, t_ctrl = 0, t_ctrl_ifs = 0, t_ctrl_delta = 0;
      uint_least32_t n_data = 0, t_data = 0, t_data_ifs = 0, t_data_cw = 0;
      uint_least32_t n_mgmt = 0, t_mgmt = 0, t_mgmt_ifs = 0, t_mgmt_cw = 0;

      uint_least32_t t_bad = 0, n_bad = 0;

      uint_least32_t t_iperf = 0, n_iperf = 0, sz_iperf = 0;
      uint_least32_t sz_data = 0;

      const uint32_t CRC_SZ = 4;
      map<eui_48, uint16_t> seq_nos;
      map<eui_48, uint16_t>::iterator seq_no;
      buffer_sptr b(w->read());
      if(b) {
         // Note: The measurement begins from the end of the first frame.
         uint64_t t1 = b->info()->timestamp2(), t2 = t1;
         for(uint32_t n = 2; b = w->read(); ++n){
            frame f(b);
            data_frame_sptr df;
            frame_control fc(f.fc());
            buffer_info_sptr info(b->info()); 
            uint16_t txtime = info->timestamp2() - info->timestamp1();
            switch(fc.type()) {
            case CTRL_FRAME:
               ++n_ctrl;
               t_ctrl += txtime;
               t_ctrl_ifs += info->channel_encoding()->SIFS();
               t_ctrl_delta += info->timestamp1() - t2 - info->channel_encoding()->SIFS();
               break;
            case DATA_FRAME:
               ++n_data;
               t_data += txtime;
               sz_data += b->data_size() + CRC_SZ;
               t_data_ifs += info->channel_encoding()->DIFS();
               t_data_cw += info->timestamp1() - t2 - info->channel_encoding()->DIFS();

               cerr << n << " " << info->timestamp1() - t2 << endl;

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
               ++n_mgmt;
               t_mgmt += txtime;
               t_mgmt_ifs += info->channel_encoding()->DIFS();
               t_mgmt_cw += info->timestamp1() - t2 - info->channel_encoding()->DIFS();
               break;
            default:
               ++n_bad;
               t_bad += 0;
               // NB: even "bad" frames would have some IFS and maybe CW time!
               break;
            }
            t2 = b->info()->timestamp2();
         }

         double dur = t2 - t1;
         if(use_sexprs) {
            cout << "(define t-dur " << t2 - t1 << ")" << endl;
            cout << "(define n-ctrl " << n_ctrl <<  ")" << endl;
            cout << "(define t-ctrl " << t_ctrl  << ")" << endl;
            cout << "(define t-ctrl-ifs " << t_ctrl_ifs << ")" << endl;
            cout << "(define t-ctrl-delta " << t_ctrl_delta << ")" << endl;
            cout << "(define n-data " << n_data <<  ")" << endl;
            cout << "(define t-data " << t_data  << ")" << endl;
            cout << "(define t-data-ifs " << t_data_ifs << ")" << endl;
            cout << "(define t-data-cw " << t_data_cw << ")" << endl;
            cout << "(define n-mgmt " << n_mgmt <<  ")" << endl;
            cout << "(define t-mgmt " << t_mgmt  << ")" << endl;
            cout << "(define t-mgmt-ifs " << t_mgmt_ifs << ")" << endl;
            cout << "(define t-mgmt-cw " << t_mgmt_cw << ")" << endl;
            cout << "(define n-iperf " << n_iperf <<  ")" << endl;
            cout << "(define t-iperf " << t_iperf  << ")" << endl;
         } else {
            cout << "T_DUR: " << t2 - t1 << ", ";
            cout << "N_CTRL: " << n_ctrl << ", ";
            cout << "T_CTRL: " << t_ctrl  <<  ", ";
            cout << "T_CTRL_IFS: " << t_ctrl_ifs << ", ";
            cout << "T_CTRL_DELTA: " << t_ctrl_delta << ", ";
            cout << "N_DATA: " << n_data << ", ";
            cout << "T_DATA: " << t_data  << ", ";
            cout << "T_DATA_IFS: " << t_data_ifs << ", ";
            cout << "T_DATA_CW: " << t_data_cw << ", ";
            cout << "N_MGMT: " << n_mgmt  << ", ";
            cout << "T_MGMT: " << t_mgmt  << ", ";
            cout << "T_MGMT_IFS: " << t_mgmt_ifs << ", ";
            cout << "T_MGMT_CW: " << t_mgmt_cw << ", ";
            cout << "T_TOTAL: " << t_ctrl + t_bad + t_ctrl_ifs + t_ctrl_delta +  + t_mgmt + t_mgmt_ifs + t_mgmt_cw + t_data + t_data_ifs + t_data_cw << ", N_TOTAL: " << n_ctrl + n_data + n_mgmt + n_bad << endl;
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
