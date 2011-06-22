#define _POSIX_C_SOURCE 199309

#include <arpa/inet.h>
#include <boost/program_options.hpp>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
// #include <sys/time.h>
#include <sys/types.h>
#include <time.h>

using namespace boost::program_options;
using namespace std;


/**
 * Report a fatal error and exit.
 *
 * \param msg A pointer to a string giving an error message.
 */
void
fatal_error(const char *msg)
{
   cerr << msg << endl;
   exit(EXIT_FAILURE);
}


/**
 * Report a fatal syscall error and exit.
 *
 * \param msg A pointer to a string giving an error message.
 */
void
fatal_syscall_error(const char *msg)
{
   cerr << msg << ": " << strerror(errno) << endl;
   exit(EXIT_FAILURE);
}


/**
 * Send ETX probes. This function sends packet_sz packets at intervals
 * of delay_ms. If a specific interface address is given in bind_str
 * then we bind to that and restrict probes to just that interface;
 * otherwise probes are sent on all available interfaces.
 *
 * \param bind_str The address of the interface to bind to.
 * \param port_no The port number to bind.
 * \param packet_sz The size of the packet to send.
 * \param delay_ms The delay between subsequent frames.
 */
void
send_probes(const string& bind_str, uint16_t port_no, uint16_t packet_sz, uint16_t delay_ms)
{
   int s = socket(AF_INET, SOCK_DGRAM, 0);
   if(-1 == s)
      fatal_syscall_error("socket(AF_INET, SOCK_DGRAM, 0)");

   const int ON = 1;
   if(-1 == setsockopt(s, SOL_SOCKET, SO_BROADCAST, &ON, sizeof(ON))) {
      fatal_syscall_error("setsockopt(s, SOL_SOCKET, SO_BROADCAST, NULL, 0)");
   }

   int options;
   struct sockaddr_in src;
   memset(&src, sizeof(src), 0);
   src.sin_family = AF_INET;
   src.sin_port   = htons(port_no);
   if("" == bind_str) {
      src.sin_addr.s_addr = INADDR_ANY;
      options = 0;
   } else {
      if(! inet_aton(bind_str.c_str(), &src.sin_addr))
         fatal_error("inet_aton(bind_str.c_str(), &src.sin_addr)");
      options = MSG_DONTROUTE;
   }

   if(-1 == bind(s, reinterpret_cast<const sockaddr*>(&src), sizeof(src)))
      fatal_syscall_error("bind(s, &dst, sizeof(dst))");

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

   timespec delay, junk;
   delay.tv_sec = delay_ms / 1000;
   delay.tv_nsec = (delay_ms % 1000) * 1000000;

   while(true) {

      // ToDo: decide if we want to apply jitter to sleep
      // ToDo: decide if we need to account for clock creep?

      // timespec tp;
      // if(-1 == clock_gettime(CLOCK_REALTIME_COARSE, &tp))
      //    fatal_error("clock_gettime(CLOCK_REALTIME_COARSE, &tp)");

      if(-1 == sendto(s, buf, buf_sz, options, reinterpret_cast<const sockaddr*>(&dst), sizeof(dst)))
         fatal_syscall_error("sendto(s, buf, buf_sz, MSG_DONTROUTE, &dst, sizeof(dst))");

      if(0 != nanosleep(&delay, &junk))
         fatal_syscall_error("nanosleep(&delay, &junk)");
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
