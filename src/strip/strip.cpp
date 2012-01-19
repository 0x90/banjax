#include <iomanip>
#include <iostream>
#include <pcap.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

int
main(int ac, char **av)
{
   while(++av, --ac) {

      string path(*av);
      string backup(path + ".orig");

      char err[PCAP_ERRBUF_SIZE];
      pcap_t *in = pcap_open_offline(path.c_str(), err);
      if(!in) {
         cerr << err << endl;
         exit(EXIT_FAILURE);
      }
      link(path.c_str(), backup.c_str());
      unlink(path.c_str());

      const size_t MAX_BUF_SZ = 8192;
      pcap_t *dead = pcap_open_dead(pcap_datalink(in), MAX_BUF_SZ);
      if(!dead) {
         cerr << "error: pcap_open_dead()" << endl;
         exit(EXIT_FAILURE);
      }

      pcap_dumper_t *out = pcap_dump_open(dead, path.c_str());
      if(!out) {
         cerr << "error: failed to open " << path << " for writing" << endl;
         exit(EXIT_FAILURE);
      }

      bool writing = false;
      const uint8_t *octets;
      struct pcap_pkthdr hdr;
      while(octets = pcap_next(in, &hdr)) {
         if((110 == hdr.caplen || 112 == hdr.caplen) && 0xff == octets[58] && 0xff == octets[59]) {
            writing = !writing;
         } else if(writing) {
            pcap_dump(reinterpret_cast<u_char*>(out), &hdr, octets);
         }
      }
      pcap_dump_close(out);
      pcap_close(dead);
      pcap_close(in);

   }
   exit(EXIT_SUCCESS);
}
