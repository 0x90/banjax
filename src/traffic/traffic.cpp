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

string
frame_type_as_string(frame_control fc)
{
   string s;
   switch(fc.type()) {
   case MGMT_FRAME:
      switch(fc.subtype()) {
      case MGMT_ACTION:
         s = "MGMT_ACTION";
         break;
      case MGMT_BEACON:
         s = "MGMT_BEACON";
         break;
      default:
         s = "MGMT_OTHER";
         break;
      }
      break;

   case CTRL_FRAME:
      switch(fc.subtype()) {
      case CTRL_ACK:
         s = "CTRL_ACK";
         break;
      case CTRL_CTS:
         s = "CTRL_CTS";
         break;
      case CTRL_RTS:
         s = "CTRL_RTS";
         break;
      default:
         s = "CTRL_OTHER";
         break;
      }
      break;

   case DATA_FRAME:
      s = "DATA";
      break;
   }
   return s;
}


int
main(int ac, char **av)
{
   try {

      string what;
      uint16_t cw;
      bool use_sexprs = false, verbose = false;
      options_description options("program options");
      options.add_options()
         ("help,?", "produce this help message")
         ("cw,c", value<uint16_t>(&cw)->default_value(0), "fixed CW in microseconds")
         ("input,i", value<string>(&what)->default_value("mon0"), "input file/device name")
         ("sexpr,s", value<bool>(&use_sexprs)->zero_tokens(), "write output as s-expressions")
         ("verbose,v", value<bool>(&verbose)->zero_tokens(), "write verbose output")
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

      uint_least32_t t_total = 0, t_tx = 0, t_ifs = 0;
      uint_least32_t n_ctrl = 0, t_ctrl = 0, t_ctrl_ifs = 0;
      uint_least32_t n_data = 0, t_data = 0, t_data_ifs = 0, t_data_cw = 0;
      uint_least32_t n_mgmt = 0, t_mgmt = 0, t_mgmt_ifs = 0, t_mgmt_cw = 0;
      int_least32_t t_ctrl_delta = 0, t_data_delta = 0, t_mgmt_delta = 0;
      uint_least32_t t_bad = 0, t_bad_ifs = 0, n_bad = 0;
      uint_least32_t t_iperf = 0, n_iperf = 0, sz_iperf = 0;
      uint_least32_t sz_data = 0;

      const uint32_t CRC_SZ = 4;
      buffer_sptr b(w->read());
      frame_control prev_fc;
      if(b) {
         uint64_t t1 = b->info()->timestamp1();
         uint64_t t2 = b->info()->timestamp2();

         t_total = t2 - t1;
         t_tx = t2 - t1;
         t_data = t2 - t1;

         for(uint32_t n = 2; b = w->read(); ++n) {
            frame f(b);
            data_frame_sptr df;
            frame_control fc(f.fc());
            buffer_info_sptr info(b->info()); 
            uint16_t txtime = info->timestamp2() - info->timestamp1();
            const int16_t IFS = static_cast<int16_t>(info->timestamp1() - t2);
            const int16_t DIFS =  static_cast<int16_t>(info->channel_encoding()->DIFS());
            const int16_t SIFS =  static_cast<int16_t>(info->channel_encoding()->SIFS());

            t_total += info->timestamp2() - t2;
            t_tx += txtime;
            t_ifs += IFS;

            switch(fc.type()) {
            case CTRL_FRAME:
               ++n_ctrl;
               t_ctrl += txtime;
               t_ctrl_ifs += SIFS;
               t_ctrl_delta += IFS - SIFS;
               break;
            case DATA_FRAME:
               ++n_data;
               t_data += txtime;
               sz_data += b->data_size() + CRC_SZ;
               t_data_ifs += DIFS;
               if(cw) {
                  t_data_cw += cw;
                  t_data_delta += IFS - DIFS - static_cast<int16_t>(cw);
               } else {
                  t_data_cw += IFS - DIFS;
               }
               if(verbose) {
                  cout << n << " " << info->timestamp1() << " " << IFS << " ";
                  cout << frame_type_as_string(prev_fc) << " " << frame_type_as_string(fc) << endl;
               }
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
                  sz_iperf += b->data_size() + CRC_SZ /* ToDo: adjust for UDP hdr*/;
               }
               break;
            case MGMT_FRAME:
               ++n_mgmt;
               t_mgmt += txtime;
               t_mgmt_ifs += DIFS;
               if(cw) {
                  t_mgmt_cw += cw;
                  t_mgmt_delta += IFS - DIFS - static_cast<int16_t>(cw);
               } else {
                  t_mgmt_cw += IFS - DIFS;
               }
               break;
            default:
               ++n_bad;
               t_bad += txtime;
               t_bad_ifs += IFS;
               if(verbose) {
                  cerr << "unknown frame type at frame " << n << endl;
               }
               break;
            }
            t2 = b->info()->timestamp2();
            prev_fc = fc;
         }

         double dur = t2 - t1;
         if(use_sexprs) {

            cout << "(define t-total " << t_total << ")" << endl;
            cout << "(define t-tx " << t_tx << ")" << endl;
            cout << "(define t-ifs " << t_ifs << ")" << endl;

            cout << "(define t-dur " << t2 - t1 << ")" << endl;
            cout << "(define n-ctrl " << n_ctrl <<  ")" << endl;
            cout << "(define t-ctrl " << t_ctrl  << ")" << endl;
            cout << "(define t-ctrl-ifs " << t_ctrl_ifs << ")" << endl;
            cout << "(define t-ctrl-delta " << t_ctrl_delta << ")" << endl;
            cout << "(define n-data " << n_data <<  ")" << endl;
            cout << "(define t-data " << t_data  << ")" << endl;
            cout << "(define t-data-ifs " << t_data_ifs << ")" << endl;
            cout << "(define t-data-cw " << t_data_cw << ")" << endl;
            cout << "(define t-data-delta " << t_data_delta << ")" << endl;
            cout << "(define n-mgmt " << n_mgmt <<  ")" << endl;
            cout << "(define t-mgmt " << t_mgmt  << ")" << endl;
            cout << "(define t-mgmt-ifs " << t_mgmt_ifs << ")" << endl;
            cout << "(define t-mgmt-cw " << t_mgmt_cw << ")" << endl;
            cout << "(define t-mgmt-delta " << t_mgmt_delta << ")" << endl;
            cout << "(define n-iperf " << n_iperf <<  ")" << endl;
            cout << "(define t-iperf " << t_iperf  << ")" << endl;
            if(n_bad) {
               cout << "(define t-bad " << t_bad << ")" << endl;
               cout << "(define t-bad " << t_bad_ifs  << ")" << endl;
               cout << "(define n-bad " << n_bad << ")" << endl;
            }
         } else {
            cout << "T_TX: " << t_tx << ", ";
            cout << "T_IFS: " << t_ifs << ", ";
            cout << "T_DUR: " << t2 - t1 << ", ";
            cout << "N_CTRL: " << n_ctrl << ", ";
            cout << "T_CTRL: " << t_ctrl  <<  ", ";
            cout << "T_CTRL_IFS: " << t_ctrl_ifs << ", ";
            if(t_ctrl_delta)
               cout << "T_CTRL_DELTA: " << t_ctrl_delta << ", ";
            cout << "N_DATA: " << n_data << ", ";
            cout << "T_DATA: " << t_data  << ", ";
            cout << "T_DATA_IFS: " << t_data_ifs << ", ";
            cout << "T_DATA_CW: " << t_data_cw << ", ";
            if(t_data_delta)
               cout << "T_DATA_DELTA: " << t_data_delta << ", ";
            cout << "N_MGMT: " << n_mgmt  << ", ";
            cout << "T_MGMT: " << t_mgmt  << ", ";
            cout << "T_MGMT_IFS: " << t_mgmt_ifs << ", ";
            cout << "T_MGMT_CW: " << t_mgmt_cw << ", ";
            if(t_mgmt_delta)
               cout << "T_MGMT_DELTA: " << t_mgmt_delta << ", ";
            if(n_bad) {
               cout << "T_BAD: "  <<  t_bad << ", ";
               cout << "T_BAD_IFS: " << t_bad_ifs << ", ";
               cout << "N_BAD: "  <<  n_bad << ", ";
            }
            cout << "T_TOTAL: " << t_ctrl + t_bad + t_ctrl_ifs + t_ctrl_delta +  + t_mgmt + t_mgmt_ifs + t_mgmt_cw + t_data + t_data_ifs + t_data_cw << ", N_TOTAL: " << n_ctrl + n_data + n_mgmt + n_bad << endl;
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
