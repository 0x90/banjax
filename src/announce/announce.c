#include <arpa/inet.h>
#include <linux/if.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int
main(int ac, char **av)
{
   if(ac != 2) {
      fprintf(stderr, "usage: announce <dev>\n");
      exit(EXIT_FAILURE);
   }

   int s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
   if(-1 == s) {
      perror("socket(PF_PACKET, SOCK_RAW, ETH_P_ALL)");
      exit(EXIT_FAILURE);
   }

   struct ifreq ifr;
   strncpy(ifr.ifr_name, av[1], IFNAMSIZ);
   ifr.ifr_name[IFNAMSIZ - 1] = '\0';
   ioctl(s, SIOCGIFINDEX, &ifr);

   struct sockaddr_ll addr;
   bzero(&addr, sizeof(addr));
   addr.sll_family   = AF_PACKET;
   addr.sll_protocol = htons(ETH_P_ALL);
   addr.sll_ifindex  = ifr.ifr_ifindex;
   if(-1 == bind(s, (struct sockaddr*) &addr, sizeof(addr))) {
      perror("bind(s, (struct sockaddr*) &addr, sizeof(addr))");
      exit(EXIT_FAILURE);
   }

   static const uint8_t PKT[] = {
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   };
   static const size_t PKT_SZ = sizeof(PKT);

   ssize_t sent = send(s, PKT, PKT_SZ, 0);
   if(PKT_SZ != sent) {
      perror("send(s, PKT, PKT_SZ, 0)");
   }

   close(s);

   exit(EXIT_SUCCESS);
}
