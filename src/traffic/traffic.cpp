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

      uint_least32_t t_ctrl = 0, n_ctrl = 0;
      uint_least32_t t_data = 0, n_data = 0;
      uint_least32_t t_mgmt = 0, n_mgmt = 0;
      uint_least32_t  t_bad = 0,  n_bad = 0;
      uint_least32_t t_iperf = 0, n_iperf = 0, sz_iperf = 0;
      
      uint_least32_t sz_data = 0;

      map<eui_48, uint16_t> seq_nos;
      map<eui_48, uint16_t>::iterator seq_no;

      const uint32_t CRC_SZ = 4;
      buffer_sptr b(w->read());
      uint64_t t1 = b->info()->timestamp1(), t2 = t1;
      for(uint32_t n = 1; b; ++n){
         frame f(b);
         data_frame_sptr df;
         buffer_info_sptr info(b->info()); 
         uint16_t txtime = info->timestamp2() - info->timestamp1();
         frame_control fc(f.fc());
         switch(fc.type()) {
         case CTRL_FRAME:
            ++n_ctrl;
            t_ctrl += txtime;
            t_ctrl += info->channel_encoding()->SIFS();
            break;
         case DATA_FRAME:
            ++n_data;
            t_data += txtime;
            t_data += info->channel_encoding()->DIFS();
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
               // update totals
               ++n_iperf;
               t_iperf += txtime;
               t_iperf += info->channel_encoding()->DIFS();
               sz_iperf += b->data_size() + CRC_SZ;
            }
            break;
         case MGMT_FRAME:
            ++n_mgmt;
            t_mgmt += txtime;
            t_mgmt += info->channel_encoding()->SIFS();
            break;
         default:
            ++n_bad;
            t_bad += 0;
            break;
         }
         t2 = b->info()->timestamp2();
         b = w->read();
      }

      double dur = t2 - t1;
      double cw = n_data * 85.5;


      cout << ", TIME, COUNT, AVG TIME, % TIME, AVG SIZE" << endl;

      cout << " CTRL: " << t_ctrl  << ", " << n_ctrl <<  ", " <<  t_ctrl / static_cast<double>(n_ctrl)  << ", " << (t_ctrl / dur) * 100.0  << "%" << endl;
      cout << " DATA: " << t_data  << ", " << n_data <<  ", " <<  t_data / static_cast<double>(n_data)  << ", " << (t_data / dur) * 100.0  << "%, " << sz_data / n_data << endl;
      cout << "IPERF: " << t_iperf << ", " << n_iperf << ", " <<  t_data / static_cast<double>(n_iperf) << ", " << (t_iperf / dur) * 100.0 << "%, " << sz_iperf / n_iperf << endl;
      cout << " MGMT: " << t_mgmt  << ", " << n_mgmt <<  ", " <<  t_mgmt / static_cast<double>(n_mgmt)  << ", " << (t_mgmt / dur) / 100.0  << "%" << endl;
      cout << "  BAD: " <<  t_bad  << ", " <<  n_bad <<  ", " <<   t_bad /  static_cast<double>(n_bad)  << "%" << endl;
      cout << endl;
      cout << "TOTAL: " << t_ctrl + t_data + t_mgmt + t_bad << ", " << n_ctrl + n_data + n_mgmt + n_bad << endl;
      cout << "ESTCW: " << cw << ", " << (cw / dur) * 100.0 << "%" << endl;
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
