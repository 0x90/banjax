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
		{"begin",	no_argument,			0, 'b' },
		{"end",		no_argument,			0, 'e' },
		{"change",	required_argument,	0, 'c' },
		{"verbose", no_argument,			0, 'v' },
		{0,			0,							0, 0 }
	};

	uint8_t val;
	int c, opt_index, verbose = 0;
	uint16_t proto = ETH_P_ALL;
	while((c = getopt_long(ac, av, "bc:ev", opts, &opt_index)) != -1) {
		switch(c) {
		case 'b':
		case 'e':
			proto = ETH_P_ALL;
			break;
		case 'c':
			proto = 0xfffe;
			val = strtoul(optarg, NULL, 10);
			break;
		case 'v':
			verbose = 1;
			break;
		}
	}
	if(optind != ac - 1) {
		fprintf(stderr, "usage: announce [opts] <dev>\n");
		exit(EXIT_FAILURE);
	}

	if(verbose)
		printf("opening %s...\n", av[optind]);

	int s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(-1 == s) {
		perror("socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))");
		exit(EXIT_FAILURE);
	}

	struct ifreq ifr;
	strncpy(ifr.ifr_name, av[optind], IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	ioctl(s, SIOCGIFINDEX, &ifr);

	struct sockaddr_ll addr;
	bzero(&addr, sizeof(addr));
	addr.sll_family	= AF_PACKET;
	addr.sll_protocol = htons(proto);
	addr.sll_ifindex	= ifr.ifr_ifindex;
	if(-1 == bind(s, (struct sockaddr*) &addr, sizeof(addr))) {
		perror("bind(s, (struct sockaddr*) &addr, sizeof(addr))");
		exit(EXIT_FAILURE);
	}

	static uint8_t pkt[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
		0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
		0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
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
