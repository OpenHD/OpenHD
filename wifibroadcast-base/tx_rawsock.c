/*   tx_rawsock (c) 2017 Rodizio, based on wifibroadcast tx by Befinitiv
 *
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>
#include "fec.h"
#include "lib.h"
#include "wifibroadcast.h"
#include <netpacket/packet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <getopt.h>

#define MAX_PACKET_LENGTH 4192
#define MAX_USER_PACKET_LENGTH 2278
#define MAX_DATA_OR_FEC_PACKETS_PER_BLOCK 32

#define IEEE80211_RADIOTAP_MCS_HAVE_BW    0x01
#define IEEE80211_RADIOTAP_MCS_HAVE_MCS   0x02
#define IEEE80211_RADIOTAP_MCS_HAVE_GI    0x04
#define IEEE80211_RADIOTAP_MCS_HAVE_FMT   0x08

#define IEEE80211_RADIOTAP_MCS_BW_20    0
#define IEEE80211_RADIOTAP_MCS_BW_40    1
#define IEEE80211_RADIOTAP_MCS_BW_20L   2
#define IEEE80211_RADIOTAP_MCS_BW_20U   3
#define IEEE80211_RADIOTAP_MCS_SGI      0x04
#define IEEE80211_RADIOTAP_MCS_FMT_GF   0x08
#define IEEE80211_RADIOTAP_MCS_HAVE_FEC   0x10
#define IEEE80211_RADIOTAP_MCS_HAVE_STBC  0x20

#define IEEE80211_RADIOTAP_MCS_FEC_LDPC   0x10
#define	IEEE80211_RADIOTAP_MCS_STBC_MASK  0x60
#define	IEEE80211_RADIOTAP_MCS_STBC_1  1
#define	IEEE80211_RADIOTAP_MCS_STBC_2  2
#define	IEEE80211_RADIOTAP_MCS_STBC_3  3
#define	IEEE80211_RADIOTAP_MCS_STBC_SHIFT 5

int sock = 0;
int socks[4];

int skipfec = 0;
int block_cnt = 0;

int param_port = 0;

long long took_last = 0;
long long took = 0;

long long injection_time_now = 0;
long long injection_time_prev = 0;

int UseMCS=0;
int UseSTBC=0;
int UseLDPC=0;

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

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 8000;
    if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) fprintf(stderr,"setsockopt SO_SNDTIMEO\n");

    int sendbuff = 131072;
    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuff, sizeof(sendbuff)) < 0) fprintf(stderr,"setsockopt SO_SNDBUF\n");

    return sock;
}



static u8 u8aRadiotapHeader[] = {
	0x00, 0x00, // <-- radiotap version
	0x0c, 0x00, // <- radiotap header length
	0x04, 0x80, 0x00, 0x00, // <-- radiotap present flags (rate + tx flags)
	0x00, // datarate (will be overwritten later in packet_header_init)
	0x00, // ??
	0x00, 0x00 // ??
};

static u8 u8aRadiotapHeader80211N[]  __attribute__((unused)) = {
    0x00, 0x00, // <-- radiotap version
    0x0d, 0x00, // <- radiotap header length
    0x00, 0x80, 0x08, 0x00, // <-- radiotap present flags:  RADIOTAP_TX_FLAGS + RADIOTAP_MCS
    0x08, 0x00,  // RADIOTAP_F_TX_NOACK
    0 , 0, 0 // bitmap, flags, mcs_index
};

static u8 u8aIeeeHeader_data_short[] = {
	0x08, 0x01, 0x00, 0x00, // frame control field (2bytes), duration (2 bytes)
	0xff // port =  1st byte of IEEE802.11 RA (mac) must be something odd (wifi hardware determines broadcast/multicast through odd/even check)
};

static u8 u8aIeeeHeader_data[] = {
	0x08, 0x02, 0x00, 0x00, // frame control field (2bytes), duration (2 bytes)
	0xff, 0x00, 0x00, 0x00, 0x00, 0x00, // port = 1st byte of IEEE802.11 RA (mac) must be something odd (wifi hardware determines broadcast/multicast through odd/even check)
	0x13, 0x22, 0x33, 0x44, 0x55, 0x66, // mac
	0x13, 0x22, 0x33, 0x44, 0x55, 0x66, // mac
	0x00, 0x00 // IEEE802.11 seqnum, (will be overwritten later by Atheros firmware/wifi chip)
};

static u8 u8aIeeeHeader_rts[] = {
	0xb4, 0x01, 0x00, 0x00, // frame control field (2 bytes), duration (2 bytes)
	0xff, //  port = 1st byte of IEEE802.11 RA (mac) must be something odd (wifi hardware determines broadcast/multicast through odd/even check)
};

int flagHelp = 0;

void usage(void) {
    printf(
	"\nUsage: tx_rawsock [options] <interfaces>\n"
	"\n"
	"Options:\n"
	"-b <count>  Number of data packets in a block (default 8). Needs to match with rx.\n"
	"-r <count>  Number of FEC packets per block (default 4). Needs to match with rx.\n"
	"-f <bytes>  Number of bytes per packet (default %d, max. %d). This is also the FEC block size. Needs to match with rx.\n"
	"-m <bytes>  Minimum number of bytes per frame (default: 28)\n"
	"-p <port>   Port number 0-127 (default 0)\n"
	"-t <type>   Frame type to send. 0 = DATA short, 1 = DATA standard, 2 = RTS\n"
	"-d <rate>   Data rate to send frames with. Currently only supported with Ralink cards. Choose 6,12,18,24,36 Mbit\n"
	"-y <mode>   Transmission mode. 0 = send on all interfaces, 1 = send only on interface with best RSSI\n"
	"-M          Use 802.11N MCS modes: 0,1,2,3\n"
	"-S          Use STBC. Only for 802.11N\n  0\1\n"
	"-L          Use LDPC. Only for 802.11n and 8812AU 0\1\n"

        "\n"
        "Example:\n"
        "  cat /dev/zero | tx_rawsock -b 8 -r 4 -f 1024 -t 1 -d 24 -y 0 wlan0 (reads zeros from stdin and sends them out on wlan0) as standard DATA frames\n"
        "\n", 1024, MAX_USER_PACKET_LENGTH);
    exit(1);
}


typedef struct {
	int seq_nr;
	int fd;
	int curr_pb;
	packet_buffer_t *pbl;
} input_t;

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long useconds = te.tv_sec*1000000LL + te.tv_usec;
    return useconds;
}


long long injection_time = 0;
long long pm_now = 0;

int packet_header_init80211N(uint8_t *packet_header, int type, int port) {
	u8 *pu8 = packet_header;

	int port_encoded = 0;


	memcpy(packet_header, u8aRadiotapHeader80211N, sizeof(u8aRadiotapHeader80211N));
	pu8 += sizeof(u8aRadiotapHeader80211N);

	switch (type) {
	    case 0: // short DATA frame
		fprintf(stderr, "using short DATA frames\n");
		port_encoded = (port * 2) + 1;
		u8aIeeeHeader_data_short[4] = port_encoded; // 1st byte of RA mac is the port
		memcpy(pu8, u8aIeeeHeader_data_short, sizeof (u8aIeeeHeader_data_short)); //copy data short header to pu8
		pu8 += sizeof (u8aIeeeHeader_data_short);
		break;
	    case 1: // standard DATA frame
		fprintf(stderr, "using standard DATA frames\n");
		port_encoded = (port * 2) + 1;
		u8aIeeeHeader_data[4] = port_encoded; // 1st byte of RA mac is the port
		memcpy(pu8, u8aIeeeHeader_data, sizeof (u8aIeeeHeader_data)); //copy data header to pu8
		pu8 += sizeof (u8aIeeeHeader_data);
		break;
	    case 2: // RTS frame
		fprintf(stderr, "using RTS frames\n");
		port_encoded = (port * 2) + 1;
		u8aIeeeHeader_rts[4] = port_encoded; // 1st byte of RA mac is the port
		memcpy(pu8, u8aIeeeHeader_rts, sizeof (u8aIeeeHeader_rts));
		pu8 += sizeof (u8aIeeeHeader_rts);
		break;
	    default:
		fprintf(stderr, "ERROR: Wrong or no frame type specified (see -t parameter)\n");
		exit(1);
		break;
	}

	//determine the length of the header
	return pu8 - packet_header;
}

int packet_header_init(uint8_t *packet_header, int type, int rate, int port) {
	u8 *pu8 = packet_header;

	int port_encoded = 0;

	switch (rate) {
	    case 1:
		u8aRadiotapHeader[8]=0x02;
		break;
	    case 2:
		u8aRadiotapHeader[8]=0x04;
		break;
	    case 5: // 5.5
		u8aRadiotapHeader[8]=0x0b;
		break;
	    case 6:
		u8aRadiotapHeader[8]=0x0c;
		break;
	    case 11:
		u8aRadiotapHeader[8]=0x16;
		break;
	    case 12:
		u8aRadiotapHeader[8]=0x18;
		break;
	    case 18:
		u8aRadiotapHeader[8]=0x24;
		break;
	    case 24:
		u8aRadiotapHeader[8]=0x30;
		break;
	    case 36:
		u8aRadiotapHeader[8]=0x48;
		break;
	    case 48:
		u8aRadiotapHeader[8]=0x60;
		break;
	    default:
		fprintf(stderr, "ERROR: Wrong or no data rate specified (see -d parameter)\n");
		exit(1);
		break;
	}

	memcpy(packet_header, u8aRadiotapHeader, sizeof(u8aRadiotapHeader));
	pu8 += sizeof(u8aRadiotapHeader);

	switch (type) {
	    case 0: // short DATA frame
		fprintf(stderr, "using short DATA frames\n");
		port_encoded = (port * 2) + 1;
		u8aIeeeHeader_data_short[4] = port_encoded; // 1st byte of RA mac is the port
		memcpy(pu8, u8aIeeeHeader_data_short, sizeof (u8aIeeeHeader_data_short)); //copy data short header to pu8
		pu8 += sizeof (u8aIeeeHeader_data_short);
		break;
	    case 1: // standard DATA frame
		fprintf(stderr, "using standard DATA frames\n");
		port_encoded = (port * 2) + 1;
		u8aIeeeHeader_data[4] = port_encoded; // 1st byte of RA mac is the port
		memcpy(pu8, u8aIeeeHeader_data, sizeof (u8aIeeeHeader_data)); //copy data header to pu8
		pu8 += sizeof (u8aIeeeHeader_data);
		break;
	    case 2: // RTS frame
		fprintf(stderr, "using RTS frames\n");
		port_encoded = (port * 2) + 1;
		u8aIeeeHeader_rts[4] = port_encoded; // 1st byte of RA mac is the port
		memcpy(pu8, u8aIeeeHeader_rts, sizeof (u8aIeeeHeader_rts));
		pu8 += sizeof (u8aIeeeHeader_rts);
		break;
	    default:
		fprintf(stderr, "ERROR: Wrong or no frame type specified (see -t parameter)\n");
		exit(1);
		break;
	}

	//determine the length of the header
	return pu8 - packet_header;
}





int pb_transmit_packet(int seq_nr, uint8_t *packet_transmit_buffer, int packet_header_len, const uint8_t *packet_data, int packet_length, int num_interfaces, int param_transmission_mode, int best_adapter) {
    int i = 0;

    //add header outside of FEC
    wifi_packet_header_t *wph = (wifi_packet_header_t*)(packet_transmit_buffer + packet_header_len);
    wph->sequence_number = seq_nr;

    //copy data
    memcpy(packet_transmit_buffer + packet_header_len + sizeof(wifi_packet_header_t), packet_data, packet_length);

    int plen = packet_length + packet_header_len + sizeof(wifi_packet_header_t);

    if (best_adapter == 5) {
	for(i=0; i<num_interfaces; ++i) {
//	    if (write(socks[i], packet_transmit_buffer, plen) < 0 ) fprintf(stdout, "!");
	    if (write(socks[i], packet_transmit_buffer, plen) < 0 ) return 1;
	}
    } else {
//	if (write(socks[best_adapter], packet_transmit_buffer, plen) < 0 ) fprintf(stdout, "!");
	if (write(socks[best_adapter], packet_transmit_buffer, plen) < 0 ) return 1;
    }
    return 0;
}




void pb_transmit_block(packet_buffer_t *pbl, int *seq_nr, int port, int packet_length, uint8_t *packet_transmit_buffer, int packet_header_len, int data_packets_per_block, int fec_packets_per_block, int num_interfaces, int param_transmission_mode, telemetry_data_t *td1) {
	int i;
	uint8_t *data_blocks[MAX_DATA_OR_FEC_PACKETS_PER_BLOCK];
	uint8_t fec_pool[MAX_DATA_OR_FEC_PACKETS_PER_BLOCK][MAX_USER_PACKET_LENGTH];
	uint8_t *fec_blocks[MAX_DATA_OR_FEC_PACKETS_PER_BLOCK];

	for(i=0; i<data_packets_per_block; ++i) data_blocks[i] = pbl[i].data;

	if(fec_packets_per_block) {
		for(i=0; i<fec_packets_per_block; ++i) fec_blocks[i] = fec_pool[i];
		fec_encode(packet_length, data_blocks, data_packets_per_block, (unsigned char **)fec_blocks, fec_packets_per_block);
	}

	uint8_t *pb = packet_transmit_buffer;
	pb += packet_header_len;

	int di = 0;
	int fi = 0;
	int seq_nr_tmp = *seq_nr;
	long long prev_time = current_timestamp();
	int counterfec = 0;

	while(di < data_packets_per_block || fi < fec_packets_per_block) { //send data and FEC packets interleaved
	    int best_adapter = 0;
	    if(param_transmission_mode == 1) {
    		int i;
    		int ac = td1->rx_status->wifi_adapter_cnt;
    		int best_dbm = -1000;

    		// find out which card has best signal
    		for(i=0; i<ac; ++i) {
    		    if (best_dbm < td1->rx_status->adapter[i].current_signal_dbm) {
    			best_dbm = td1->rx_status->adapter[i].current_signal_dbm;
        		best_adapter = i;
    		    }
    		}
//    		printf ("bestadapter: %d (%d dbm)\n",best_adapter, best_dbm);
	    } else {
		best_adapter = 5; // set to 5 to let transmit packet function know it shall transmit on all interfaces
	    }

	    if(di < data_packets_per_block) {
		if (pb_transmit_packet(seq_nr_tmp, packet_transmit_buffer, packet_header_len, data_blocks[di], packet_length,num_interfaces, param_transmission_mode,best_adapter)) td1->tx_status->injection_fail_cnt++;
		seq_nr_tmp++;
		di++;
	    }

	    if(fi < fec_packets_per_block) {
		if (skipfec < 1) {
		    if (pb_transmit_packet(seq_nr_tmp, packet_transmit_buffer, packet_header_len, fec_blocks[fi], packet_length,num_interfaces,param_transmission_mode,best_adapter)) td1->tx_status->injection_fail_cnt++;
		} else {
		    if (counterfec % 2 == 0) {
			if (pb_transmit_packet(seq_nr_tmp, packet_transmit_buffer, packet_header_len, fec_blocks[fi], packet_length,num_interfaces,param_transmission_mode,best_adapter)) td1->tx_status->injection_fail_cnt++;
		    } else {
//			   fprintf(stdout,"not transmitted\n");
		    }
		    counterfec++;
		}
		seq_nr_tmp++;
		fi++;
	    }
	    skipfec--;
	}

	block_cnt++;
	td1->tx_status->injected_block_cnt++;

	took_last = took;
	took = current_timestamp() - prev_time;

//	if (took > 50) fprintf(stdout,"write took %lldus\n", took);
	if (took > (packet_length * (data_packets_per_block + fec_packets_per_block)) / 1.5 ) { // we simply assume 1us per byte = 1ms per 1024 byte packet (not very exact ...)
//	    fprintf(stdout,"\nwrite took %lldus skipping FEC packets ...\n", took);
	    skipfec=4;
	    td1->tx_status->skipped_fec_cnt = td1->tx_status->skipped_fec_cnt + skipfec;
	}

	if(block_cnt % 50 == 0) {
	    fprintf(stdout,"\t\t%d blocks sent, injection time per block %lldus, %d fecs skipped, %d packet injections failed.          \r", block_cnt,td1->tx_status->injection_time_block,td1->tx_status->skipped_fec_cnt,td1->tx_status->injection_fail_cnt);
	    fflush(stdout);
	}

	if (took < took_last) { // if we have a lower injection_time than last time, ignore
	    took = took_last;
	}

	injection_time_now = current_timestamp();
	if (injection_time_now - injection_time_prev > 220) {
	    injection_time_prev = current_timestamp();
	    td1->tx_status->injection_time_block = took;
	    took=0;
	    took_last=0;
	}


	*seq_nr += data_packets_per_block + fec_packets_per_block;

	//reset the length back
	for(i=0; i< data_packets_per_block; ++i) pbl[i].len = 0;

}

void status_memory_init(wifibroadcast_tx_status_t *s) {
    s->last_update = 0;
    s->injected_block_cnt = 0;
    s->skipped_fec_cnt = 0;
    s->injection_fail_cnt = 0;
    s->injection_time_block = 0;
}


wifibroadcast_rx_status_t *telemetry_wbc_status_memory_open(void) {
    int fd = 0;
//    int sharedmem = 0;
    // TODO: Clean up rx_status shared memory handling
//    if (transmission_mode == 1) {
//	while(sharedmem == 0) {
    	    fd = shm_open("/wifibroadcast_rx_status_0", O_RDWR, S_IRUSR | S_IWUSR);
        	if(fd < 0) {
    //        	    fprintf(stderr, "Could not open wifibroadcast rx status - retrying ...\n");
        	} else {
//            	    sharedmem = 1;
        	}
//        	usleep(150000);
//	}
//        if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t)) == -1) {
//                perror("ftruncate");
//                exit(1);
//        }
        void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
//        if (retval == MAP_FAILED) {
//                perror("mmap");
//                exit(1);
//        }
//    }
    return (wifibroadcast_rx_status_t*)retval;
}

wifibroadcast_tx_status_t *telemetry_wbc_status_memory_open_tx(void) {
	int fd = 0;
	char buf[128];
	int sharedmem = 0;
	// TODO: Clean up rx_status shared memory handling
	while(sharedmem == 0) {
	    sprintf(buf, "/wifibroadcast_tx_status_%d", param_port);
    	    fd = shm_open(buf, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        	if(fd < 0) {
            	    fprintf(stderr, "Could not open wifibroadcast tx status - retrying ...\n");
        	} else {
            	    sharedmem = 1;
        	}
        	usleep(150000);
	}
        if (ftruncate(fd, sizeof(wifibroadcast_tx_status_t)) == -1) {
                perror("ftruncate");
                exit(1);
        }
        void *retval = mmap(NULL, sizeof(wifibroadcast_tx_status_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (retval == MAP_FAILED) {
                perror("mmap");
                exit(1);
        }
	wifibroadcast_tx_status_t *tretval = (wifibroadcast_tx_status_t*)retval;
	status_memory_init(tretval);
	return tretval;
}



void telemetry_init(telemetry_data_t *td) {
    td->rx_status = telemetry_wbc_status_memory_open();
    td->tx_status = telemetry_wbc_status_memory_open_tx();
}


int main(int argc, char *argv[]) {

    setpriority(PRIO_PROCESS, 0, -10);

    char fBrokenSocket = 0;
    int pcnt = 0;
    uint8_t packet_transmit_buffer[MAX_PACKET_LENGTH];
    size_t packet_header_length = 0;
    input_t input;

    int param_data_packets_per_block = 8;
    int param_fec_packets_per_block = 4;
    int param_packet_length = 1024;
    int param_min_packet_length = 24;
    int param_packet_type = 1;
    int param_data_rate = 18;
    int param_transmission_mode = 0;

    printf("tx_rawsock (c)2017 by Rodizio. Based on wifibroadcast tx by Befinitiv. GPL2 licensed.\n");

    while (1) {
	int nOptionIndex;
	static const struct option optiona[] = {
	    { "help", no_argument, &flagHelp, 1 },
	    { 0, 0, 0, 0 }
	};

	int c = getopt_long(argc, argv, "h:r:f:p:b:m:t:d:y:M:S:L:", optiona, &nOptionIndex);
	if (c == -1) break;
	switch (c) {
	    case 0: // long option
		break;
	    case 'h': // help
		usage();
		break;
	    case 'r': // retransmissions
		param_fec_packets_per_block = atoi(optarg);
		break;
	    case 'f': // MTU
		param_packet_length = atoi(optarg);
		break;
	    case 'p': //port
		param_port = atoi(optarg);
		break;
	    case 'b': //retransmission block size
		param_data_packets_per_block = atoi(optarg);
		break;
	    case 'm': //minimum packet length
		param_min_packet_length = atoi(optarg);
		break;
	    case 't': // packet type
		param_packet_type = atoi(optarg);
		break;
	    case 'd': // data rate
		param_data_rate = atoi(optarg);
		break;
	    case 'y': // transmission mode
		param_transmission_mode = atoi(optarg);
		break;
            case 'M': //use 802.11N datarates
		UseMCS = atoi(optarg);
		break;
            case 'S': //use STBC
                UseSTBC = atoi(optarg);
                break;
            case 'L': //use LDPC
                UseLDPC = atoi(optarg);
                break;

	    default:
		fprintf(stderr, "unknown switch %c\n", c);
		usage();
		break;
	}
    }

    if (optind >= argc) usage();

    if(param_packet_length > MAX_USER_PACKET_LENGTH) {
	fprintf(stderr, "ERROR; Packet length is limited to %d bytes (you requested %d bytes)\n", MAX_USER_PACKET_LENGTH, param_packet_length);
	return (1);
    }

    if(param_min_packet_length > param_packet_length) {
	fprintf(stderr, "ERROR; Minimum packet length is higher than maximum packet length (%d > %d)\n", param_min_packet_length, param_packet_length);
	return (1);
    }

    if(param_data_packets_per_block > MAX_DATA_OR_FEC_PACKETS_PER_BLOCK || param_fec_packets_per_block > MAX_DATA_OR_FEC_PACKETS_PER_BLOCK) {
	fprintf(stderr, "ERROR: Data and FEC packets per block are limited to %d (you requested %d data, %d FEC)\n", MAX_DATA_OR_FEC_PACKETS_PER_BLOCK, param_data_packets_per_block, param_fec_packets_per_block);
	return (1);
    }

    if(UseMCS == 1)
    {
	fprintf(stderr, "Using 802.11N mode\n");
	u8 mcs_flags = 0;
	u8 mcs_known = (IEEE80211_RADIOTAP_MCS_HAVE_MCS | IEEE80211_RADIOTAP_MCS_HAVE_BW | IEEE80211_RADIOTAP_MCS_HAVE_GI | IEEE80211_RADIOTAP_MCS_HAVE_STBC | IEEE80211_RADIOTAP_MCS_HAVE_FEC);

	if(UseSTBC == 1 )
	{
		fprintf(stderr, "STBC enabled\n");
		mcs_flags = mcs_flags | IEEE80211_RADIOTAP_MCS_STBC_1 << IEEE80211_RADIOTAP_MCS_STBC_SHIFT;
	}

	if(UseLDPC == 1 )
	{
		fprintf(stderr, "LDPC enabled\n");
		mcs_flags = mcs_flags | IEEE80211_RADIOTAP_MCS_FEC_LDPC;
	}

	u8aRadiotapHeader80211N[10] = mcs_known;
	u8aRadiotapHeader80211N[11] = mcs_flags;
	u8aRadiotapHeader80211N[12] = param_data_rate;
         packet_header_length = packet_header_init80211N(packet_transmit_buffer, param_packet_type,  param_port);
    }
    else
    {
        packet_header_length = packet_header_init(packet_transmit_buffer, param_packet_type, param_data_rate, param_port);
    }

    input.fd = STDIN_FILENO;
    input.seq_nr = 0;
    input.curr_pb = 0;
    input.pbl = lib_alloc_packet_buffer_list(param_data_packets_per_block, MAX_PACKET_LENGTH);

    //prepare the buffers with headers
    int j = 0;
    for(j=0; j<param_data_packets_per_block; ++j) {
    	input.pbl[j].len = 0;
    }

    //initialize forward error correction
    fec_init();

    // initialize telemetry shared mem for rssi based transmission (-y 1)
    telemetry_data_t td;
    telemetry_init(&td);

    int x = optind;
    int num_interfaces = 0;

    while(x < argc && num_interfaces < 4) {
	socks[num_interfaces] = open_sock(argv[x]);
        ++num_interfaces;
        ++x;
        usleep(20000); // wait a bit between configuring interfaces to reduce Atheros and Pi USB flakiness
    }

    while (!fBrokenSocket) {

	packet_buffer_t *pb = input.pbl + input.curr_pb;

	// if the buffer is fresh we add a payload header
	if(pb->len == 0) {
	    pb->len += sizeof(payload_header_t); //make space for a length field (will be filled later)
	}

	int inl = read(input.fd, pb->data + pb->len, param_packet_length - pb->len); //read the data
	if(inl < 0 || inl > param_packet_length-pb->len) {
	    perror("reading stdin");
	    return 1;
	}

	if(inl == 0) { // EOF
	    fprintf(stderr, "Warning: Lost connection to stdin. Please make sure that a data source is connected\n");
	    usleep(5e5);
	    continue;
	}

	pb->len += inl;

	// check if this packet is finished
	if(pb->len >= param_min_packet_length) {
	    payload_header_t *ph = (payload_header_t*)pb->data;
	    // write the length into the packet. this is needed since with fec we cannot use the wifi packet lentgh anymore.
	    // We could also set the user payload to a fixed size but this would introduce additional latency since tx would need to wait until that amount of data has been received
	    ph->data_length = pb->len - sizeof(payload_header_t);
	    pcnt++;
	    // check if this block is finished
	    if(input.curr_pb == param_data_packets_per_block-1) {
		pb_transmit_block(input.pbl, &(input.seq_nr), param_port, param_packet_length, packet_transmit_buffer, packet_header_length, param_data_packets_per_block, param_fec_packets_per_block, num_interfaces, param_transmission_mode, &td);
		input.curr_pb = 0;
	    } else {
		input.curr_pb++;
	    }
	}
    }

    printf("ERROR: Broken socket!\n");
    return (0);
}
