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

#define SERVER "127.0.0.1"
#define BUFLEN 2  //Max length of buffer
#define PORT2 1258 //BandSwitch py script in


#define InUDPPort 5565  //RC Joystick -> USB -> Android phone -> WiFi -> RPi ground hotspot -> 5565 udp in port
#define PORT 5566 // Encrypted RC out via SVPCom
#define PORT2 1258 //BandSwitch py script in
#define PORT3 1259 //IP or USB camera switch py script in

#define SWITCH_COUNT 16

char messageRCEncrypt[40];   //Encrypted RC Message

int NICCount=0;
int Channel = 0;
int ChannelIPCamera = 0;
int IsEncrypt = 0;
int IsIPCameraSwitcherEnabled = 0;
int IsBandSwitcherEnabled = 0;

static uint16_t *rcData = NULL;

uint16_t *rc_channels_memory_open(void) {

	int fd = shm_open("/wifibroadcast_rc_channels", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

	if (fd < 0) {
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


static SDL_Joystick *js;
char *ifname = NULL;
int flagHelp = 0;
int sock = 0;
int socks[5];
int type[5];

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

    unsigned int switches : SWITCH_COUNT; // 16 bits for rc channels 9 - 24  as switches

} __attribute__ ((__packed__));

struct framedata_s framedatas;

struct framedata_n {
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
	uint8_t rt13;

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
    unsigned int switches : SWITCH_COUNT; // 16 bits for rc channels 9 - 24 as switches
} __attribute__ ((__packed__));

struct framedata_n framedatan;


void usage(void)
{
    printf(
        "Usage: rctxUDP_IN  $ChannelBandSwitcher $ChannelIPCamera $IsBandSwitcherEnabled $IsIPCameraSwitcherEnabled $IsEncrypt $NICS wlan0\n"
        "\n"
        "Example:\n"
        "  rctxUDP_IN 6 7 0 1 0 wlan0\n"
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


void sendRC(telemetry_data_t *td)
{
    uint8_t i;
    uint8_t z;
  
    int best_adapter = 0;
        if(td->rx_status != NULL)
    {
        int j = 0;
        int ac = td->rx_status->wifi_adapter_cnt;
        int best_dbm = -1000;
 
    // find out which card has best signal and ignore ralink (type=1) ones
        for(j=0; j<ac; ++j)
        {
            if ((best_dbm < td->rx_status->adapter[j].current_signal_dbm)&&(type[j] != 0))
            {
                best_dbm = td->rx_status->adapter[j].current_signal_dbm;
                best_adapter = j;
            //printf ("best_adapter: :%d\n",best_adapter);
            }
        }
//  printf ("bestadapter: %d (%d dbm)\n",best_adapter, best_dbm);
        if(NICCount == 1)
        {
             if (type[best_adapter] == 2) {
//	printf ("bestadapter: %d (%d dbm)\n",best_adapter, best_dbm);
	            if (write(socks[0], &framedatan, sizeof(framedatan)) < 0 ) fprintf(stderr, "!");	/// framedata_s = 28 or 29 bytes
	        } else {
	            if (write(socks[0], &framedatas, sizeof(framedatas)) < 0 ) fprintf(stderr, "!");	/// framedata_s = 28 or 29 bytes
	        }
        }
        else
        {
            if (type[best_adapter] == 2) {
//	printf ("bestadapter: %d (%d dbm)\n",best_adapter, best_dbm);
	            if (write(socks[best_adapter], &framedatan, sizeof(framedatan)) < 0 ) fprintf(stderr, "!");	/// framedata_s = 28 or 29 bytes
	        } else {
	            if (write(socks[best_adapter], &framedatas, sizeof(framedatas)) < 0 ) fprintf(stderr, "!");	/// framedata_s = 28 or 29 bytes
	        }
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
	char buf[21];
	int tmp = 0;

	struct sockaddr_in si_me, si_other;
	int s, i, slen = sizeof(si_other) , recv_len;
	
	char line[100], path[100];
    FILE* procfile;

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
	x += 5;
	
    Channel = atoi(argv[1]);
	ChannelIPCamera = atoi(argv[2]);

	IsBandSwitcherEnabled = atoi(argv[3]);
	IsIPCameraSwitcherEnabled = atoi(argv[4]);
	IsEncrypt =  atoi(argv[5]);

	if (IsEncrypt == 0)
	{
    while(x < argc && num_interfaces < 8) {
    	snprintf(path, 45, "/sys/class/net/%s/device/uevent", argv[x]);
        procfile = fopen(path, "r");
        if(!procfile) {fprintf(stderr,"ERROR: opening %s failed!\n", path); return 0;}
        fgets(line, 100, procfile); // read the first line
        fgets(line, 100, procfile); // read the 2nd line
	if (strncmp(line, "DRIVER=ath9k_htc", 16) == 0 || 
        (
         strncmp(line, "DRIVER=8812au",    13) == 0 || 
         strncmp(line, "DRIVER=8814au",    13) == 0 || 
         strncmp(line, "DRIVER=rtl8812au", 16) == 0 || 
         strncmp(line, "DRIVER=rtl8814au", 16) == 0 || 
         strncmp(line, "DRIVER=rtl88xxau", 16) == 0
        )) {   
		if (strncmp(line, "DRIVER=ath9k_htc", 16) == 0) {
			  fprintf(stderr, "rctx: Atheros card detected\n");
	          type[num_interfaces] = 1;
		} else {
			  fprintf(stderr, "rctx: Realtek card detected\n");
			  type[num_interfaces] = 2;
		}
    } else { // ralink or mediatek
              fprintf(stderr, "rctx: Ralink card detected\n");
	          type[num_interfaces] = 0;
    }
	socks[num_interfaces] = open_sock(argv[x]);
        ++num_interfaces;
	++x;
        fclose(procfile);
    usleep(20000); // wait a bit between configuring interfaces to reduce Atheros and Pi USB flakiness
    }
	}

	framedatas.rt1 = 0; // <-- radiotap version
	framedatas.rt2 = 0; // <-- radiotap version

	framedatas.rt3 = 12; // <- radiotap header length
	framedatas.rt4 = 0; // <- radiotap header length

	framedatas.rt5 = 4; // <-- radiotap present flags
	framedatas.rt6 = 128; // <-- radiotap present flags
	framedatas.rt7 = 0; // <-- radiotap present flags
	framedatas.rt8 = 0; // <-- radiotap present flags

	framedatas.rt9 = 24; // <-- radiotap rate
	framedatas.rt10 = 0; // <-- radiotap stuff
	framedatas.rt11 = 0; // <-- radiotap stuff
	framedatas.rt12 = 0; // <-- radiotap stuff

	framedatas.fc1 = 180; // <-- frame control field (0xb4)
	framedatas.fc2 = 191; // <-- frame control field (0xbf)
	framedatas.dur1 = 0; // <-- duration
	framedatas.dur2 = 0; // <-- duration

	framedatan.rt1 = 0; // <-- radiotap version      (0x00)
	framedatan.rt2 = 0; // <-- radiotap version      (0x00)

	framedatan.rt3 = 13; // <- radiotap header length(0x0d)
	framedatan.rt4 = 0; // <- radiotap header length (0x00)

	framedatan.rt5 = 0; // <-- radiotap present flags(0x00)
	framedatan.rt6 = 128; // <-- RADIOTAP_TX_FLAGS + (0x80)
	framedatan.rt7 = 8; // <--  RADIOTAP_MCS         (0x08)
	framedatan.rt8 = 0; //                           (0x00)

	framedatan.rt9 = 8; // <-- RADIOTAP_F_TX_NOACK   (0x08)
	framedatan.rt10 = 0; //                          (0x00)
	framedatan.rt11 = 55; // <-- bitmap              (0x37)
	framedatan.rt12 = 48; // <-- flags               (0x30)
	framedatan.rt13 = 0; // <-- mcs_index            (0x00)

	framedatan.fc1 = 180; // <-- frame control field (0xb4)
	framedatan.fc2 = 191; // <-- frame control field (0xbf)
	framedatan.dur1 = 0; // <-- duration
	framedatan.dur2 = 0; // <-- duration


	// we need to prefill channels since we have no values for them as
	// long as the corresponding axis has not been moved yet
	rcData = rc_channels_memory_open();
	rcData[8] = 0;		/// switches


	
	 //Incoming UDP packets from joystick via android HID app 
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        return 1;
    }
     
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(InUDPPort);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
		fprintf(stderr, "Bind error. Exit");
        return 1;
    }
	//udp init1 end
	//________________________________
	//UDP RC Encrypted out init
        struct sockaddr_in si_otherRCEncrypt;
	int sRCEncrypt, iRCEncrypt, slenRCEncrypt = sizeof(si_otherRCEncrypt);
        

	if ((sRCEncrypt = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		perror("sRCEncrypt");
		exit(1);
	}

	memset((char *) &si_otherRCEncrypt, 0, sizeof(si_otherRCEncrypt));
	si_otherRCEncrypt.sin_family = AF_INET;
	si_otherRCEncrypt.sin_port = htons(PORT);

	if (inet_aton(SERVER, &si_otherRCEncrypt.sin_addr) == 0)
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	//udp init end
    //________________________________
	//Outgoing BandSwitch socket
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
	//________________________________
	//udp init IP or USB camera sender
    struct sockaddr_in si_other3;
	int s3, slen3 = sizeof(si_other3);
	char message3[BUFLEN];

	if ((s3 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		exit(1);
	}

	memset((char *) &si_other3, 0, sizeof(si_other3));
	si_other3.sin_family = AF_INET;
	si_other3.sin_port = htons(PORT3);

	if (inet_aton(SERVER, &si_other3.sin_addr) == 0)
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	//udp init end
	//________________________________
	


	
	// init RSSI shared memory
	telemetry_data_t td;
	telemetry_init(&td);
	int k = 0;

	while(1)
	{

        if ((recv_len = recvfrom(s, buf, 21, 0, (struct sockaddr *) &si_other, &slen)) == -1)
        {
			fprintf(stderr, "Recvfrom error. Exit");
            return 1;
		}

		framedatas.seqnumber = buf[16];
		framedatas.chan1 = (buf[1] << 8) | buf[0];
        framedatas.chan2 = (buf[3] << 8) | buf[2];
        framedatas.chan3 = (buf[5] << 8) | buf[4];
        framedatas.chan4 = (buf[7] << 8) | buf[6];
        framedatas.chan5 = (buf[9] << 8) | buf[8];
        framedatas.chan6 = (buf[11] << 8) | buf[10];
        framedatas.chan7 = (buf[13] << 8) | buf[12];
        framedatas.chan8 = (buf[15] << 8) | buf[14];

		framedatas.switches = (buf[20] << 8) | buf[19];
		
		framedatan.seqnumber = buf[16];
		framedatan.chan1 = (buf[1] << 8) | buf[0];
        framedatan.chan2 = (buf[3] << 8) | buf[2];
        framedatan.chan3 = (buf[5] << 8) | buf[4];
        framedatan.chan4 = (buf[7] << 8) | buf[6];
        framedatan.chan5 = (buf[9] << 8) | buf[8];
        framedatan.chan6 = (buf[11] << 8) | buf[10];
        framedatan.chan7 = (buf[13] << 8) | buf[12];
        framedatan.chan8 = (buf[15] << 8) | buf[14];

		framedatan.switches = (buf[20] << 8) | buf[19];
		
		rcData[0] = (buf[1] << 8) | buf[0];
        rcData[1] = (buf[3] << 8) | buf[2];
        rcData[2] = (buf[5] << 8) | buf[4];
        rcData[3] = (buf[7] << 8) | buf[6];
        rcData[4] = (buf[9] << 8) | buf[8];
        rcData[5] = (buf[11] << 8) | buf[10];
        rcData[6] = (buf[13] << 8) | buf[12];
        rcData[7] = (buf[15] << 8) | buf[14];

		if( Channel >= 1 && Channel <= 8 && IsBandSwitcherEnabled == 1)
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
		if (ChannelIPCamera  >= 1 && ChannelIPCamera  <= 8 && IsIPCameraSwitcherEnabled == 1)
		{
			message3[0] = 0;
			message3[1] = 0;
			tmp = ChannelIPCamera;
			tmp--;
			message3[0] = rcData[tmp] & 0xFF;
			message3[1] = rcData[tmp] >> 8;

			if (sendto(s3, message3, 2, 0, (struct sockaddr *) &si_other3, slen3) == -1)
			{
				//printf("sendto() error");
			}
		}
		
		if (IsEncrypt == 1)
		{
			if (sendto(sRCEncrypt, buf, 21, 0, (struct sockaddr *) &si_otherRCEncrypt, slenRCEncrypt) == -1)
			{
				fprintf(stderr, "sendto() error");
				exit(1);
			}

		}
		else
		{
			sendRC(&td);
		}
	
	//	fprintf(stderr,"sending... seq: %d \n", framedatas.seqnumber);
	//	fprintf(stderr,"sending... seq: %d \n", framedatan.seqnumber);
		//sendRC(&td);

	}

	return 0;
}
