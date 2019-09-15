// tx_telemetry (c)2017 by Rodizio. Based on wifibroadcast tx by Befinitiv. GPL2 licensed.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>
#include "lib.h"
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
#include "mavlink/common/mavlink.h"

int sock = 0;
int socks[5];
int type[5];

//struct timeval time;

mavlink_status_t status;
mavlink_message_t msg;

uint8_t headers_atheros[40]; // header buffer for atheros
uint8_t headers_ralink[40]; // header buffer for ralink
uint8_t headers_Realtek[40]; // header buffer for Realtek

int headers_atheros_len = 0;
int headers_ralink_len = 0;
int headers_Realtek_len = 0;

uint8_t packet_buffer_ath[402]; // wifi packet to be sent (263 + len and seq + radiotap and ieee headers)
uint8_t packet_buffer_ral[402]; // wifi packet to be sent (263 + len and seq + radiotap and ieee headers)
uint8_t packet_buffer_rea[402]; // wifi packet to be sent (263 + len and seq + radiotap and ieee headers)

// telemetry frame header consisting of seqnr and payload length
struct header_s {
    uint32_t seqnumber;
    uint16_t length;
} __attribute__ ((__packed__)); // TODO: check if packed works as intended
struct header_s header;

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

static u8 u8aRadiotapHeader[] = {
        0x00, 0x00, // <-- radiotap version
        0x0c, 0x00, // <- radiotap header length
        0x04, 0x80, 0x00, 0x00, // <-- radiotap present flags
        0x00, // datarate (will be overwritten later)
        0x00,
        0x00, 0x00
};

static u8 u8aRadiotapHeader80211n[] = {
	    0x00, 0x00, // <-- radiotap version
	    0x0d, 0x00, // <- radiotap header length
	    0x00, 0x80, 0x08, 0x00, // <-- radiotap present flags (tx flags, mcs)
	    0x08, 0x00, 	// tx-flag
	    0x37, 			// mcs have: bw, gi, stbc ,fec
	    0x30,			// mcs: 20MHz bw, long guard interval, stbc, ldpc 
	    0x00,			// mcs index 0 (speed level, will be overwritten later)
};

static u8 u8aIeeeHeader_data[] = {
        0x08, 0x02, 0x00, 0x00, // frame control field (2 bytes), duration (2 bytes)
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00,// 1st byte of MAC will be overwritten with encoded port
        0x13, 0x22, 0x33, 0x44, 0x55, 0x66, // mac
        0x13, 0x22, 0x33, 0x44, 0x55, 0x66, // mac
        0x00, 0x00 // IEEE802.11 seqnum, (will be overwritten later by Atheros firmware/wifi chip)
};

static u8 u8aIeeeHeader_data_short[] = {
        0x08, 0x01, 0x00, 0x00, // frame control field (2 bytes), duration (2 bytes)
        0xff // 1st byte of MAC will be overwritten with encoded port
};

static u8 u8aIeeeHeader_rts[] = {
        0xb4, 0x01, 0x00, 0x00, // frame control field (2 bytes), duration (2 bytes)
        0xff // 1st byte of MAC will be overwritten with encoded port
};

static u8 dummydata[] = {
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0xdd
};

int flagHelp = 0;

void usage(void) {
    printf(
	"\nUsage: tx_telemetry [options] <interfaces>\n\n"
	"Options:\n"
	"-c <cts>    0 = CTS protection disabled. 1 = CTS protection enabled (Atheros only)\n"
	"-p <port>   Port. Default = 1\n"
	"-r <count>  Retransmission count. 1 = send each frame once, 2 = twice, 3 = three times ... Default = 1\n"
	"-x <type>   Telemetry protocol to be used. 0 = Mavlink. 1 = generic (for all others).\n"
	"-d <rate>   Data rate to send frames with. Currently only supported with Ralink cards. Choose 6,12,18,24,36 Mbit\n"
	"-y <mode>   Transmission mode. 0 = send on all interfaces, 1 = send only on interface with best RSSI\n"
	"-z <debug>  Debug. 0 = disable debug output, 1 = enable debug output\n\n"
        "Example:\n"
        "  cat /dev/serial0 | tx_telemetry -c 0 -p 1 -r 1 -x 0 -d 24 -y 0 wlan0\n"
        "\n");
    exit(1);
}

wifibroadcast_rx_status_t *telemetry_wbc_status_memory_open(void) {
    int fd = 0;

    fd = shm_open("/wifibroadcast_rx_status_0", O_RDONLY, S_IRUSR | S_IWUSR);
    if(fd < 0) {
	fprintf(stderr, "ERROR: Could not open wifibroadcast rx status!\n");
	exit(1);
    }

    void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t), PROT_READ, MAP_SHARED, fd, 0);
    if (retval == MAP_FAILED) { perror("mmap"); exit(1); }
    return (wifibroadcast_rx_status_t*)retval;
}

void telemetry_init(telemetry_data_t *td) {
    td->rx_status = telemetry_wbc_status_memory_open();
}

void sendpacket(uint32_t seqno, uint16_t len, telemetry_data_t *td, int transmission_mode, int num_int, uint8_t data[302]) {
	header.seqnumber = seqno;
	header.length = len;
//	fprintf(stderr,"seqno: %d",seqno);
	int padlen = 0;
	int best_adapter = 0;
	if(transmission_mode == 1) {
	    int i;
	    int best_dbm = -1000;
	    // find out which card has best signal
	    for(i=0; i<num_int; ++i) {
	    	if (best_dbm < td->rx_status->adapter[i].current_signal_dbm) {
		    best_dbm = td->rx_status->adapter[i].current_signal_dbm;
		    best_adapter = i;
		}
	    }
//	    printf ("bestadapter: %d (%d dbm)\n",best_adapter, best_dbm);
		switch (type[best_adapter]) {
	    case 0: // type: Ralink
		    // telemetry header (seqno and len)
		    memcpy(packet_buffer_ral + headers_ralink_len, &header, 6);
		    memcpy(packet_buffer_ral + headers_ralink_len + 6, data, len);
		    if (len < 18) { // pad to minimum length
			padlen = 18 - len;
			memcpy(packet_buffer_ral + headers_ralink_len + 6 + len, dummydata, padlen);
		    }
		    if (write(socks[best_adapter], &packet_buffer_ral, headers_ralink_len + 6 + len + padlen) < 0 ) { fprintf(stderr, "."); exit(1); }
		break;
	    case 1: // type: atheros
		    memcpy(packet_buffer_ath + headers_atheros_len, &header, 6);
		    // telemetry data
		    memcpy(packet_buffer_ath + headers_atheros_len + 6, data, len);
		    if (len < 5) {
			padlen = 5 - len;
			memcpy(packet_buffer_ath + headers_atheros_len + 6 + len, dummydata, padlen);
		    }
		    if (write(socks[best_adapter], &packet_buffer_ath, headers_atheros_len + 6 + len + padlen) < 0 ) { fprintf(stderr, "."); exit(1); }
		break;
	    case 2: // type: Realtek
		    memcpy(packet_buffer_rea + headers_Realtek_len, &header, 6);
		    memcpy(packet_buffer_rea + headers_Realtek_len + 6, data, len);
		    if (len < 5) { // if telemetry payload is too short, pad to minimum length
			padlen = 5 - len;
			memcpy(packet_buffer_rea + headers_Realtek_len + 6 + len, dummydata, padlen);
		    }
		    if (write(socks[best_adapter], &packet_buffer_rea, headers_Realtek_len + 6 + len + padlen) < 0 ) { fprintf(stderr, "."); exit(1); }	
		break;
	    default:
		fprintf(stderr, "ERROR: Wrong or no frame type specified (see -t parameter)\n");
		exit(1);
		break;
	    }
	} else { // transmit on all interfaces
	    int i;
	    for(i=0; i<num_int; ++i) {		
		switch (type[i]) {
	    case 0: // type: Ralink
		    // telemetry header (seqno and len)
		    memcpy(packet_buffer_ral + headers_ralink_len, &header, 6);
		    memcpy(packet_buffer_ral + headers_ralink_len + 6, data, len);
		    if (len < 18) { // pad to minimum length
			padlen = 18 - len;
			memcpy(packet_buffer_ral + headers_ralink_len + 6 + len, dummydata, padlen);
		    }
		    if (write(socks[i], &packet_buffer_ral, headers_ralink_len + 6 + len + padlen) < 0 ) { fprintf(stderr, "."); exit(1); }
		break;
	    case 1: // type: atheros
		    memcpy(packet_buffer_ath + headers_atheros_len, &header, 6);
		    // telemetry data
		    memcpy(packet_buffer_ath + headers_atheros_len + 6, data, len);
		    if (len < 5) {
			padlen = 5 - len;
			memcpy(packet_buffer_ath + headers_atheros_len + 6 + len, dummydata, padlen);
		    }
		    if (write(socks[i], &packet_buffer_ath, headers_atheros_len + 6 + len + padlen) < 0 ) { fprintf(stderr, "."); exit(1); }
		break;
	    case 2: // type: Realtek
		    memcpy(packet_buffer_rea + headers_Realtek_len, &header, 6);
		    memcpy(packet_buffer_rea + headers_Realtek_len + 6, data, len);
		    if (len < 5) { // if telemetry payload is too short, pad to minimum length
			padlen = 5 - len;
			memcpy(packet_buffer_rea + headers_Realtek_len + 6 + len, dummydata, padlen);
		    }
		    if (write(socks[i], &packet_buffer_rea, headers_Realtek_len + 6 + len + padlen) < 0 ) { fprintf(stderr, "."); exit(1); }	
		break;
	    default:
		fprintf(stderr, "ERROR: Wrong or no frame type specified (see -t parameter)\n");
		exit(1);
		break;
	    }
	    }
	}
}

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    return milliseconds;
}

int main(int argc, char *argv[]) {
    char fBrokenSocket = 0;
    char line[100], path[100];
    FILE* procfile;

    int pcnt = 0;
    int port_encoded = 0;
    int param_cts = 0;
    int param_port = 1;
    int param_retransmissions = 1;
    int param_telemetry_protocol = 0;
    int param_data_rate = 12;
    int param_transmission_mode = 0;
    int param_debug = 0;

    uint8_t buf[402]; // data read from stdin
    uint8_t mavlink_message[263];

    uint16_t len_msg = 0;
    uint32_t seqno = 0;

    fprintf(stdout,"tx_telemetry (c)2017 by Rodizio. Based on wifibroadcast tx by Befinitiv. GPL2 licensed.\n");

    while (1) {
	int nOptionIndex;
	static const struct option optiona[] = {
	    { "help", no_argument, &flagHelp, 1 },
	    { 0, 0, 0, 0 }
	};

	int c = getopt_long(argc, argv, "h:c:p:r:x:d:y:z:", optiona, &nOptionIndex);
	if (c == -1) break;
	switch (c) {
	    case 0: // long option
		break;
	    case 'h': // help
		usage();
		break;
	    case 'c': // CTS protection
		param_cts = atoi(optarg);
		break;
	    case 'p': // port
		param_port = atoi(optarg);
		break;
	    case 'r': // retransmissions
		param_retransmissions = atoi(optarg);
		break;
	    case 'x': // telemetry protocol
		param_telemetry_protocol = atoi(optarg);
		break;
	    case 'd': // data rate
		param_data_rate = atoi(optarg);
		break;
	    case 'y': // transmission mode
		param_transmission_mode = atoi(optarg);
		break;
	    case 'z': // debug
		param_debug = atoi(optarg);
		break;
	    default:
		fprintf(stderr, "unknown switch %c\n", c);
		usage();
		break;
	}
    }

    if (optind >= argc) usage();

    int x = optind;
    int num_interfaces = 0;

    while(x < argc && num_interfaces < 5) {
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
			  fprintf(stderr, "tx_telemetry: Atheros card detected\n");
	          type[num_interfaces] = 1;
		} else {
			  fprintf(stderr, "tx_telemetry: Realtek card detected\n");
			  type[num_interfaces] = 2;
		}
    } else { // ralink or mediatek
              fprintf(stderr, "tx_telemetry: Ralink card detected\n");
	          type[num_interfaces] = 0;
    }
	socks[num_interfaces] = open_sock(argv[x]);
        ++num_interfaces;
	++x;
        fclose(procfile);
        usleep(10000); // wait a bit between configuring interfaces to reduce Atheros and Pi USB flakiness
    }

    // initialize telemetry shared mem for rssi based transmission (-y 1)
    telemetry_data_t td;
    telemetry_init(&td);

    switch (param_data_rate) {
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
            fprintf(stderr, "tx_telemetry: ERROR: Wrong or no data rate specified (see -d parameter)\n");
            exit(1);
            break;
    }

    port_encoded = (param_port * 2) + 1;
    u8aIeeeHeader_rts[4] = port_encoded;
    u8aIeeeHeader_data[4] = port_encoded;
    u8aIeeeHeader_data_short[4] = port_encoded;

    // for Atheros use data frames if CTS protection enabled or rts if disabled
    // CTS protection causes R/C transmission to stop for some reason, always use rts frames (i.e. no cts protection)
    //param_cts = 0;
    if(param_cts == 1) { // use data frames
	memcpy(headers_atheros, u8aRadiotapHeader, sizeof(u8aRadiotapHeader)); // radiotap header
	memcpy(headers_atheros + sizeof(u8aRadiotapHeader), u8aIeeeHeader_data, sizeof(u8aIeeeHeader_data)); // ieee header
	headers_atheros_len = sizeof(u8aRadiotapHeader) + sizeof(u8aIeeeHeader_data);
    } else { // use rts frames
	memcpy(headers_atheros, u8aRadiotapHeader, sizeof(u8aRadiotapHeader)); // radiotap header
	memcpy(headers_atheros + sizeof(u8aRadiotapHeader), u8aIeeeHeader_rts, sizeof(u8aIeeeHeader_rts)); // ieee header
	headers_atheros_len = sizeof(u8aRadiotapHeader) + sizeof(u8aIeeeHeader_rts);
    }

    // for Ralink always use data short
    memcpy(headers_ralink, u8aRadiotapHeader, sizeof(u8aRadiotapHeader));// radiotap header
    memcpy(headers_ralink + sizeof(u8aRadiotapHeader), u8aIeeeHeader_data_short, sizeof(u8aIeeeHeader_data_short));// ieee header
    headers_ralink_len = sizeof(u8aRadiotapHeader) + sizeof(u8aIeeeHeader_data_short);
	
	// for Realtek use rts frames
	memcpy(headers_Realtek, u8aRadiotapHeader80211n, sizeof(u8aRadiotapHeader80211n)); // radiotap header
	memcpy(headers_Realtek + sizeof(u8aRadiotapHeader80211n), u8aIeeeHeader_rts, sizeof(u8aIeeeHeader_rts)); // ieee header
	headers_Realtek_len = sizeof(u8aRadiotapHeader80211n) + sizeof(u8aIeeeHeader_rts);

    // radiotap and ieee headers
    memcpy(packet_buffer_ath, headers_atheros, headers_atheros_len);
    memcpy(packet_buffer_ral, headers_ralink, headers_ralink_len);
	memcpy(packet_buffer_rea, headers_Realtek, headers_Realtek_len);

    long long prev_time = current_timestamp();
    long long prev_time_read = current_timestamp();

    while (!fBrokenSocket) {
	int inl = read(STDIN_FILENO, buf, 350); // read the data
	if (param_debug == 1) {
	    long long took_read = current_timestamp() - prev_time_read;
	    prev_time_read = current_timestamp();
	    fprintf(stderr, "read delta:%lldms bytes:%d ",took_read,inl);
	}

	if(inl < 0) { fprintf(stderr,"tx_telemetry: ERROR: reading stdin"); return 1; }
	if(inl > 350) { fprintf(stderr,"tx_telemetry: Warning: Input data > 300 bytes"); continue; }
	if(inl == 0) { fprintf(stderr, "tx_telemetry: Warning: Lost connection to stdin\n"); usleep(1e5); continue;} // EOF

	if (param_telemetry_protocol == 0) { // parse Mavlink
	    int i = 0;
	    for(i=0; i < inl; i++) {
		uint8_t c = buf[i];
		if (mavlink_parse_char(0, c, &msg, &status)) {
		    if (param_debug == 1) {
			long long took = current_timestamp() - prev_time;
			prev_time = current_timestamp();
			fprintf(stderr, "Msg delta:%lldms seq:%d sysid:%d, compid:%d  ",took, msg.seq, msg.sysid, msg.compid);
            		switch (msg.msgid){
                    	    case MAVLINK_MSG_ID_SYS_STATUS:
				fprintf(stderr, "SYS_STATUS ");
                        	break;
                    	    case MAVLINK_MSG_ID_HEARTBEAT:
				fprintf(stderr, "HEARTBEAT ");
                        	break;
                    	    case MAVLINK_MSG_ID_COMMAND_ACK:
				fprintf(stderr, "COMMAND_ACK:%d ",mavlink_msg_command_ack_get_command(&msg));
                        	break;
                    	    case MAVLINK_MSG_ID_COMMAND_INT:
				fprintf(stderr, "COMMAND_INT:%d ",mavlink_msg_command_int_get_command(&msg));
                        	break;
                    	    case MAVLINK_MSG_ID_EXTENDED_SYS_STATE:
				fprintf(stderr, "EXTENDED_SYS_STATE: vtol_state:%d, landed_state:%d",mavlink_msg_extended_sys_state_get_vtol_state(&msg),mavlink_msg_extended_sys_state_get_landed_state(&msg));
                        	break;
                    	    case MAVLINK_MSG_ID_LOCAL_POSITION_NED:
				fprintf(stderr, "LOCAL_POSITION_NED ");
                        	break;
                    	    case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:
				fprintf(stderr, "POSITION_TARGET_LOCAL_NED ");
                        	break;
                    	    case MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT:
				fprintf(stderr, "POSITION_TARGET_GLOBAL_INT ");
                        	break;
                    	    case MAVLINK_MSG_ID_ESTIMATOR_STATUS:
				fprintf(stderr, "ESTIMATOR_STATUS ");
                        	break;
                    	    case MAVLINK_MSG_ID_HOME_POSITION:
				fprintf(stderr, "HIGHRES_IMU ");
                        	break;
                    	    case MAVLINK_MSG_ID_NAMED_VALUE_FLOAT:
				fprintf(stderr, "NAMED_VALUE_FLOAT ");
                        	break;
                    	    case MAVLINK_MSG_ID_NAMED_VALUE_INT:
				fprintf(stderr, "NAMED_VALUE_INT ");
                        	break;
                    	    case MAVLINK_MSG_ID_PARAM_VALUE:
				fprintf(stderr, "PARAM_VALUE ");
                        	break;
                    	    case MAVLINK_MSG_ID_PARAM_SET:
				fprintf(stderr, "PARAM_SET ");
                        	break;
                    	    case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
				fprintf(stderr, "PARAM_REQUEST_READ ");
                        	break;
                    	    case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
				fprintf(stderr, "PARAM_REQUEST_LIST ");
                        	break;
                    	    case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:
				fprintf(stderr, "RC_CHANNELS_OVERRIDE ");
                        	break;
                    	    case MAVLINK_MSG_ID_RC_CHANNELS:
				fprintf(stderr, "RC_CHANNELS ");
                        	break;
                    	    case MAVLINK_MSG_ID_MANUAL_CONTROL:
				fprintf(stderr, "MANUAL_CONTROL ");
                        	break;
                    	    case MAVLINK_MSG_ID_COMMAND_LONG:
				fprintf(stderr, "COMMAND_LONG:%d ",mavlink_msg_command_long_get_command(&msg));
                        	break;
                    	    case MAVLINK_MSG_ID_STATUSTEXT:
				fprintf(stderr, "STATUSTEXT: severity:%d ",mavlink_msg_statustext_get_severity(&msg));
                        	break;
                    	    case MAVLINK_MSG_ID_SYSTEM_TIME:
				fprintf(stderr, "SYSTEM_TIME ");
                        	break;
                    	    case MAVLINK_MSG_ID_PING:
				fprintf(stderr, "PING ");
                        	break;
                    	    case MAVLINK_MSG_ID_CHANGE_OPERATOR_CONTROL:
				fprintf(stderr, "CHANGE_OPERATOR_CONTROL ");
                        	break;
                    	    case MAVLINK_MSG_ID_CHANGE_OPERATOR_CONTROL_ACK:
				fprintf(stderr, "CHANGE_OPERATOR_CONTROL_ACK ");
                        	break;
                    	    case MAVLINK_MSG_ID_MISSION_WRITE_PARTIAL_LIST:
				fprintf(stderr, "MISSION_WRITE_PARTIAL_LIST ");
                        	break;
                    	    case MAVLINK_MSG_ID_MISSION_ITEM:
				fprintf(stderr, "MISSION_ITEM ");
                        	break;
                    	    case MAVLINK_MSG_ID_MISSION_REQUEST:
				fprintf(stderr, "MISSION_REQUEST ");
                        	break;
                    	    case MAVLINK_MSG_ID_MISSION_SET_CURRENT:
				fprintf(stderr, "MISSION_SET_CURRENT ");
				break;
                    	    case MAVLINK_MSG_ID_MISSION_CURRENT:
				fprintf(stderr, "MISSION_CURRENT ");
                        	break;
                    	    case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:
				fprintf(stderr, "MISSION_REQUEST_LIST ");
                        	break;
                    	    case MAVLINK_MSG_ID_MISSION_COUNT:
				fprintf(stderr, "MISSION_COUNT ");
                        	break;
                    	    case MAVLINK_MSG_ID_MISSION_CLEAR_ALL:
				fprintf(stderr, "MISSION_CLEAR_ALL ");
                        	break;
                    	    case MAVLINK_MSG_ID_MISSION_ACK:
				fprintf(stderr, "MISSION_ACK ");
                        	break;
                    	    case MAVLINK_MSG_ID_MISSION_ITEM_INT:
				fprintf(stderr, "MISSION_ITEM_INT ");
                        	break;
                    	    case MAVLINK_MSG_ID_MISSION_REQUEST_INT:
				fprintf(stderr, "MISSION_REQUEST_INT ");
                        	break;
                    	    case MAVLINK_MSG_ID_SET_MODE:
				fprintf(stderr, "SET_MODE ");
                        	break;
                    	    case MAVLINK_MSG_ID_REQUEST_DATA_STREAM:
				fprintf(stderr, "REQUEST_DATA_STREAM ");
                        	break;
                    	    case MAVLINK_MSG_ID_DATA_STREAM:
				fprintf(stderr, "DATA_STREAM ");
                        	break;
                	    default:
                    		fprintf(stderr, "OTHER MESSAGE ID:%d ",msg.msgid);
                    		break;
			}
			fprintf(stderr, "\n");
		    }
		    len_msg = mavlink_msg_to_send_buffer(mavlink_message, &msg);
		    if (param_retransmissions == 1) {
			sendpacket(seqno, len_msg, &td, param_transmission_mode, num_interfaces, mavlink_message);
		    } else { // send twice
			sendpacket(seqno, len_msg, &td, param_transmission_mode, num_interfaces, mavlink_message);
			usleep(500); // wait 0.5ms to increase probability of 2nd packet coming through
			sendpacket(seqno, len_msg, &td, param_transmission_mode, num_interfaces, mavlink_message);
		    }
		    pcnt++;
		    seqno++;
		}
	    }
	} else { // generic telemetry handling
	        if (param_retransmissions == 1) {
		    sendpacket(seqno, inl, &td, param_transmission_mode, num_interfaces, buf);
		} else { // send twice
		    sendpacket(seqno, inl, &td, param_transmission_mode, num_interfaces, buf);
		    usleep(500); // wait 0.5ms to increase probability of 2nd packet coming through
		    sendpacket(seqno, inl, &td, param_transmission_mode, num_interfaces, buf);
		}
		pcnt++;
		seqno++;
	}

	if(pcnt % 128 == 0) {
	    printf("\t\t%d packets sent\r", pcnt);
	    fflush(stdout);
	}
    }

    printf("TX_TELEMETRY ERROR: Broken socket!\n");
    return (0);
}
