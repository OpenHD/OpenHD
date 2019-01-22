// wifisbackgroundscan by Rodizio. Based on wifibroadcast rx by Befinitiv. Licensed under GPL2
// shows wifi traffic in the background for debugging purposes
/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "lib.h"
#include "wifibroadcast.h"
#include "radiotap.h"
#include <time.h>
#include <sys/resource.h>

// this is where we store a summary of the information from the radiotap header
typedef struct  {
	int m_nChannel;
	int m_nChannelFlags;
	int m_nRate;
	int m_nAntenna;
	int m_nRadiotapFlags;
} __attribute__((packed)) PENUMBRA_RADIOTAP_DATA;


int flagHelp = 0;

wifibroadcast_rx_status_t *rx_status = NULL;

void usage(void) {
	printf(
	    "wifibackgroundscan by Rodizio. Based on wifibroadcast rx by Befinitiv. Licensed under GPL2\n"
	    "\n"
	    "Usage: wifibackgroundscan <interfaces>\n\n"
	    "Example:\n"
	    "  wifibackgroundscan wlan0 (check for wifi traffic on interface wlan0 and print numbers to stdout)\n"
	    "\n");
	exit(1);
}

typedef struct {
	pcap_t *ppcap;
	int selectable_fd;
	int n80211HeaderLength;
} monitor_interface_t;



void open_and_configure_interface(const char *name, monitor_interface_t *interface) {
	struct bpf_program bpfprogram;
	char szProgram[512];
	char szErrbuf[PCAP_ERRBUF_SIZE];

	// open the interface in pcap
	szErrbuf[0] = '\0';

	interface->ppcap = pcap_open_live(name, 44, 0, -1, szErrbuf);
	if (interface->ppcap == NULL) {
		fprintf(stderr, "Unable to open %s: %s\n", name, szErrbuf);
		exit(1);
	}
	
	if(pcap_setnonblock(interface->ppcap, 1, szErrbuf) < 0) {
		fprintf(stderr, "Error setting %s to nonblocking mode: %s\n", name, szErrbuf);
	}

	int nLinkEncap = pcap_datalink(interface->ppcap);

	if (nLinkEncap == DLT_IEEE802_11_RADIO) {
		interface->n80211HeaderLength = 0x18; // 24 bytes
//		sprintf(szProgram, "ether[0x00:1] == 0x08 || ether[0x00:1] == 0xc4 || ether[0x00:1] == 0xb4 || ether[0x00:1] == 0xd4 || ether[0x00:1] == 0x94 || ether[0x00:1] == 0x84"); // match on 1st byte (data, cts, rts, ack, block ack)
		sprintf(szProgram, "ether[0x00:2] == 0x0842 || ether[0x00:2] == 0x0841|| ether[0x00:1] == 0xc4 || ether[0x00:2] == 0xb400 || ether[0x00:1] == 0xd4 || ether[0x00:1] == 0x94 || ether[0x00:1] == 0x84 || ether[0x00:1] == 0x80"); // match on 1st byte (data, cts, rts, ack, block ack, beacon)
	} else {
		fprintf(stderr, "ERROR: unknown encapsulation on %s! check if monitor mode is supported and enabled\n", name);
		exit(1);
	}

	if (pcap_compile(interface->ppcap, &bpfprogram, szProgram, 1, 0) == -1) {
		puts(szProgram);
		puts(pcap_geterr(interface->ppcap));
		fprintf(stderr,"error in pcap_compile");
		exit(1);
	} else {
		if (pcap_setfilter(interface->ppcap, &bpfprogram) == -1) {
			fprintf(stderr, "%s\n", szProgram);
			fprintf(stderr, "%s\n", pcap_geterr(interface->ppcap));
		} else {
		}
		pcap_freecode(&bpfprogram);
	}

	interface->selectable_fd = pcap_get_selectable_fd(interface->ppcap);
}



void process_packet(monitor_interface_t *interface, int adapter_no) {
		struct pcap_pkthdr * ppcapPacketHeader = NULL;
		struct ieee80211_radiotap_iterator rti;
		PENUMBRA_RADIOTAP_DATA prd;
		u8 payloadBuffer[100];
		u8 *pu8Payload = payloadBuffer;
		int bytes;
		int n;
		int retval;
		int u16HeaderLen;

		// receive
		retval = pcap_next_ex(interface->ppcap, &ppcapPacketHeader,
		    (const u_char**)&pu8Payload);

		if (retval < 0) {
			if (strcmp("The interface went down",pcap_geterr(interface->ppcap)) == 0) {
			    fprintf(stderr, "rx: The interface went down\n");
			    exit(9);
			} else {
			    fprintf(stderr, "rx: %s\n", pcap_geterr(interface->ppcap));
			    exit(2);
			}
		}

		if (retval != 1) return;

		// fetch radiotap header length from radiotap header (seems to be 36 for Atheros and 18 for Ralink)
		u16HeaderLen = (pu8Payload[2] + (pu8Payload[3] << 8));
//		fprintf(stderr, "u16headerlen: %d\n", u16HeaderLen);

		bytes = ppcapPacketHeader->len - (u16HeaderLen + interface->n80211HeaderLength);
//		fprintf(stderr, "bytes: %d\n", bytes);

		if (bytes < 0) return;
//			printf("inside process_packet after bytes check\n");

		if (ieee80211_radiotap_iterator_init(&rti, (struct ieee80211_radiotap_header *)pu8Payload, ppcapPacketHeader->len) < 0) {
		    fprintf(stderr,"error radiotap_iterator_init");
		    exit(1);
		}
		//fprintf(stderr,"ppcapPacketHeader->len:%d\n",ppcapPacketHeader->len);

		while ((n = ieee80211_radiotap_iterator_next(&rti)) == 0) {
			switch (rti.this_arg_index) {
		    // we don't use these radiotap infos right now, disabled
		    /*
			case IEEE80211_RADIOTAP_RATE:
				prd.m_nRate = (*rti.this_arg);
				break;

			case IEEE80211_RADIOTAP_CHANNEL:
				prd.m_nChannel =
				    le16_to_cpu(*((u16 *)rti.this_arg));
				prd.m_nChannelFlags =
				    le16_to_cpu(*((u16 *)(rti.this_arg + 2)));
				break;

			case IEEE80211_RADIOTAP_ANTENNA:
				prd.m_nAntenna = (*rti.this_arg) + 1;
				break;
		    */
			case IEEE80211_RADIOTAP_FLAGS:
				prd.m_nRadiotapFlags = *rti.this_arg;
				break;
//			case IEEE80211_RADIOTAP_DBM_ANTSIGNAL:
//				rx_status->adapter[adapter_no].current_signal_dbm = (int8_t)(*rti.this_arg);
//				break;

			}
		}

		return;
}


long long current_timestamp() {
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    return milliseconds;
}


int main(int argc, char *argv[])
{
	monitor_interface_t interfaces[8];
	int num_interfaces = 0;
	int i;
	int packetcounter = 0;

	long long start_time = 0;
	long long now = 0;

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
		default:
			fprintf(stderr, "unknown switch %c\n", c);
			usage();
		}
	}

	if (optind >= argc)
		usage();
	
	int x = optind;
	while(x < argc && num_interfaces < 8) {
		open_and_configure_interface(argv[x], interfaces + num_interfaces);
		++num_interfaces;
		++x;
		usleep(10000); // wait a bit between configuring interfaces to reduce Atheros and Pi USB flakiness
	}

	start_time = current_timestamp();
	now = current_timestamp();

	int counter = 0;

	for(;;)	{

		fd_set readset;
		struct timeval to;

		to.tv_sec = 0;
		to.tv_usec = 1e5;
	
		FD_ZERO(&readset);
		for(i=0; i<num_interfaces; ++i)
			FD_SET(interfaces[i].selectable_fd, &readset);

		int n = select(30, &readset, NULL, NULL, &to);

		for(i=0; i<num_interfaces; ++i) {
			if(n == 0) {
//			    printf("n == 0\n");
			    //break;
			}
			if(FD_ISSET(interfaces[i].selectable_fd, &readset)) {
			    packetcounter++;
//			    printf("packetcount: %d\n", packetcounter);
			    process_packet(interfaces + i, i);
			}
		}
		now = current_timestamp();
		if (now - start_time > 1000) { // scan for 1 second
		    start_time = current_timestamp();
		    printf("%d,%d\n",counter, packetcounter);
		    fflush(stdout);
		    packetcounter = 0;
		    counter++;
		}
	}
	return (0);
}
