/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#define __STDC_LIMIT_MACROS 1

#include <dot11/frame.hpp>
#include <dot11/data_frame.hpp>
#include <dot11/frame_control.hpp>
#include <net/buffer.hpp>
#include <net/buffer_info.hpp> 
#include <net/encoding.hpp>
#include <net/eui_48.hpp>
#include <net/wnic.hpp>
#include <net/wnic_timestamp_fix.hpp>
#include <net/wnic_timestamp_swizzle.hpp>

#include <boost/program_options.hpp>
#include <cstdlib>
#include <stdint.h>
#include <iomanip>
#include <iostream>
#include <unistd.h>

using namespace boost::program_options;
using namespace dot11;
using namespace net;
using namespace std;

double 
frame_succ_time(buffer_sptr b)
{
   buffer_info_sptr info(b->info());
   encoding_sptr enc(info->channel_encoding());

   const uint32_t CRC_SZ = 4;
   const uint32_t FRAME_SZ = b->data_size() + CRC_SZ;
   const bool PREAMBLE =  info->has(CHANNEL_FLAGS) && (info->channel_flags() & CHANNEL_PREAMBLE_SHORT);
   const uint32_t T_RTS_CTS = 0;
   const uint32_t DATA_RATE = info->rate_Kbs();
   const uint32_t T_DATA = enc->txtime(FRAME_SZ, DATA_RATE, PREAMBLE);
   const uint32_t ACK_SZ = 14;
   const uint32_t ACK_RATE = enc->response_rate(DATA_RATE);
   const uint32_t T_ACK = enc->txtime(ACK_SZ, ACK_RATE, PREAMBLE);

   return T_RTS_CTS + T_DATA + enc->SIFS() + T_ACK + enc->DIFS();
}

int
main(int ac, char **av)
{
   try {

      bool help;
      string enc_str, what;
      options_description options("program options");
      options.add_options()
         ("help,?", value(&help)->default_value(false)->zero_tokens(), "produce this help message")
         ("encoding,e", value<string>(&enc_str)->default_value("OFDM"), "channel encoding")
         ("input,i", value<string>(&what)->default_value("mon0"), "input file/device name")
         ;

      variables_map vars;       
      store(parse_command_line(ac, av, options), vars);
      notify(vars);   

      if(help) {
         cout << options << endl;
         exit(EXIT_SUCCESS);
      }

      wnic_sptr w = wnic::open(what);
      w = wnic_sptr(new wnic_timestamp_swizzle(w));
      w = wnic_sptr(new wnic_timestamp_fix(w));

      uint32_t n1 = 0;
      uint_least32_t x1 = 0;

      uint32_t n2 = 0;
      uint_least32_t x2 = 0;

      uint32_t n3 = 0;
      uint_least32_t x3 = 0;

      buffer_sptr b, l, null;
      for(uint_least32_t i = 1; b = w->read(); ++i)  {
         frame f(b);
         frame_control fc(f.fc());
         switch(fc.subtype()) {
         case CTRL_ACK:
            if(l) {
               const uint32_t T_ACK_ACK = b->info()->timestamp2() - l->info()->timestamp2();
               x1 += T_ACK_ACK;
               ++n1;
               // cout << T_ACK_ACK << endl;
            }
            l = b;
            break;
         case DATA_QOS:
            if(!f.address1().is_special()) {
               const uint32_t T_FRM_SUCC = frame_succ_time(b);
               x2 += T_FRM_SUCC;
               ++n2;
               if(l) {
                  const uint32_t T_DATA_ACK = (b->info()->timestamp2() - l->info()->timestamp2());
                  x3 += T_DATA_ACK;
                  ++n3;
                  cout << T_DATA_ACK << endl;
               }
            }
            break;
         default:
            l = null;
            break;
         }
      }

      encoding_sptr enc(encoding::get(enc_str));
      const double PKT_SUCC = x1 / static_cast<double>(n1);
      const double FRM_SUCC = x2 / static_cast<double>(n2);
      cout << endl << endl;
      cout << "PKT_SUCC: " << PKT_SUCC << ", ";
      cout << "FRM_SUCC: " << FRM_SUCC << ", ";
      cout << "DIFF: " << PKT_SUCC - FRM_SUCC << ", ";
      cout << "SLOTS: " << (PKT_SUCC - FRM_SUCC) / static_cast<double>(enc->slot_time()) << ", ";
      cout << "DATA-ACK: " << (x3 / static_cast<double>(n3)) << endl;

   } catch(const exception& x) {
      cerr << x.what() << endl;
      exit(EXIT_FAILURE);
   } catch(...) {
      cerr << "unhandled exception!" << endl;
      exit(EXIT_FAILURE);
   }
   return EXIT_SUCCESS;
}

