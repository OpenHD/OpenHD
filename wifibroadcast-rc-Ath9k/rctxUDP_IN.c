// rctx by Rodizio
// Based on JS2Serial by Oliver Mueller and wbc all-in-one tx by Anemostec.
// Thanks to dino_de for the Joystick switches and mavlink code
// Licensed under GPL2
#include <stdlib.h>
#include <stdio.h>
#include <sys/resource.h>
#include <SDL/SDL.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <string.h>
#include <getopt.h>
#include "lib.h"

#include "/tmp/rctx.h"

#define SERVER "127.0.0.1"
#define BUFLEN 2  //Max length of buffer
#define PORT2 1258 //BandSwitch py script in


#ifdef JSSWITCHES  // 1 or 2 byte more for channels 9 - 16/24 as switches

	static uint16_t *rcData = NULL;

	uint16_t *rc_channels_memory_open(void)
	{

		int fd = shm_open("/wifibroadcast_rc_channels", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

		if(fd < 0) {
			fprintf(stderr,"rc shm_open\n");
			exit(1);
		}

		if (ftruncate(fd, 9 * sizeof(uint16_t)) == -1) {
			fprintf(stderr,"rc ftruncate\n");
			exit(1);
		}

		void *retval = mmap(NULL, 9 * sizeof(uint16_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (retval == MAP_FAILED) {
			fprintf(stderr,"rc mmap\n");
			exit(1);
		}

	return (uint16_t *)retval;
	}
#else
	static uint16_t rcData[8]; // interval [1000;2000]
#endif

static SDL_Joystick *js;
char *ifname = NULL;
int flagHelp = 0;
int sock = 0;
int socks[5];

struct framedata_s {
    // 88 bits of data (11 bits per channel * 8 channels) = 11 bytes
    uint8_t rt1;
    uint8_t rt2;
    uint8_t rt3;
    uint8_t rt4;
    uint8_t rt5;
    uint8_t rt6;
    uint8_t rt7;
    uint8_t rt8;

    uint8_t rt9;
    uint8_t rt10;
    uint8_t rt11;
    uint8_t rt12;

    uint8_t fc1;
    uint8_t fc2;
    uint8_t dur1;
    uint8_t dur2;

    uint8_t seqnumber;

    unsigned int chan1 : 11;
    unsigned int chan2 : 11;
    unsigned int chan3 : 11;
    unsigned int chan4 : 11;
    unsigned int chan5 : 11;
    unsigned int chan6 : 11;
    unsigned int chan7 : 11;
    unsigned int chan8 : 11;

    unsigned int switches : 16; // 8 or 16 bits for rc channels 9 - 16/24  as switches

} __attribute__ ((__packed__));

struct framedata_s framedata;


void usage(void)
{
    printf(
        "rctx by Rodizio. Based on JS2Serial by Oliver Mueller and wbc all-in-one tx by Anemostec. GPL2\n"
        "\n"
        "Usage: rctx <interfaces>\n"
        "\n"
        "Example:\n"
        "  rctx wlan0\n"
        "\n");
    exit(1);
}

static int open_sock (char *ifname) {
    struct sockaddr_ll ll_addr;
    struct ifreq ifr;


    sock = socket (AF_PACKET, SOCK_RAW, 0);
    if (sock == -1) {
	fprintf(stderr, "Error:\tSocket failed\n");
	exit(1);
    }

    ll_addr.sll_family = AF_PACKET;
    ll_addr.sll_protocol = 0;
    ll_addr.sll_halen = ETH_ALEN;

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
	fprintf(stderr, "Error:\tioctl(SIOCGIFINDEX) failed\n");
	exit(1);
    }

    ll_addr.sll_ifindex = ifr.ifr_ifindex;

    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
	fprintf(stderr, "Error:\tioctl(SIOCGIFHWADDR) failed\n");
	exit(1);
    }

    memcpy(ll_addr.sll_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

    if (bind (sock, (struct sockaddr *)&ll_addr, sizeof(ll_addr)) == -1) {
	fprintf(stderr, "Error:\tbind failed\n");
	close(sock);
	exit(1);
    }

    if (sock == -1 ) {
        fprintf(stderr,
        "Error:\tCannot open socket\n"
        "Info:\tMust be root with an 802.11 card with RFMON enabled\n");
        exit(1);
    }

    return sock;
}


void sendRC(telemetry_data_t *td) {

     if (write(socks[0], &framedata, sizeof(framedata)) < 0 ) { fprintf(stderr, "!"); exit(1); }
}



wifibroadcast_rx_status_t *telemetry_wbc_status_memory_open(void) {
    int fd = 0;
    int sharedmem = 0;

    while(sharedmem == 0) {
        fd = shm_open("/wifibroadcast_rx_status_0", O_RDONLY, S_IRUSR | S_IWUSR);
	    if(fd < 0) {
		fprintf(stderr, "Could not open wifibroadcast rx status - will try again ...\n");
	    } else {
		sharedmem = 1;
	    }
	    usleep(100000);
    }

//        if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t)) == -1) {
//                perror("ftruncate");
//                exit(1);
//        }

        void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t), PROT_READ, MAP_SHARED, fd, 0);
        if (retval == MAP_FAILED) {
                perror("mmap");
                exit(1);
        }

        return (wifibroadcast_rx_status_t*)retval;

return 0;
}


void telemetry_init(telemetry_data_t *td) {
    td->rx_status = telemetry_wbc_status_memory_open();
}


int main (int argc, char *argv[]) {
	char buf[21];
	int Channel = 0;
	int tmp = 0;

	struct sockaddr_in si_me, si_other;
	int s, i, slen = sizeof(si_other) , recv_len;

    while (1) {
	int nOptionIndex;
	static const struct option optiona[] = {
	    { "help", no_argument, &flagHelp, 1 },
	    { 0, 0, 0, 0 }
	};
	int c = getopt_long(argc, argv, "h:",
	    optiona, &nOptionIndex);

	if (c == -1)
	    break;
	switch (c) {
	case 0: // long option
	    break;
	case 'h': // help
	    usage();
	    break;
	default:
	    fprintf(stderr, "unknown switch %c\n", c);
	    usage();
	}
    }

    if (optind >= argc) {
	usage();
    }

    int x = optind;
    int num_interfaces = 0;
     x++;
    Channel = atoi(argv[1]);

    while(x < argc && num_interfaces < 8) {
	socks[num_interfaces] = open_sock(argv[x]);
	++num_interfaces;
	++x;
	usleep(20000); // wait a bit between configuring interfaces to reduce Atheros and Pi USB flakiness
    }

	framedata.rt1 = 0; // <-- radiotap version
	framedata.rt2 = 0; // <-- radiotap version

	framedata.rt3 = 12; // <- radiotap header length
	framedata.rt4 = 0; // <- radiotap header length

	framedata.rt5 = 4; // <-- radiotap present flags
	framedata.rt6 = 128; // <-- radiotap present flags
	framedata.rt7 = 0; // <-- radiotap present flags
	framedata.rt8 = 0; // <-- radiotap present flags

	framedata.rt9 = 24; // <-- radiotap rate
	framedata.rt10 = 0; // <-- radiotap stuff
	framedata.rt11 = 0; // <-- radiotap stuff
	framedata.rt12 = 0; // <-- radiotap stuff

	framedata.fc1 = 180; // <-- frame control field (0xb4)
	framedata.fc2 = 191; // <-- frame control field (0xbf)
	framedata.dur1 = 0; // <-- duration
	framedata.dur2 = 0; // <-- duration



	// we need to prefill channels since we have no values for them as
	// long as the corresponding axis has not been moved yet
#ifdef	JSSWITCHES
	rcData = rc_channels_memory_open();
	rcData[8]=0;		/// switches
#endif



	
	 //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        return 1;
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(5565);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
	fprintf(stderr, "Bind error. Exit");
        return 1;
    }
//udp init1 end

//udp init2
        struct sockaddr_in si_other2;
        int s2, slen2 = sizeof(si_other2);
        char message2[BUFLEN];

        if ((s2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        {
                exit(1);
        }

        memset((char *) &si_other2, 0, sizeof(si_other2));
        si_other2.sin_family = AF_INET;
        si_other2.sin_port = htons(PORT2);

        if (inet_aton(SERVER, &si_other2.sin_addr) == 0)
        {
                fprintf(stderr, "inet_aton() failed\n");
                exit(1);
        }
//udp init2 end


	
	// init RSSI shared memory
	telemetry_data_t td;
	telemetry_init(&td);

	while(1)
	{

        if ((recv_len = recvfrom(s, buf, 21, 0, (struct sockaddr *) &si_other, &slen)) == -1)
        {
		fprintf(stderr, "Recvfrom error. Exit");
            return 1;
	}

	framedata.seqnumber = buf[16];
	framedata.chan1 = (buf[1] << 8) | buf[0];
        framedata.chan2 = (buf[3] << 8) | buf[2];
        framedata.chan3 = (buf[5] << 8) | buf[4];
        framedata.chan4 = (buf[7] << 8) | buf[6];
        framedata.chan5 = (buf[9] << 8) | buf[8];
        framedata.chan6 = (buf[11] << 8) | buf[10];
        framedata.chan7 = (buf[13] << 8) | buf[12];
        framedata.chan8 = (buf[15] << 8) | buf[14];

	rcData[0] = (buf[1] << 8) | buf[0];
        rcData[1] = (buf[3] << 8) | buf[2];
        rcData[2] = (buf[5] << 8) | buf[4];
        rcData[3] = (buf[7] << 8) | buf[6];
        rcData[4] = (buf[9] << 8) | buf[8];
        rcData[5] = (buf[11] << 8) | buf[10];
        rcData[6] = (buf[13] << 8) | buf[12];
        rcData[7] = (buf[15] << 8) | buf[14];

//	framedata.Is16 = buf[18];
	framedata.switches = (buf[20] << 8) | buf[19];

	if( Channel >= 1 && Channel <= 8 )
	{
		message2[0] = 0;
		message2[1] = 0;
		tmp = Channel;
		tmp--;
		//fprintf(stderr, "Channel value: %d \n", rcData[tmp] );
		message2[0] = rcData[tmp] & 0xFF;
		message2[1] = rcData[tmp] >> 8;

                if (sendto(s2, message2, 2, 0, (struct sockaddr *) &si_other2, slen2) == -1)
		{
			//printf("sendto() error");
		}

	}
	
//	fprintf(stderr,"sending... seq: %d \n", framedata.seqnumber);
	sendRC(&td);

	}

	return 0;
}
