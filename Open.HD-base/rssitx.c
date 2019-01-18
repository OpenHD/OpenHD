// rssitx by Rodizio (c) 2017. Licensed under GPL2
// reads rssi from shared mem and sends it out on wifi interfaces (for R/C and telemetry uplink RSSI)
#include <stdlib.h>
#include <stdio.h>
#include <sys/resource.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pcap.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <string.h>
#include <getopt.h>
#include "lib.h"
#include <inttypes.h>
#include <stdbool.h>

char *ifname = NULL;
int flagHelp = 0;

int sock=0;
int socks[5];

bool no_signal, no_signal_rc;

struct framedata_s {
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

    uint8_t mac1_1; // Port
    uint8_t mac1_2;
    uint8_t mac1_3;
    uint8_t mac1_4;
    uint8_t mac1_5;
    uint8_t mac1_6;

    uint8_t mac2_1;
    uint8_t mac2_2;
    uint8_t mac2_3;
    uint8_t mac2_4;
    uint8_t mac2_5;
    uint8_t mac2_6;

    uint8_t mac3_1;
    uint8_t mac3_2;
    uint8_t mac3_3;
    uint8_t mac3_4;
    uint8_t mac3_5;
    uint8_t mac3_6;

    uint8_t ieeeseq1;
    uint8_t ieeeseq2;

    int8_t signal;
    uint32_t lostpackets;
    int8_t signal_rc;
    uint32_t lostpackets_rc;
    uint8_t cpuload;
    uint8_t temp;
    uint32_t injected_block_cnt;
    uint32_t skipped_fec_cnt;
    uint32_t injection_fail_cnt;
    long long injection_time_block;
    uint16_t bitrate_kbit;
    uint16_t bitrate_measured_kbit;
    uint8_t cts;
    uint8_t undervolt;
}  __attribute__ ((__packed__));

struct framedata_s framedata;


void usage(void)
{
    printf(
        "rssitx by Rodizio.\n"
        "\n"
        "Usage: rssitx <interface>\n"
        "\n"
        "Example:\n"
        "  rssitx wlan0\n"
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


void sendRSSI(int sock, telemetry_data_t *td) {
	if(td->rx_status != NULL) {
		long double a[4], b[4];
		FILE *fp;

		int best_dbm = -127;
		int best_dbm_rc = -127;
		int cardcounter = 0;
		int number_cards = td->rx_status->wifi_adapter_cnt;
		int number_cards_rc = td->rx_status_rc->wifi_adapter_cnt;
//		printf("num_cards:%d num_cards_rc:%d\n", number_cards,number_cards_rc);

//		no_signal=true;
//                for(cardcounter=0; cardcounter<number_cards; ++cardcounter) {
//		    if (td->rx_status->adapter[cardcounter].signal_good == 1) { printf("card[%i] signal good\n",cardcounter); };
//		    printf("dbm[%i]: %d  \n",cardcounter, td->rx_status->adapter[cardcounter].current_signal_dbm);
//		    if (td->rx_status->adapter[cardcounter].signal_good == 1) {
//                	if (best_dbm < td->rx_status->adapter[cardcounter].current_signal_dbm) best_dbm = td->rx_status->adapter[cardcounter].current_signal_dbm;
//		    }
//		    if (td->rx_status->adapter[cardcounter].signal_good == 1) no_signal=false;
//                }

		no_signal_rc=true;
                for(cardcounter=0; cardcounter<number_cards_rc; ++cardcounter) {
		    if (td->rx_status_rc->adapter[cardcounter].signal_good == 1) { printf("card[%i] rc signal good\n",cardcounter); };
		    printf("dbm_rc[%i]: %d\n",cardcounter, td->rx_status_rc->adapter[cardcounter].current_signal_dbm);
		    if (td->rx_status_rc->adapter[cardcounter].signal_good == 1) {
                	if (best_dbm_rc < td->rx_status_rc->adapter[cardcounter].current_signal_dbm) best_dbm_rc = td->rx_status_rc->adapter[cardcounter].current_signal_dbm;
		    }
		    if (td->rx_status_rc->adapter[cardcounter].signal_good == 1) no_signal_rc=false;
                }

		if (no_signal_rc == false) { printf("rc signal good   "); };
		printf("best_dbm_rc:%d\n",best_dbm_rc);

		if (no_signal == false) {
		    framedata.signal = best_dbm;
		} else {
		    framedata.signal = -127;
		}
		if (no_signal_rc == false) {
		    framedata.signal_rc = best_dbm_rc;
		} else {
		    framedata.signal_rc = -127;
		}
		framedata.lostpackets = td->rx_status->lost_packet_cnt;
		framedata.lostpackets_rc = td->rx_status_rc->lost_packet_cnt;

		framedata.injected_block_cnt = td->tx_status->injected_block_cnt;
		framedata.skipped_fec_cnt = td->tx_status->skipped_fec_cnt;
		framedata.injection_fail_cnt = td->tx_status->injection_fail_cnt;
		framedata.injection_time_block = td->tx_status->injection_time_block;

    		fp = fopen("/proc/stat","r");
    		fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
    		fclose(fp);
    		usleep(333333); // send about 3 times per second
    		fp = fopen("/proc/stat","r");
    		fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&b[0],&b[1],&b[2],&b[3]);
    		fclose(fp);
		framedata.cpuload = (((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]))) * 100;

		fp = fopen("/sys/class/thermal/thermal_zone0/temp","r");
		int temp = 0;
		fscanf(fp,"%d",&temp);
		fclose(fp);
		//fprintf(stderr,"temp: %d\n",temp/1000);
		framedata.temp = temp / 1000;
	}

//	printf("signal: %d\n", framedata.signal);
//	printf("skipped fec %d\n", td->tx_status->skipped_fec_cnt);
//	printf("injection time: %lld\n", td->tx_status->injection_time_block);
//	printf("injection time: %lld\n", td->tx_status->injection_time_block);

//	fprintf(stdout,"\t\t%d blocks injected, injection time per block %lldus, %d fecs skipped, %d packet injections failed.          \r", td->tx_status->injected_block_cnt,td->tx_status->injection_time_block,td->tx_status->skipped_fec_cnt,td->tx_status->injection_fail_cnt);

	printf("signal_rc: %d\n", framedata.signal_rc);
//	printf("lostpackets: %d\n", framedata.lostpackets);
//	printf("lostpackets_rc: %d\n", framedata.lostpackets_rc);
	// send three times with different delay in between to increase robustness against packetloss
	if (write(socks[0], &framedata, 74) < 0 ) fprintf(stderr, "!");
	usleep(1500);
	if (write(socks[0], &framedata, 74) < 0 ) fprintf(stderr, "!");
	usleep(2000);
	if (write(socks[0], &framedata, 74) < 0 ) fprintf(stderr, "!");
}



wifibroadcast_rx_status_t *telemetry_wbc_status_memory_open(void) {
    int fd = 0;
    fd = shm_open("/wifibroadcast_rx_status_3", O_RDONLY, S_IRUSR | S_IWUSR);
    if(fd < 0) {
	fprintf(stderr, "RSSITX: ERROR: Could not open wifibroadcast rx uplink status!\n");
	exit(1);
    }
    void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t), PROT_READ, MAP_SHARED, fd, 0);
    if (retval == MAP_FAILED) { perror("mmap"); exit(1); }
    return (wifibroadcast_rx_status_t*)retval;
}

wifibroadcast_rx_status_t_rc *telemetry_wbc_status_memory_open_rc(void) {
    int fd = 0;
    fd = shm_open("/wifibroadcast_rx_status_rc", O_RDONLY, S_IRUSR | S_IWUSR);
    if(fd < 0) {
	fprintf(stderr, "RSSITX: ERROR: Could not open wifibroadcast R/C status!\n");
	exit(1);
    }
    void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t_rc), PROT_READ, MAP_SHARED, fd, 0);
    if (retval == MAP_FAILED) { perror("mmap"); exit(1); }
    return (wifibroadcast_rx_status_t_rc*)retval;
}

wifibroadcast_tx_status_t *telemetry_wbc_status_memory_open_tx(void) {
    int fd = 0;
    fd = shm_open("/wifibroadcast_tx_status_0", O_RDONLY, S_IRUSR | S_IWUSR);
    if(fd < 0) {
	fprintf(stderr, "RSSITX: ERROR: Could not open wifibroadcast tx status!\n");
	exit(1);
    }
    void *retval = mmap(NULL, sizeof(wifibroadcast_tx_status_t), PROT_READ, MAP_SHARED, fd, 0);
    if (retval == MAP_FAILED) { perror("mmap"); exit(1); }
    return (wifibroadcast_tx_status_t*)retval;
}

void telemetry_init(telemetry_data_t *td) {
	// init RSSI shared memory
	td->rx_status = telemetry_wbc_status_memory_open();
	td->rx_status_rc = telemetry_wbc_status_memory_open_rc();
	td->tx_status = telemetry_wbc_status_memory_open_tx();
}

int main (int argc, char *argv[]) {
	setpriority(PRIO_PROCESS, 0, 10);

	int done = 1;

//	fprintf(stderr,"RSSI TX started\n");
	socks[0] = open_sock(argv[1]);

	FILE * pFile;
	int bitrate_kbit;
	pFile = fopen ("/tmp/bitrate_kbit","r");
	if(NULL == pFile) {
    	    perror("ERROR: Could not open /tmp/bitrate_kbit");
    	    exit(EXIT_FAILURE);
	}
        fscanf(pFile, "%i\n", &bitrate_kbit);
//	printf("bitrate_kbit: %i\n", bitrate_kbit);
	fclose (pFile);

	int bitrate_measured_kbit;
	pFile = fopen ("/tmp/bitrate_measured_kbit","r");
	if(NULL == pFile) {
    	    perror("ERROR: Could not open /tmp/measured_kbit");
    	    exit(EXIT_FAILURE);
	}
	fscanf(pFile, "%i\n", &bitrate_measured_kbit);
//	printf("bitrate_measured_kbit: %i\n", bitrate_measured_kbit);
	fclose (pFile);

	int cts;
	pFile = fopen ("/tmp/cts","r");
	if(NULL == pFile) {
    	    perror("ERROR: Could not open /tmp/cts");
    	    exit(EXIT_FAILURE);
	}
	fscanf(pFile, "%i\n", &cts);
//	printf("cts: %i\n", cts);
	fclose (pFile);

	int undervolt;
	pFile = fopen ("/tmp/undervolt","r");
	if(NULL == pFile) {
    	    perror("ERROR: Could not open /tmp/undervolt");
    	    exit(EXIT_FAILURE);
	}
	fscanf(pFile, "%i\n", &undervolt);
//	printf("undervolt: %i\n", undervolt);
	fclose (pFile);

	telemetry_data_t td;
	telemetry_init(&td);

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

	framedata.fc1 = 8; // <-- frame control field 0x08 = 8 data frame (180 = rts frame)
	framedata.fc2 = 2; // <-- frame control field 0x02 = 2
	framedata.dur1 = 0; // <-- duration
	framedata.dur2 = 0; // <-- duration

	framedata.mac1_1 = 255 ; // port 127
	framedata.mac1_2 = 0;
	framedata.mac1_3 = 0;
	framedata.mac1_4 = 0;
	framedata.mac1_5 = 0;
	framedata.mac1_6 = 0;

	framedata.mac2_1 = 0;
	framedata.mac2_2 = 0;
	framedata.mac2_3 = 0;
	framedata.mac2_4 = 0;
	framedata.mac2_5 = 0;
	framedata.mac2_6 = 0;

	framedata.mac3_1 = 0;
	framedata.mac3_2 = 0;
	framedata.mac3_3 = 0;
	framedata.mac3_4 = 0;
	framedata.mac3_5 = 0;
	framedata.mac3_6 = 0;

	framedata.ieeeseq1 = 0;
	framedata.ieeeseq2 = 0;

	framedata.signal = 0;
	framedata.lostpackets = 0;
	framedata.signal_rc = 0;
	framedata.lostpackets_rc = 0;
	framedata.cpuload = 0;
	framedata.temp = 0;
	framedata.injected_block_cnt = 0;
	framedata.skipped_fec_cnt = 0;
	framedata.injection_fail_cnt = 0;
	framedata.injection_time_block = 0;

	framedata.bitrate_kbit = bitrate_kbit;
	framedata.bitrate_measured_kbit = bitrate_measured_kbit;
	framedata.cts = cts;
	framedata.undervolt = undervolt;

	while (done) {
		sendRSSI(sock,&td);
	}
	return 0;
}
