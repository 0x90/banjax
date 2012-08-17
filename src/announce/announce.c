#include <arpa/inet.h>
#include <getopt.h>
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
	static struct option opts[] = {
		{"begin",  no_argument,       0, 'b' },
		{"end",    no_argument,       0, 'e' },
		{"change", required_argument, 0, 'c' },
		{0,        0,                 0, 0 }
	};

	uint8_t val;
	int c, opt_index;
	uint16_t proto = ETH_P_ALL;
	while(c = getopt_long(ac, av, "bc:e", opts, &opt_index)) {
		switch(c) {
		case 'b':
		case 'e':
			proto = ETH_P_ALL;
			break;
		case 'c':
			proto = 0xfffe;
			val = strtoul(optarg, NULL, 10);
			break;
		default:
			exit(EXIT_FAILURE);
		}
	}
   if(ac != 2) {
      fprintf(stderr, "usage: announce [opts] <dev>\n");
      exit(EXIT_FAILURE);
   }
	exit(EXIT_SUCCESS);

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
   addr.sll_protocol = htons(proto);
   addr.sll_ifindex  = ifr.ifr_ifindex;
   if(-1 == bind(s, (struct sockaddr*) &addr, sizeof(addr))) {
      perror("bind(s, (struct sockaddr*) &addr, sizeof(addr))");
      exit(EXIT_FAILURE);
   }

   static uint8_t pkt[] = {
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   };
   static const size_t PKT_SZ = sizeof(pkt);


	if(proto != ETH_P_ALL)
		pkt[1] = val;

   ssize_t sent = send(s, pkt, PKT_SZ, 0);
   if(PKT_SZ != sent) {
      perror("send(s, pkt, PKT_SZ, 0)");
		exit(EXIT_FAILURE);
   }
   close(s);

   exit(EXIT_SUCCESS);
}
