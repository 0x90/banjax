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

      string enc_str, ta_str, what;
      options_description options("program options");
      options.add_options()
         ("help,?", "produce this help message")
         ("encoding,e", value<string>(&enc_str)->default_value("OFDM"), "channel encoding")
         ("input,i", value<string>(&what)->default_value("mon0"), "input file/device name")
         ("ta,a", value<string>(&ta_str)->default_value("00:0b:6b:0a:82:34"), "transmitter address")
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
      encoding_sptr enc(encoding::get(enc_str));

      eui_48 ta(ta_str.c_str());
      uint16_t txc = 0, seq_no = 0;
      uint_least32_t n_cw = 0, t_cw = 0;
      buffer_sptr b(w->read()), p, sp, null;
      enum { SKIPPING, CONTENDING, TRANSMITTING, ACKNOWLEDGING } state = SKIPPING;
      for(uint32_t n = 1; b; ++n){
         frame f(b);
         frame_control fc(f.fc());
         switch(state) {

         default:
         case SKIPPING:
            switch(fc.subtype()) {
            case CTRL_ACK:
               state = CONTENDING;
               p = b;
               break;
            default:
               state = SKIPPING;
               p = null;
               break;
            }
            break;

         case CONTENDING:
            switch(fc.subtype()) {
            case DATA:
            case DATA_QOS:
               if(f.address2() == ta && !fc.retry()) {
                  state = TRANSMITTING;
                  txc = 0;
                  seq_no = f.sc().sequence_no();
                  int32_t ifs = b->info()->timestamp1() - p->info()->timestamp2();
                  cout << n << " " << b->info()->timestamp1() << " " << ifs << " " << txc << endl;
                  ++n_cw;
                  t_cw += ifs;
                  p = b;
               } else {
                  state = SKIPPING;
                  p = null;
               }
               break;
            case CTRL_ACK:
               state = CONTENDING;
               p = b;
               break;
            default:
               state = SKIPPING;
               p = null;
               break;
            }
            break;

         case TRANSMITTING:
            switch(fc.subtype()) {
            case DATA:
            case DATA_QOS:
               if(f.address2() == ta && fc.retry() && f.sc().sequence_no() == seq_no) {
                  state = TRANSMITTING;
                  ++txc;
                  int32_t ifs = b->info()->timestamp1() - p->info()->timestamp2();
                  cout << n << " " << b->info()->timestamp1() << " " << ifs << " " << txc << endl;
                  ++n_cw;
                  t_cw += ifs;
                  p = b;
               } else {
                  state = SKIPPING;
                  p = null;
               }
               break;
            case CTRL_ACK:
               state = ACKNOWLEDGING;
               sp = p;
               p = b;
               break;
            default:
               state = SKIPPING;
               p = null;
               break;
            }
            break;

         case ACKNOWLEDGING:
            switch(fc.subtype()) {
            case DATA:
            case DATA_QOS:
               if(f.address2() == ta && fc.retry() && f.sc().sequence_no() == seq_no) {
                  state = TRANSMITTING;
                  ++txc;
                  int32_t ifs = b->info()->timestamp1() - p->info()->timestamp2();
                  // NB: we assume sender detects RF energy and waits
                  cout << n << " " << b->info()->timestamp1() << " " << ifs << " " << txc <<  " *" << endl;
                  ++n_cw;
                  t_cw += ifs;
                  p = b;
                  sp = null;
               } else if(f.address2() == ta && !fc.retry()) {
                  state = TRANSMITTING;
                  txc = 0;
                  seq_no = f.sc().sequence_no();
                  int32_t ifs = b->info()->timestamp1() - p->info()->timestamp2();
                  cout << n << " " << b->info()->timestamp1() << " " << ifs << " " << txc << endl;
                  ++n_cw;
                  t_cw += ifs;
                  p = b;
                  sp = null;
               } else {
                  state = SKIPPING;
                  p = null;
                  sp = null;
               }
               break;
            case CTRL_ACK:
               state = CONTENDING;
               p = b;
               sp = null;
               break;
            default:
               state = SKIPPING;
               p = null;
               sp = null;
               break;
            }

         }
         b = w->read();
      }
      cerr << "AVG CONTENTION TIME = " << (t_cw / static_cast<double>(n_cw)) - enc->DIFS() << endl;

   } catch(const error& x) {
      cerr << x.what() << endl;
   } catch(const std::exception& x) {
      cerr << x.what() << endl;
   } catch(...) {
      cerr << "unhandled exception!" << endl;
   }
   exit(EXIT_FAILURE);
}
