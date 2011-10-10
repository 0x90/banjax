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

      string enc_str, what;
      options_description options("program options");
      options.add_options()
         ("help,?", "produce this help message")
         ("encoding,e", value<string>(&enc_str)->default_value("OFDM"), "channel encoding")
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
      encoding_sptr enc(encoding::get(enc_str));

      uint16_t txc = 0;
      uint_least32_t n_cw = 0, t_cw = 0;
      buffer_sptr b(w->read()), p, pp, null;
      enum { SKIPPING, CONTENDING, TRANSMITTING } state = SKIPPING;
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
               if(!fc.retry()) {
                  state = TRANSMITTING;
                  txc = 0;
                  int32_t ifs = b->info()->timestamp1() - p->info()->timestamp2();
                  cout << n << " " << b->info()->timestamp1() << " " << ifs << " " << txc << endl;
                  ++n_cw;
                  t_cw += ifs;
                  p = b;
               } else {
                  cerr << n << " - possible ACK/RETRANSMIT transition!" << endl;
                  // we assume sender hasn't seen the ACK
                  // WARNING: beware of ACKTimeouts!
                  state = TRANSMITTING;
                  ++txc;
                  int32_t ifs = b->info()->timestamp1() - pp->info()->timestamp2();
                  cout << n << " " << b->info()->timestamp1() << " " << ifs << " " << txc << endl;
                  ++n_cw;
                  t_cw += ifs;
                  p = pp;
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
               if(fc.retry()) {
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
               state = CONTENDING;
               pp = p;
               p = b;
               break;
            default:
               state = SKIPPING;
               p = null;
               break;
            }
            break;

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
