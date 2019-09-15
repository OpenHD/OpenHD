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
int type[5];

bool no_signal, no_signal_rc;

struct framedata_n {
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

    uint8_t mac1_1; // Port
	
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

struct framedata_n framedatan;	

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

struct framedata_s framedatas;

struct framedata_l {
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

struct framedata_l framedatal;	

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


void sendRSSI(int num_int, telemetry_data_t *td) {
	int i;
	for(i=0; i<num_int; ++i) {		
	switch (type[i]) {
	case 0: // type: Ralink
	    if(td->rx_status != NULL) {
		long double a[4], b[4];
		FILE *fp;

		int best_dbm = -127;
		int best_dbm_rc = -127;
		int cardcounter = 0;
		int number_cards_rc = td->rx_status_rc->wifi_adapter_cnt;

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
		    framedatal.signal = best_dbm;
		} else {
		    framedatal.signal = -127;
		}
		if (no_signal_rc == false) {
		    framedatal.signal_rc = best_dbm_rc;
		} else {
		    framedatal.signal_rc = -127;
		}
		framedatal.lostpackets = td->rx_status->lost_packet_cnt;
		framedatal.lostpackets_rc = td->rx_status_rc->lost_packet_cnt;

		framedatal.injected_block_cnt = td->tx_status->injected_block_cnt;
		framedatal.skipped_fec_cnt = td->tx_status->skipped_fec_cnt;
		framedatal.injection_fail_cnt = td->tx_status->injection_fail_cnt;
		framedatal.injection_time_block = td->tx_status->injection_time_block;

    		fp = fopen("/proc/stat","r");
    		fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
    		fclose(fp);
    		usleep(333333); // send about 3 times per second
    		fp = fopen("/proc/stat","r");
    		fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&b[0],&b[1],&b[2],&b[3]);
    		fclose(fp);
		framedatal.cpuload = (((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]))) * 100;

		fp = fopen("/sys/class/thermal/thermal_zone0/temp","r");
		int temp = 0;
		fscanf(fp,"%d",&temp);
		fclose(fp);
		//fprintf(stderr,"temp: %d\n",temp/1000);
		framedatal.temp = temp / 1000;
	}
	printf("signal_rc: %d\n", framedatal.signal_rc);
	if (write(socks[i], &framedatal, 74) < 0 ) fprintf(stderr, "!");
	usleep(1500);
	if (write(socks[i], &framedatal, 74) < 0 ) fprintf(stderr, "!");
	usleep(2000);
	if (write(socks[i], &framedatal, 74) < 0 ) fprintf(stderr, "!");	
	break;
	case 1: // type: atheros
		if(td->rx_status != NULL) {
		long double a[4], b[4];
		FILE *fp;

		int best_dbm = -127;
		int best_dbm_rc = -127;
		int cardcounter = 0;
		int number_cards_rc = td->rx_status_rc->wifi_adapter_cnt;

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
		    framedatas.signal = best_dbm;
		} else {
		    framedatas.signal = -127;
		}
		if (no_signal_rc == false) {
		    framedatas.signal_rc = best_dbm_rc;
		} else {
		    framedatas.signal_rc = -127;
		}
		framedatas.lostpackets = td->rx_status->lost_packet_cnt;
		framedatas.lostpackets_rc = td->rx_status_rc->lost_packet_cnt;

		framedatas.injected_block_cnt = td->tx_status->injected_block_cnt;
		framedatas.skipped_fec_cnt = td->tx_status->skipped_fec_cnt;
		framedatas.injection_fail_cnt = td->tx_status->injection_fail_cnt;
		framedatas.injection_time_block = td->tx_status->injection_time_block;

    		fp = fopen("/proc/stat","r");
    		fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
    		fclose(fp);
    		usleep(333333); // send about 3 times per second
    		fp = fopen("/proc/stat","r");
    		fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&b[0],&b[1],&b[2],&b[3]);
    		fclose(fp);
		framedatas.cpuload = (((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]))) * 100;

		fp = fopen("/sys/class/thermal/thermal_zone0/temp","r");
		int temp = 0;
		fscanf(fp,"%d",&temp);
		fclose(fp);
		//fprintf(stderr,"temp: %d\n",temp/1000);
		framedatas.temp = temp / 1000;
	}
	printf("signal_rc: %d\n", framedatas.signal_rc);
	if (write(socks[i], &framedatas, 74) < 0 ) fprintf(stderr, "!");
	usleep(1500);
	if (write(socks[i], &framedatas, 74) < 0 ) fprintf(stderr, "!");
	usleep(2000);
	if (write(socks[i], &framedatas, 74) < 0 ) fprintf(stderr, "!");
	break;
	case 2: // type: Realtek
	    if(td->rx_status != NULL) {
		long double a[4], b[4];
		FILE *fp;

		int best_dbm = -127;
		int best_dbm_rc = -127;
		int cardcounter = 0;
		int number_cards_rc = td->rx_status_rc->wifi_adapter_cnt;

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
		    framedatan.signal = best_dbm;
		} else {
		    framedatan.signal = -127;
		}
		if (no_signal_rc == false) {
		    framedatan.signal_rc = best_dbm_rc;
		} else {
		    framedatan.signal_rc = -127;
		}
		framedatan.lostpackets = td->rx_status->lost_packet_cnt;
		framedatan.lostpackets_rc = td->rx_status_rc->lost_packet_cnt;

		framedatan.injected_block_cnt = td->tx_status->injected_block_cnt;
		framedatan.skipped_fec_cnt = td->tx_status->skipped_fec_cnt;
		framedatan.injection_fail_cnt = td->tx_status->injection_fail_cnt;
		framedatan.injection_time_block = td->tx_status->injection_time_block;

    		fp = fopen("/proc/stat","r");
    		fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
    		fclose(fp);
    		usleep(333333); // send about 3 times per second
    		fp = fopen("/proc/stat","r");
    		fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&b[0],&b[1],&b[2],&b[3]);
    		fclose(fp);
		framedatan.cpuload = (((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]))) * 100;

		fp = fopen("/sys/class/thermal/thermal_zone0/temp","r");
		int temp = 0;
		fscanf(fp,"%d",&temp);
		fclose(fp);
		//fprintf(stderr,"temp: %d\n",temp/1000);
		framedatan.temp = temp / 1000;
	}
	printf("signal_rc: %d\n", framedatan.signal_rc);
	if (write(socks[i], &framedatan, 74) < 0 ) fprintf(stderr, "!");
	usleep(1500);
	if (write(socks[i], &framedatan, 74) < 0 ) fprintf(stderr, "!");
	usleep(2000);
	if (write(socks[i], &framedatan, 74) < 0 ) fprintf(stderr, "!");	
	break;
	default:
    fprintf(stderr, "ERROR: Wrong or no frame type specified (see -t parameter)\n");
	exit(1);
	break;
  }
  }
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
			  fprintf(stderr, "rssitx: Atheros card detected\n");
	          type[num_interfaces] = 1;
		} else {
			  fprintf(stderr, "rssitx: Realtek card detected\n");
			  type[num_interfaces] = 2;
		}
    } else { // ralink or mediatek
              fprintf(stderr, "rssitx: Ralink card detected\n");
	          type[num_interfaces] = 0;
    }
	socks[num_interfaces] = open_sock(argv[x]);
        ++num_interfaces;
	++x;
        fclose(procfile);
        usleep(10000); // wait a bit between configuring interfaces to reduce Atheros and Pi USB flakiness
    }
	//	fprintf(stderr,"RSSI TX started\n");
	//socks[0] = open_sock(argv[3]);

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

	    framedatan.fc1 = 180; // <-- frame control field (0x08 = 8 = data frame) (0xb4 = 180 = rts frame)
	    framedatan.fc2 = 1; // <-- frame control field 0x02，0x01，0xbf
	    framedatan.dur1 = 0; // <-- duration
	    framedatan.dur2 = 0; // <-- duration

	    framedatan.mac1_1 = 255 ; // port 127
		
		framedatan.signal = 0;
	    framedatan.lostpackets = 0;
	    framedatan.signal_rc = 0;
	    framedatan.lostpackets_rc = 0;
	    framedatan.cpuload = 0;
	    framedatan.temp = 0;
	    framedatan.injected_block_cnt = 0;
	    framedatan.skipped_fec_cnt = 0;
	    framedatan.injection_fail_cnt = 0;
	    framedatan.injection_time_block = 0;

	    framedatan.bitrate_kbit = bitrate_kbit;
	    framedatan.bitrate_measured_kbit = bitrate_measured_kbit;
	    framedatan.cts = cts;
	    framedatan.undervolt = undervolt;

		
		
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

	    framedatas.fc1 = 8; // <-- frame control field 0x08 = 8 data frame (180 = rts frame)
	    framedatas.fc2 = 2; // <-- frame control field 0x02 = 2
	    framedatas.dur1 = 0; // <-- duration
	    framedatas.dur2 = 0; // <-- duration

	    framedatas.mac1_1 = 255 ; // port 127
	    framedatas.mac1_2 = 0;
	    framedatas.mac1_3 = 0;
	    framedatas.mac1_4 = 0;
	    framedatas.mac1_5 = 0;
	    framedatas.mac1_6 = 0;

	    framedatas.mac2_1 = 0;
	    framedatas.mac2_2 = 0;
	    framedatas.mac2_3 = 0;
	    framedatas.mac2_4 = 0;
	    framedatas.mac2_5 = 0;
	    framedatas.mac2_6 = 0;

	    framedatas.mac3_1 = 0;
	    framedatas.mac3_2 = 0;
	    framedatas.mac3_3 = 0;
	    framedatas.mac3_4 = 0;
	    framedatas.mac3_5 = 0;
	    framedatas.mac3_6 = 0;

	    framedatas.ieeeseq1 = 0;
	    framedatas.ieeeseq2 = 0;
		
		framedatas.signal = 0;
	    framedatas.lostpackets = 0;
	    framedatas.signal_rc = 0;
	    framedatas.lostpackets_rc = 0;
	    framedatas.cpuload = 0;
	    framedatas.temp = 0;
	    framedatas.injected_block_cnt = 0;
	    framedatas.skipped_fec_cnt = 0;
	    framedatas.injection_fail_cnt = 0;
	    framedatas.injection_time_block = 0;

	    framedatas.bitrate_kbit = bitrate_kbit;
	    framedatas.bitrate_measured_kbit = bitrate_measured_kbit;
	    framedatas.cts = cts;
	    framedatas.undervolt = undervolt;
		
  
  
	    framedatal.rt1 = 0; // <-- radiotap version
	    framedatal.rt2 = 0; // <-- radiotap version

	    framedatal.rt3 = 12; // <- radiotap header length
	    framedatal.rt4 = 0; // <- radiotap header length

	    framedatal.rt5 = 4; // <-- radiotap present flags
	    framedatal.rt6 = 128; // <-- radiotap present flags
	    framedatal.rt7 = 0; // <-- radiotap present flags
	    framedatal.rt8 = 0; // <-- radiotap present flags

	    framedatal.rt9 = 24; // <-- radiotap rate
	    framedatal.rt10 = 0; // <-- radiotap stuff
	    framedatal.rt11 = 0; // <-- radiotap stuff
	    framedatal.rt12 = 0; // <-- radiotap stuff

	    framedatal.fc1 = 8; // <-- frame control field 0x08 = 8 data frame (180 = rts frame)
	    framedatal.fc2 = 1; // <-- frame control field 0x01 = 1
	    framedatal.dur1 = 0; // <-- duration
	    framedatal.dur2 = 0; // <-- duration

	    framedatal.mac1_1 = 255 ; // port 127
		
		framedatal.signal = 0;
	    framedatal.lostpackets = 0;
	    framedatal.signal_rc = 0;
	    framedatal.lostpackets_rc = 0;
	    framedatal.cpuload = 0;
	    framedatal.temp = 0;
	    framedatal.injected_block_cnt = 0;
	    framedatal.skipped_fec_cnt = 0;
	    framedatal.injection_fail_cnt = 0;
	    framedatal.injection_time_block = 0;

	    framedatal.bitrate_kbit = bitrate_kbit;
	    framedatal.bitrate_measured_kbit = bitrate_measured_kbit;
	    framedatal.cts = cts;
	    framedatal.undervolt = undervolt;
		
	while (done) {
		sendRSSI(num_interfaces,&td);
	}
	return 0;
}
