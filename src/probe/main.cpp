#define _POSIX_C_SOURCE 199309
#define __STDC_LIMIT_MACROS ENABLED

#include <arpa/inet.h>
#include <boost/program_options.hpp>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <util/exceptions.hpp>
#include <util/syscall_error.hpp>

using namespace boost::program_options;
using namespace std;
using util::raise;
using util::syscall_error;


/**
 * Add two timespecs and return the result.
 *
 * \param lhs A timespec.
 * \param rhs A timespec.
 * \return The result of adding lhs + rhs.
 */
timespec
operator+(const timespec& lhs, const timespec& rhs)
{
   timespec r;
   const int64_t NS_PER_S = INT64_C(1000000000);
   int64_t t = static_cast<int64_t>(lhs.tv_nsec) + static_cast<int64_t>(rhs.tv_nsec); 
   r.tv_sec = lhs.tv_sec + rhs.tv_sec + (t / NS_PER_S);
   r.tv_nsec = t % NS_PER_S;
   return r;
}


/**
 * Multiply a timespec by the rhs value and return the result.
 *
 * \param lhs A timespec.
 * \param rhs A double that specifies the multiplication factor.
 * \return The result of multiplying lhs * rhs.
 */
timespec
operator*(const timespec& lhs, double rhs)
{
   timespec r;
   const int64_t NS_PER_S = INT64_C(1000000000);
   int64_t t = lhs.tv_nsec * rhs;
   r.tv_sec = (lhs.tv_sec * rhs) + (t / NS_PER_S);
   r.tv_nsec = t % NS_PER_S;
   return r;
}


/**
 * Send ETX probes. This function sends packet_sz probe packets at
 * intervals of delay_ms. If a specific interface address is given in
 * bind_str then we bind to that and restrict probes to just that
 * interface; otherwise probes are sent on all available interfaces.
 *
 * \param bind_str The dotted decimal address of the interface to bind to.
 * \param port_no The port number to bind.
 * \param packet_sz The size of the packet to send.
 * \param delay_ms The delay between subsequent frames.
 */
void
send_probes(const string& bind_str, uint16_t port_no, uint16_t packet_sz, uint16_t delay_ms)
{
   int s = socket(AF_INET, SOCK_DGRAM, 0);
   if(-1 == s) {
      ostringstream msg;
      msg << "socket(AF_INET, SOCK_DGRAM, 0): ";
      msg << strerror(errno) << endl;
      raise<syscall_error>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
   }

   const int ON = 1;
   if(-1 == setsockopt(s, SOL_SOCKET, SO_BROADCAST, &ON, sizeof(ON))) {
      ostringstream msg;
      msg << "setsockopt(s, SOL_SOCKET, SO_BROADCAST, NULL, 0): ";
      msg << strerror(errno) << endl;
      raise<syscall_error>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
   }

   int options = 0;
   struct sockaddr_in src;
   memset(&src, sizeof(src), 0);
   src.sin_family = AF_INET;
   src.sin_port   = htons(port_no);
   if("" == bind_str) {
      src.sin_addr.s_addr = INADDR_ANY;
   } else {
      options = MSG_DONTROUTE;
      int err = inet_aton(bind_str.c_str(), &src.sin_addr); 
      if(0 != err) {
         ostringstream msg;
         msg << "inet_aton(bind_str.c_str(), &src.sin_addr): ";
         msg << "failed (err=" << err << ")" << endl;
         raise<syscall_error>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
      }
   }

   if(-1 == bind(s, reinterpret_cast<const sockaddr*>(&src), sizeof(src))) {
      ostringstream msg;
      msg << "bind(s, &dst, sizeof(dst)): ";
      msg << strerror(errno) << endl;
      raise<syscall_error>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
   }

   struct sockaddr_in dst;
   memset(&dst, sizeof(dst), 0);
   dst.sin_family = AF_INET;
   dst.sin_port = htons(port_no);
   dst.sin_addr.s_addr = INADDR_BROADCAST;

   const uint16_t buf_sz = packet_sz;
   char buf[buf_sz];
   for(uint16_t i = 0; i < buf_sz; ++i) {
      buf[i] = 'A' + (i % 26);
   }

   timespec start;
   if(-1 == clock_gettime(CLOCK_REALTIME_COARSE, &start)) {
      ostringstream msg;
      msg << "clock_gettime(CLOCK_REALTIME_COARSE, &start): ";
      msg << strerror(errno) << endl;
      raise<syscall_error>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
   }

   timespec tick, delta, jitter, junk;
   delta.tv_sec = delay_ms / 1000;
   delta.tv_nsec = (delay_ms % 1000) * 1000000;

   for(uint32_t i = 0; i < UINT32_MAX; ++i) {
      if(-1 == sendto(s, buf, buf_sz, options, reinterpret_cast<const sockaddr*>(&dst), sizeof(dst))) {
         ostringstream msg;
         msg << "sendto(s, buf, buf_sz, MSG_DONTROUTE, &dst, sizeof(dst)): ";
         msg << strerror(errno) << endl;
         raise<syscall_error>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
      }
      tick = start + (delta * i) /* + jitter */;
      int err = clock_nanosleep(CLOCK_REALTIME_COARSE, TIMER_ABSTIME, &tick, &junk);
      if(0 != err) {
         ostringstream msg;
         msg << "clock_nanosleep(CLOCK_REALTIME_COARSE, TIMER_ABSTIME, &delay, &junk): ";
         msg << strerror(err) << endl;
         raise<syscall_error>(__PRETTY_FUNCTION__, __FILE__, __LINE__, msg.str());
      }
   }
   close(s);
}


/**
 * ETX probe generator.
 *
 * \param ac The argument count.
 * \param av The argument vector.
 * \return EXIT_FAILURE.
 */
int
main(int ac, char **av)
{
   try {

      uint16_t delay_ms;
      uint16_t port_no;
      uint16_t packet_sz;
      string bind_str;

      options_description options("program options");
      options.add_options()
         ("help,?", "produce this help message")
         ("bind,b",  value<string>(&bind_str), "address of interface to bind to")
         ("delay,d", value<uint16_t>(&delay_ms)->default_value(1000), "delay between probes (in milliseconds)")
         ("port,p",  value<uint16_t>(&port_no)->default_value(5000), "port number")
         ("size,s",  value<uint16_t>(&packet_sz)->default_value(32), "size of probe packets")
         ;
      
      variables_map vars;       
      store(parse_command_line(ac, av, options), vars);
      notify(vars);   

      if(vars.count("help")) {
         cout << options << endl;
         exit(EXIT_SUCCESS);
      }

      send_probes(bind_str, port_no, packet_sz, delay_ms);

   } catch(const error& x) {
      cerr << x.what() << endl;
   } catch(const std::exception& x) {
      cerr << x.what() << endl;
   } catch(...) {
      cerr << "unhandled exception!" << endl;
   }
   return EXIT_FAILURE;
}
