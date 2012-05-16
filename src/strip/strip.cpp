#include <iomanip>
#include <iostream>
#include <pcap.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

struct ieee80211_radiotap_header {
   u_int8_t it_version;         /* set to 0 */
   u_int8_t it_pad;
   u_int16_t it_len;            /* entire length */
   u_int32_t it_present;        /* fields present */
} __attribute__((__packed__));

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
      const uint16_t ANNOUNCE_SZ = 82;
      while(octets = pcap_next(in, &hdr)) {
         struct ieee80211_radiotap_header *radiotap = (struct ieee80211_radiotap_header*) octets;
         uint16_t frame_sz = hdr.len - radiotap->it_len;
         if(0 != radiotap->it_version) {
            fputs("error: can't find radiotap header\n", stderr);
            break;
         }
         if((ANNOUNCE_SZ == frame_sz || ANNOUNCE_SZ == frame_sz - 4) && 0xff == octets[58] && 0xff == octets[59]) {
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
