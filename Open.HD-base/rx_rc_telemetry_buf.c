// rx_rc_telemetry_buf by Rodizio. Based on wifibroadcast rx by Befinitiv. Mavlink code contributed by dino_de. Licensed under GPL2
// Dirty buffering implementation to cope with out-of-order reception of telemetry packets with multiple RX adapters.
// Will be integrated into rx_rc_telemetry in the future hopefully.
// If you have some C coding skills, a proper buffering implementation would be appreciated ;)
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
#include <fcntl.h>        // serialport
#include <termios.h>      // serialport
#include "mavlink/common/mavlink.h"

// this is where we store a summary of the information from the radiotap header
typedef struct {
	int m_nChannel;
	int m_nChannelFlags;
	int m_nRate;
	int m_nAntenna;
	int m_nRadiotapFlags;
} __attribute__((packed)) PENUMBRA_RADIOTAP_DATA;


typedef struct {
	uint32_t seq;
	uint16_t len;
	uint8_t filled;
	char payload[400];
} buffer_t;
buffer_t buffer[100];


//uint32_t seqbuffer[5];

int flagHelp = 0;
int param_baudrate = 0;
int param_rc_protocol = 0;
int param_output = 0;
int param_port = 0;
int param_debug = 0;
char *param_serialport = "none";

uint32_t seqno_telemetry = 0;
uint8_t seqno_rc = 0;
uint32_t seqnolast_telemetry = 0;
uint8_t seqnolast_rc = 0;

int seqnumplayed = 0;

int telemetry_received_yet = 0;
int rc_received_yet = 0;


int port_encoded;

wifibroadcast_rx_status_t *rx_status = NULL;
wifibroadcast_rx_status_t *rx_status_rc = NULL;

uint16_t sumdcrc = 0;
uint16_t ibuschecksum = 0;


int lastseq;

const uint16_t sumdcrc16_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, 0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

void usage(void) {
	printf(
	    "rx_rc_telemetry_buf by Rodizio. Based on wifibroadcast rx by Befinitiv. Mavlink code contributed by dino_de. Licensed under GPL2\n"
	    "Hopefully fixes out-of-order packet issues in rx_rc_telemetry\n"
	    "Quick hack that only supports telemetry, no R/C\n"
	    "\n"
	    "Usage: rx_rc_telemetry_buf [options] <interfaces>\n\nOptions\n"
	    "-o <output>     0 = output telemetry and R/C to serialport. 1 = output telemetry to STDOUT, R/C to serialport\n"
	    "-b <baudrate>   Serial port baudrate\n"
	    "-s <serialport> Serial port to use\n"
	    "-r <protocol>   R/C protocol to output. 0 = MSP. 1 = Mavlink. 2 = SUMD. 3 = IBUS. 4 = SRXL/XBUS. 99 = disable R/C\n"
	    "-p <port>       Port for telemetry data. Default = 1\n"
	    "-d <debug>      Debug. Set to 1 for debug output\n"
	    "\n"
	    "Example:\n"
	    "  rx_rc_telemetry_buf -o 0 -b 19200 -s /dev/serial0 -r 0 -p 1 wlan0\n"
	    "\n");
	exit(1);
}

typedef struct {
	pcap_t *ppcap;
	int selectable_fd;
	int n80211HeaderLength;
} monitor_interface_t;

// telemetry frame header consisting of seqnr and payload length
struct header_s {
	uint32_t seqnumber;
	uint16_t length;
}; // __attribute__ ((__packed__)); // not packed for now, doesn't work for some reason
struct header_s header;

/*
struct rcdata_s {
	unsigned int chan1 : 11;
	unsigned int chan2 : 11;
	unsigned int chan3 : 11;
	unsigned int chan4 : 11;
	unsigned int chan5 : 11;
	unsigned int chan6 : 11;
	unsigned int chan7 : 11;
	unsigned int chan8 : 11;
	unsigned int switches : 16;
} __attribute__ ((__packed__));
struct rcdata_s rcdata;
*/

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    return milliseconds;
}


void open_and_configure_interface(const char *name, monitor_interface_t *interface) {
	struct bpf_program bpfprogram;
	char szProgram[512];
	char szErrbuf[PCAP_ERRBUF_SIZE];

	int port_encoded = (param_port * 2) + 1;

	// open the interface in pcap
	szErrbuf[0] = '\0';

	interface->ppcap = pcap_open_live(name, 400, 0, -1, szErrbuf);
	if (interface->ppcap == NULL) {
		fprintf(stderr, "Unable to open %s: %s\n", name, szErrbuf);
		exit(1);
	}
	
	if(pcap_setnonblock(interface->ppcap, 1, szErrbuf) < 0) {
		fprintf(stderr, "Error setting %s to nonblocking mode: %s\n", name, szErrbuf);
	}

	int nLinkEncap = pcap_datalink(interface->ppcap);

	// match (RTS BF) or (DATA, DATA SHORT, RTS (and port))
	if (nLinkEncap == DLT_IEEE802_11_RADIO) {
	    if (param_rc_protocol != 99) { // only match on R/C packets if R/C enabled
		sprintf(szProgram, "ether[0x00:4] == 0xb4bf0000 || ((ether[0x00:2] == 0x0801 || ether[0x00:2] == 0x0802 || ether[0x00:4] == 0xb4010000) && ether[0x04:1] == 0x%.2x)", port_encoded);
	    } else {
		sprintf(szProgram, "(ether[0x00:2] == 0x0801 || ether[0x00:2] == 0x0802 || ether[0x00:4] == 0xb4010000) && ether[0x04:1] == 0x%.2x", port_encoded);
	    }
	} else {
		fprintf(stderr, "ERROR: unknown encapsulation on %s! check if monitor mode is supported and enabled\n", name);
		exit(1);
	}

	if (pcap_compile(interface->ppcap, &bpfprogram, szProgram, 1, 0) == -1) {
		puts(szProgram);
		puts(pcap_geterr(interface->ppcap));
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


void receive_packet(monitor_interface_t *interface, int adapter_no) {
    struct pcap_pkthdr * ppcapPacketHeader = NULL;
    struct ieee80211_radiotap_iterator rti;
    PENUMBRA_RADIOTAP_DATA prd;
    u8 payloadBuffer[300];
    u8 *pu8Payload = payloadBuffer;
    int bytes;
    int n;
    int retval;
    int u16HeaderLen;
    int type = 0; // r/c or telemetry
    int dbm_tmp;

    retval = pcap_next_ex(interface->ppcap, &ppcapPacketHeader, (const u_char**)&pu8Payload); // receive
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
//  fprintf(stderr, "u16headerlen: %d\n", u16HeaderLen);

    pu8Payload += u16HeaderLen;
    switch (pu8Payload[1]) {
    case 0xbf: // rts (R/C)
//	fprintf(stderr, "rts R/C frame\n");
        interface->n80211HeaderLength = 0x04;
	type = 0;
        break;
    case 0x01: // data short, rts (telemetry)
//	fprintf(stderr, "data short or rts telemetry frame\n");
        interface->n80211HeaderLength = 0x05;
	type = 1;
        break;
    case 0x02: // data (telemetry)
//	fprintf(stderr, "data telemetry frame\n");
        interface->n80211HeaderLength = 0x18;
	type = 1;
        break;
    default:
        break;
    }
    pu8Payload -= u16HeaderLen;

//  fprintf(stderr, "ppcapPacketHeader->len: %d\n", ppcapPacketHeader->len);
    if (ppcapPacketHeader->len < (u16HeaderLen + interface->n80211HeaderLength)) exit(1);

    bytes = ppcapPacketHeader->len - (u16HeaderLen + interface->n80211HeaderLength);
    if (param_debug == 1) fprintf(stderr, "len:%d ", bytes);
    if (bytes < 0) exit(1);

    if (ieee80211_radiotap_iterator_init(&rti, (struct ieee80211_radiotap_header *)pu8Payload, ppcapPacketHeader->len) < 0) exit(1);

    while ((n = ieee80211_radiotap_iterator_next(&rti)) == 0) {
	switch (rti.this_arg_index) {
	    case IEEE80211_RADIOTAP_FLAGS:
		prd.m_nRadiotapFlags = *rti.this_arg;
		break;
	    case IEEE80211_RADIOTAP_DBM_ANTSIGNAL:
//		fprintf(stderr, "ant signal:%d\n",(int8_t)(*rti.this_arg));
//		rx_status->adapter[adapter_no].current_signal_dbm = (int8_t)(*rti.this_arg);
		dbm_tmp = (int8_t)(*rti.this_arg);
		break;
	}
    }

    if (param_debug == 1) fprintf(stderr, "dbm:%d ", dbm_tmp);

    rx_status->adapter[adapter_no].current_signal_dbm = dbm_tmp;
    rx_status->adapter[adapter_no].received_packet_cnt++;
    rx_status->last_update = time(NULL);
    dbm_tmp = 0;
    pu8Payload += u16HeaderLen + interface->n80211HeaderLength;
    memcpy(&header,pu8Payload,6); //copy header (seqnum and length) to header struct
    pu8Payload += 6;

    int already_received = 0;
    int t;
    for (t=0;t<100;t++) {
	if (buffer[t].seq == header.seqnumber) { // seqnum has already been received
	    already_received = 1;
//	    fprintf(stderr,"already received\n");
	}
    }

    if ((already_received == 0) && (header.seqnumber > lastseq)) {
	int y;
	int done = 0;
	    for (y=0;y<100;y++) {
		if (buffer[y].filled == 0) { // buffer is empty, use it
		    memcpy(buffer[y].payload,pu8Payload,header.length);
		    buffer[y].len = header.length;
		    buffer[y].seq = header.seqnumber;
		    buffer[y].filled = 1;
		    if (param_debug == 1) fprintf(stderr,"seq %d->buf[%d] ",buffer[y].seq,y);
		    done = 1;
		} else {
		    //fprintf(stderr,"buffer[%d] already filled\n",y);
		}
		if (done == 1) {
		    //fprintf(stderr,"done!\n");
		    break;
		}
	    }
	    if (done == 0) { // no empty buffer found, we just use a filled one
		int randomnum = rand() % 100;
		memcpy(buffer[randomnum].payload,pu8Payload,header.length);
		buffer[randomnum].len = header.length;
		buffer[randomnum].seq = header.seqnumber;
		buffer[randomnum].filled = 1;
		if (param_debug == 1) fprintf(stderr,"FULL! seq %d->buf[%d] ",buffer[randomnum].seq,randomnum);
	    }

    } else {
	if (param_debug == 1) fprintf(stderr,"seq %d dupe ",header.seqnumber);
    }

}


void status_memory_init(wifibroadcast_rx_status_t *s) {
	s->received_block_cnt = 0;
	s->damaged_block_cnt = 0;
	s->received_packet_cnt = 0;
	s->lost_packet_cnt = 0;
	s->tx_restart_cnt = 0;
	s->wifi_adapter_cnt = 0;
	s->kbitrate = 0;

	int i;
	for(i=0; i<8; ++i) {
		s->adapter[i].received_packet_cnt = 0;
		s->adapter[i].wrong_crc_cnt = 0;
		s->adapter[i].current_signal_dbm = 0;
	}
}


wifibroadcast_rx_status_t *status_memory_open(void) {
	char buf[128];
	int fd;
	
	sprintf(buf, "/wifibroadcast_rx_status_%d",param_port);
	fd = shm_open(buf, O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0) {
		perror("shm_open"); exit(1);
	}
//	if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t)) == -1) {
//		perror("ftruncate"); exit(1);
//	}
	void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (retval == MAP_FAILED) {
		perror("mmap"); exit(1);
	}
	wifibroadcast_rx_status_t *tretval = (wifibroadcast_rx_status_t*)retval;
	status_memory_init(tretval);
	return tretval;
}

wifibroadcast_rx_status_t *status_memory_open_rc(void) {
	char buf[128];
	int fd;
	
	sprintf(buf, "/wifibroadcast_rx_status_rc");
	fd = shm_open(buf, O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0) {
		perror("shm_open"); exit(1);
	}
//	if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t)) == -1) {
//		perror("ftruncate"); exit(1);
//	}
	void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (retval == MAP_FAILED) {
		perror("mmap"); exit(1);
	}
	wifibroadcast_rx_status_t *tretval = (wifibroadcast_rx_status_t*)retval;
	status_memory_init(tretval);
	return tretval;
}



int main(int argc, char *argv[]) {
	printf("RX R/C Telemetry_buf started\n");

	srand(time(NULL));

	long long prev_time = 0;
	long long delta = 0;

	monitor_interface_t interfaces[8];
	int i, p, serialport, num_interfaces = 0;

	int writefailcounter, writesuccess, atleastonebufferfilled, lostpackets, nothing_received_cnt = 0;
	int abuffers_filled = 0;


	for (p=0;p<100;p++) {
	    buffer[p].filled = 0;
	    buffer[p].seq = 0;
	}

	while (1) {
		int nOptionIndex;
		static const struct option optiona[] = { { "help", no_argument, &flagHelp, 1 }, { 0, 0, 0, 0 } };
		int c = getopt_long(argc, argv, "h:o:b:s:r:p:d:", optiona, &nOptionIndex);
		if (c == -1)
			break;
		switch (c) {
		case 0: // long option
			break;
		case 'h': // help
			usage();
		case 'o': // output
			param_output = atoi(optarg);
			break;
		case 'b': // baudrate
			param_baudrate = atoi(optarg);
			break;
		case 's': // serialport
			param_serialport = optarg;
			break;
		case 'r': // R/C protocol
			param_rc_protocol = atoi(optarg);
			break;
		case 'p': // port
			param_port = atoi(optarg);
			break;
		case 'd': // debug
			param_debug = atoi(optarg);
			break;
		default:
			fprintf(stderr, "unknown switch %c\n", c);
			usage();
		}
	}
	if (optind >= argc) usage();
	int x = optind;

//	fprintf(stderr,"RX_RC_TELEMETRY: Serialport: %s\n",param_serialport);

        char path[45], line[100];
        FILE* procfile;

	while(x < argc && num_interfaces < 8) {
                snprintf(path, 45, "/sys/class/net/%s/device/uevent", argv[x]);
                procfile = fopen(path, "r");
                if(!procfile) {fprintf(stderr,"ERROR: opening %s failed!\n", path); return 0;}
                fgets(line, 100, procfile); // read the first line
                fgets(line, 100, procfile); // read the 2nd line
                if(strncmp(line, "DRIVER=ath9k_htc", 16) == 0) { // it's an atheros card
		    fprintf(stderr, "RX_RC_TELEMETRY: Driver: Atheros\n");
//                  rx_status->adapter[j].type = (int8_t)(0);
                } else { // ralink
		    fprintf(stderr, "RX_RC_TELEMETRY: Driver: Ralink\n");
//                  rx_status->adapter[j].type = (int8_t)(1);
                }
		open_and_configure_interface(argv[x], interfaces + num_interfaces);
		++num_interfaces;
                fclose(procfile);
		++x;
		usleep(10000); // wait a bit between configuring interfaces to reduce Atheros and Pi USB flakiness
	}

	rx_status = status_memory_open();
	rx_status->wifi_adapter_cnt = num_interfaces;
	fprintf(stderr, "RX_RC_TELEMETRY: rx_status->wifi_adapter_cnt: %d\n",rx_status->wifi_adapter_cnt);

	if (param_rc_protocol != 99) { // do not open rx status rc if no R/C output configured
	    rx_status_rc = status_memory_open_rc();
	    rx_status_rc->wifi_adapter_cnt = num_interfaces;
	    fprintf(stderr, "RX_RC_TELEMETRY: rx_status_rc->wifi_adapter_cnt: %d\n",rx_status_rc->wifi_adapter_cnt);
	}

	fprintf(stderr, "RX_RC_TELEMETRY: num_interfaces:%d\n",num_interfaces);

	if (param_baudrate != 0) {
	    serialport = open(param_serialport, O_WRONLY | O_NOCTTY | O_NDELAY);
	    //fprintf(stderr, "RX_RC_TELEMETRY: serialport return: %d\n",serialport);
	    if (serialport == -1) { // for some strange reason this doesn't work, although strace and the above fprintf shows -1
		printf("RX_RC_TELEMETRY ERROR: Unable to open UART. Ensure it is not in use by another application\n");
	    }
    	    switch (param_baudrate) {
    	    case 2400:
		{
                struct termios options;
                tcgetattr(serialport, &options);
                cfmakeraw(&options);
                cfsetispeed(&options, B2400);
                options.c_cflag &= ~CSIZE;
                options.c_cflag |= CS8; // Set 8 data bits
                options.c_cflag &= ~PARENB; // Set no parity
                options.c_cflag &= ~CSTOPB; // 1 stop bit
                options.c_lflag &= ~ECHO; // no echo
                options.c_cflag &= ~CRTSCTS; // no RTS/CTS Flow Control
                options.c_cflag |= CLOCAL; // Set local mode on
                tcsetattr(serialport, TCSANOW, &options); //write options
                printf("UART %s output set to %d baud\n",param_serialport,param_baudrate);
        	break;
		}
    	    case 4800:
		{
                struct termios options;
                tcgetattr(serialport, &options);
                cfmakeraw(&options);
                cfsetospeed(&options, B4800);
                options.c_cflag &= ~CSIZE;
                options.c_cflag |= CS8; // Set 8 data bits
                options.c_cflag &= ~PARENB; // Set no parity
                options.c_cflag &= ~CSTOPB; // 1 stop bit
                options.c_lflag &= ~ECHO; // no echo
                options.c_cflag &= ~CRTSCTS; // no RTS/CTS Flow Control
                options.c_cflag |= CLOCAL; // Set local mode on
                tcsetattr(serialport, TCSANOW, &options); //write options
                printf("UART %s output set to %d baud\n",param_serialport,param_baudrate);
        	break;
		}
    	    case 9600:
		{
                struct termios options;
                tcgetattr(serialport, &options);
                cfmakeraw(&options);
                cfsetospeed(&options, B9600);
                options.c_cflag &= ~CSIZE;
                options.c_cflag |= CS8; // Set 8 data bits
                options.c_cflag &= ~PARENB; // Set no parity
                options.c_cflag &= ~CSTOPB; // 1 stop bit
                options.c_lflag &= ~ECHO; // no echo
                options.c_cflag &= ~CRTSCTS; // no RTS/CTS Flow Control
                options.c_cflag |= CLOCAL; // Set local mode on
                tcsetattr(serialport, TCSANOW, &options); //write options
                printf("UART %s output set to %d baud\n",param_serialport,param_baudrate);
        	break;
		}
    	    case 19200:
		{
                struct termios options;
                tcgetattr(serialport, &options);
                cfmakeraw(&options);
                cfsetospeed(&options, B19200);
                options.c_cflag &= ~CSIZE;
                options.c_cflag |= CS8; // Set 8 data bits
                options.c_cflag &= ~PARENB; // Set no parity
                options.c_cflag &= ~CSTOPB; // 1 stop bit
                options.c_lflag &= ~ECHO; // no echo
                options.c_cflag &= ~CRTSCTS; // no RTS/CTS Flow Control
                options.c_cflag |= CLOCAL; // Set local mode on
                tcsetattr(serialport, TCSANOW, &options); //write options
                printf("UART %s output set to %d baud\n",param_serialport,param_baudrate);
        	break;
		}
    	    case 38400:
		{
                struct termios options;
                tcgetattr(serialport, &options);
                cfmakeraw(&options);
                cfsetospeed(&options, B38400);
                options.c_cflag &= ~CSIZE;
                options.c_cflag |= CS8; // Set 8 data bits
                options.c_cflag &= ~PARENB; // Set no parity
                options.c_cflag &= ~CSTOPB; // 1 stop bit
                options.c_lflag &= ~ECHO; // no echo
                options.c_cflag &= ~CRTSCTS; // no RTS/CTS Flow Control
                options.c_cflag |= CLOCAL; // Set local mode on
                tcsetattr(serialport, TCSANOW, &options); //write options
                printf("UART %s output set to %d baud\n",param_serialport,param_baudrate);
        	break;
		}
    	    case 57600:
		{
                struct termios options;
                tcgetattr(serialport, &options);
                cfmakeraw(&options);
                cfsetospeed(&options, B57600);
                options.c_cflag &= ~CSIZE;
                options.c_cflag |= CS8; // Set 8 data bits
                options.c_cflag &= ~PARENB; // Set no parity
                options.c_cflag &= ~CSTOPB; // 1 stop bit
                options.c_lflag &= ~ECHO; // no echo
                options.c_cflag &= ~CRTSCTS; // no RTS/CTS Flow Control
                options.c_cflag |= CLOCAL; // Set local mode on
                tcsetattr(serialport, TCSANOW, &options); //write options
                printf("UART %s output set to %d baud\n",param_serialport,param_baudrate);
        	break;
		}
    	    case 115200:
		{
                struct termios options;
                tcgetattr(serialport, &options);
                cfmakeraw(&options);
                cfsetospeed(&options, B115200);
                options.c_cflag &= ~CSIZE;
                options.c_cflag |= CS8; // Set 8 data bits
                options.c_cflag &= ~PARENB; // Set no parity
                options.c_cflag &= ~CSTOPB; // 1 stop bit
                options.c_lflag &= ~ECHO; // no echo
                options.c_cflag &= ~CRTSCTS; // no RTS/CTS Flow Control
                options.c_cflag |= CLOCAL; // Set local mode on
                tcsetattr(serialport, TCSANOW, &options); //write options
                printf("UART %s output set to %d baud\n",param_serialport,param_baudrate);
        	break;
		}
    	    default:
        	printf("ERROR: unsupported baudrate: %d\n", param_baudrate);
        	exit(1);
    	    }
	}

	lastseq = 0;


	for(;;) {
//	    fprintf(stderr,"---- loop start ---\n");
	    struct timeval to;

	    if (((writefailcounter > 0) || (abuffers_filled > 10)) && (nothing_received_cnt < 50)) { // if write has failed (i.e. there was no seqnum +1) last time, we set a short timeout to not wait too long for successive packets that may never arrive
		to.tv_sec = 0;
		to.tv_usec = 100; // 0.1ms timeout
//		fprintf(stderr,"0.5ms\n");
	    } else {
		to.tv_sec = 0;
		to.tv_usec = 500000; // 500ms timeout
//		fprintf(stderr,"500ms\n");
	    }

	    fd_set readset;
	    FD_ZERO(&readset);
	    for(i=0; i<num_interfaces; ++i) FD_SET(interfaces[i].selectable_fd, &readset);
	    int n = select(30, &readset, NULL, NULL, &to); // TODO: check how the 30 works exactly

	    int received_packet = 0;
	    for(i=0; i<num_interfaces; ++i) {
		if(n == 0) break;
//		printf("i: %d  ",i);
		if(FD_ISSET(interfaces[i].selectable_fd, &readset)) {
			if (param_debug == 1) fprintf(stderr,"  PKT=");
			received_packet = 1;
			receive_packet(interfaces + i, i);
		}
	    }
	    if ((param_debug == 1) && (received_packet == 1)) fprintf(stderr,"\n");

	    if ((param_debug == 1) && (writefailcounter > 1)) fprintf(stderr," writefailcounter:%d, nothing_received_count:%d\n",writefailcounter,nothing_received_cnt);

	    if ((writefailcounter >= 30) || (nothing_received_cnt > 0)){ // if we didn't write for the last N times (because seqnums were ooo or lost) set lastseq to lowest seq in buffer
		int lowestseq=2000000000; // just something very high
    		for (p=0;p<100;p++) {
		    // find out lowest filled seqnum here and write it to lastseq
		    if (buffer[p].filled == 1) { // only do stuff if buffer filled (i.e. seqnum is valid)
			if (buffer[p].seq < lowestseq) {
			    lowestseq = buffer[p].seq;
			}
		    }
		}
//		fprintf(stderr,"      writfailcounter:%d, lastseq:%d, lowestseq:%d\n",writefailcounter, lastseq, lowestseq);
		if (lowestseq != 2000000000) {
		    if (lowestseq > lastseq){
			lostpackets = lowestseq - lastseq - 1;
            		rx_status->lost_packet_cnt = (rx_status->lost_packet_cnt + lostpackets);
//			fprintf(stderr,"lowestseq > lasstseq, setting lastseq to lowestseq - 1. lostpackets:%d, rx_stats->lost_packet_cnt:%d\n",lostpackets,rx_status->lost_packet_cnt);
			lostpackets = 0;
			lastseq = lowestseq - 1;
		    } else {
//			fprintf(stderr,"       lowestseq NOT > lastseq, setting lastseq to 0\n");
			lastseq = 0;
		    }
		} else {
//		    fprintf(stderr,"       lowestseq = 2000000000, setting lastseq to 0\n");
		    lastseq = 0;
		}
//		writefailcounter=0; // TODO: check if needed?
	    }

	    if (received_packet == 0) {
		nothing_received_cnt++;
	    } else {
		nothing_received_cnt = 0;
		if (param_debug == 1) {
		    for (p=0;p<15;p++) {
			if (buffer[p].filled == 1) {
			    fprintf(stderr,"  B%d %d",p,buffer[p].seq);
			} else {
			    fprintf(stderr,"  b%d %d",p,buffer[p].seq);
			}
		    }
		}
		// count filled buffers
		abuffers_filled=0;
		for (p=0;p<100;p++) {
		    if (buffer[p].filled == 1) {
			abuffers_filled++;
		    }
		}
		if (param_debug == 1) fprintf(stderr," filled:%d\n",abuffers_filled);
	    }
	    received_packet = 0;

	    for (p=0;p<100;p++) {
		if (buffer[p].filled == 1) { // only do stuff if buffer filled (i.e. seqnum is valid)
//		    fprintf(stderr,"buffer[%d].seq:%d (lastseq:%d)\n",p,buffer[p].seq,lastseq);
		    atleastonebufferfilled = 1;
		    if (buffer[p].seq == lastseq + 1) {     // only write if seqs in order and no lost seqs
			delta = current_timestamp() - prev_time;
			prev_time = current_timestamp();
			write(STDOUT_FILENO, buffer[p].payload, buffer[p].len);
			if (param_debug == 1) fprintf(stderr,"written seqnum -----> %d delta:%lldms\n",buffer[p].seq,delta);
			lastseq = buffer[p].seq;
			buffer[p].filled = 0;
			writesuccess = 1;
			writefailcounter=0;
		    } else { // set buffer to empty if seq not zero, seq below lastseq and buffer filled
	    		if ((buffer[p].seq != 0) && (buffer[p].seq <= lastseq) && (buffer[p].filled == 1)) {
			    buffer[p].filled = 0;
			}
		    }
		}
	    }

	    if ((writesuccess == 0) && (atleastonebufferfilled == 1)){ //only increase writefailcounter if we had filled buffers but no matching seqnums (and there was actually something to be written out)
		writefailcounter++;
	    }
	    writesuccess=0;
	    atleastonebufferfilled=0;

	}
	return (0);
}
