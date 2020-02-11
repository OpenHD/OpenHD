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
#include "openhdlib.h"

#include "/tmp/rctx.h"


wifibroadcast_rx_status_t *telemetry_wbc_status_memory_open(void);


#define UPDATE_INTERVAL 2000 // read Joystick every 2 ms or 500x per second
#define JOY_CHECK_NTH_TIME 400 // check if joystick disconnected every 400th time or 200ms or 5x per second
#define JOYSTICK_N 0
#define JOY_DEV "/sys/class/input/js0"

int NICCount=0;

#ifdef JSSWITCHES  // 1 or 2 byte more for channels 9 - 16/24 as switches

	static uint16_t *rcData = NULL;

	uint16_t *rc_channels_memory_open(void) {

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
#ifdef JSSWITCHES
    unsigned int switches : JSSWITCHES; // 8 or 16 bits for rc channels 9 - 16/24  as switches
#endif
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

    NICCount++;

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


int16_t parsetoMultiWii(Sint16 value) {
	return (int16_t)(((((double)value)+32768.0)/65.536)+1000);
}


void readAxis(SDL_Event *event) {
	SDL_Event myevent = (SDL_Event)*event;
	switch(myevent.jaxis.axis) {
		case ROLL_AXIS:
				rcData[0]=parsetoMultiWii(myevent.jaxis.value);
			break;
		case PITCH_AXIS:
				rcData[1]=parsetoMultiWii(myevent.jaxis.value);
			break;
		case THROTTLE_AXIS:
				rcData[2]=parsetoMultiWii(myevent.jaxis.value);
			break;
		case YAW_AXIS:
				rcData[3]=parsetoMultiWii(myevent.jaxis.value);
			break;
		case AUX1_AXIS:
				rcData[4]=parsetoMultiWii(myevent.jaxis.value);
			break;
		case AUX2_AXIS:
				rcData[5]=parsetoMultiWii(myevent.jaxis.value);
			break;
		case AUX3_AXIS:
				rcData[6]=parsetoMultiWii(myevent.jaxis.value);
			break;
		case AUX4_AXIS:
				rcData[7]=parsetoMultiWii(myevent.jaxis.value);
			break;
		default:
			break; //do nothing
	}
}


static int eventloop_joystick (void) {
  SDL_Event event;
  while (SDL_PollEvent (&event)) {
    switch (event.type) {
		case SDL_JOYAXISMOTION:
			//printf ("Joystick %d, Axis %d moved to %d\n", event.jaxis.which, event.jaxis.axis, event.jaxis.value);
			readAxis(&event);
			return 2;
			break;
#ifdef	JSSWITCHES  // channels 9 - 16 as switches
		case SDL_JOYBUTTONDOWN:
			if (event.jbutton.button < JSSWITCHES) { // newer Taranis software can send 24 buttons - we use 16
				rcData[8] |= 1 << event.jbutton.button;
			}
			return 5;
			break;
		case SDL_JOYBUTTONUP:
			if (event.jbutton.button < JSSWITCHES) {
				rcData[8] &= ~(1 << event.jbutton.button);
			}
			return 4;
			break;
#endif
			case SDL_QUIT:
			return 0;
    }
    usleep(100);
  }
  return 1;
}

void sendRC(unsigned char seqno, telemetry_data_t *td) {
    uint8_t i;
    uint8_t z;

    framedata.seqnumber = seqno;
    framedata.chan1 = rcData[0];
    framedata.chan2 = rcData[1];
    framedata.chan3 = rcData[2];
    framedata.chan4 = rcData[3];
    framedata.chan5 = rcData[4];
    framedata.chan6 = rcData[5];
    framedata.chan7 = rcData[6];
    framedata.chan8 = rcData[7];
#ifdef JSSWITCHES
	framedata.switches = rcData[8];	/// channels 9 - 24 as switches
//	printf ("rcdata0:%x\t",rcData[8]);
#endif
//  printf ("rcdata0:%d\n",rcData[0]);

    int best_adapter = 0;
    	if(td->rx_status != NULL)
	{
		int j = 0;
		int ac = td->rx_status->wifi_adapter_cnt;
		int best_dbm = -1000;

	// find out which card has best signal and ignore ralink (type=1) ones
		for(j=0; j<ac; ++j)
		{
			if ((best_dbm < td->rx_status->adapter[j].current_signal_dbm)&&(td->rx_status->adapter[j].type == 0))
			{
				best_dbm = td->rx_status->adapter[j].current_signal_dbm;
				best_adapter = j;
			//printf ("best_adapter: :%d\n",best_adapter);
		   	}
		}
//	printf ("bestadapter: %d (%d dbm)\n",best_adapter, best_dbm);
		if(NICCount == 1)
		{
			 if (write(socks[0], &framedata, sizeof(framedata)) < 0 ) { fprintf(stderr, "!"); exit(1); }
		}
		else
		{
			if (write(socks[best_adapter], &framedata, sizeof(framedata)) < 0 ) { fprintf(stderr, "!"); exit(1); }	/// framedata_s = 28 or 29 bytes
		}
	}
	else
	{
		printf ("ERROR: Could not open rx status memory!");
    	}
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
    int done = 1;
    int joy_connected = 0;
    int joy = 1;
    int update_nth_time = 0;

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

	fprintf(stderr, "Waiting for joystick ...");
	while (joy) {
	    joy_connected=access(JOY_DEV, F_OK);
	    fprintf(stderr, ".");
	    if (joy_connected == 0) {
		fprintf(stderr, "connected!\n");
		joy=0;
	    }
	    usleep(100000);
	}

	// we need to prefill channels since we have no values for them as
	// long as the corresponding axis has not been moved yet
#ifdef	JSSWITCHES
	rcData = rc_channels_memory_open();
	rcData[8]=0;		/// switches
#endif
	rcData[0]=AXIS0_INITIAL;
	rcData[1]=AXIS1_INITIAL;
	rcData[2]=AXIS2_INITIAL;
	rcData[3]=AXIS3_INITIAL;
	rcData[4]=AXIS4_INITIAL;
	rcData[5]=AXIS5_INITIAL;
	rcData[6]=AXIS6_INITIAL;
	rcData[7]=AXIS7_INITIAL;

	if (SDL_Init (SDL_INIT_JOYSTICK | SDL_INIT_VIDEO) != 0)
	{
		printf ("ERROR: %s\n", SDL_GetError ());
		return EXIT_FAILURE;
	}
	atexit (SDL_Quit);
	js = SDL_JoystickOpen (JOYSTICK_N);
	if (js == NULL)
	{
		printf("Couldn't open desired Joystick: %s\n",SDL_GetError());
		done=0;
	} else {
		printf ("\tName:       %s\n", SDL_JoystickName(JOYSTICK_N));
		printf ("\tAxis:       %i\n", SDL_JoystickNumAxes(js));
		printf ("\tTrackballs: %i\n", SDL_JoystickNumBalls(js));
		printf ("\tButtons:   %i\n",SDL_JoystickNumButtons(js));
		printf ("\tHats: %i\n",SDL_JoystickNumHats(js)); 
	}

	// init RSSI shared memory
	telemetry_data_t td;
	telemetry_init(&td);

	int counter = 0;
	int seqno = 0;
	int k = 0;
	while (done) {
		done = eventloop_joystick();
//		fprintf(stderr, "eventloop_joystick\n");
		if (counter % UPDATE_NTH_TIME == 0) {
//		    fprintf(stderr, "SendRC\n");
		    for(k=0; k < TRANSMISSIONS; ++k) {
			sendRC(seqno,&td);
			usleep(2000); // wait 2ms between sending multiple frames to lower collision probability
		    }
		    seqno++;
		}
		if (counter % JOY_CHECK_NTH_TIME == 0) {
		    joy_connected=access(JOY_DEV, F_OK);
		    if (joy_connected != 0) {
			fprintf(stderr, "joystick disconnected, exiting\n");
			done=0;
		    }
		}
		usleep(UPDATE_INTERVAL);
		counter++;
	}
	SDL_JoystickClose (js);
	return EXIT_SUCCESS;
}
