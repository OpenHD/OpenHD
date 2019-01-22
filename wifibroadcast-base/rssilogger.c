// rssilogger.c (c) 2017 by Rodizio. Logger for RSSI/packetloss. Licensed under GPL2
// usage: ./rssilogger <shared memory file>
// example: ./rssilogger /wifibroadcast_rx_status_0
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <resolv.h>
#include <string.h>
#include <utime.h>
#include <unistd.h>
#include <getopt.h>
#include <pcap.h>
#include <endian.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "lib.h"

wifibroadcast_rx_status_t *status_memory_open(char* shm_file) {
	int fd;
	for(;;) {
		fd = shm_open(shm_file, O_RDWR, S_IRUSR | S_IWUSR);
		if(fd > 0) {
			break;
		}
//		fprintf(stderr,"rssilogger: Waiting for rx to be started ...\n");
		usleep(3000000);
	}
	if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t)) == -1) {
		perror("ftruncate");
		exit(1);
	}
	void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (retval == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}
	return (wifibroadcast_rx_status_t*)retval;
}

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    return milliseconds;
}

int main(int argc, char *argv[]) {
	wifibroadcast_rx_status_t *t = status_memory_open(argv[1]);

	int packets_lost = 0;
	int packets_lost_last = 0;
	int packets_lost_per_second = 0;

	int badblocks = 0;
	int badblocks_last = 0;
	int badblocks_per_second = 0;

	int packets_received[6];
	packets_received[0]=0;
	packets_received[1]=0;
	packets_received[2]=0;
	packets_received[3]=0;
	packets_received[4]=0;
	packets_received[5]=0;

	int packets_received_last[6];
	packets_received_last[0]=0;
	packets_received_last[1]=0;
	packets_received_last[2]=0;
	packets_received_last[3]=0;
	packets_received_last[4]=0;
	packets_received_last[5]=0;

	int pps[6];
	pps[0]=0;
	pps[1]=0;
	pps[2]=0;
	pps[3]=0;
	pps[4]=0;
	pps[5]=0;

	float counter = 0;
	for(;;) {
		int i;
		// .csv format is:
		// counter, kbitrate, packets_lost_per_second, badblocks_per_second,adapter1_dbm,adapter1_pps,adapter2_dbm,adaoter2_pps, ...
		printf("%.1f,", counter);
		printf("%d,", t->kbitrate);
                packets_lost = t->lost_packet_cnt;
		packets_lost_per_second = (packets_lost - packets_lost_last) * 5;
		packets_lost_last = t->lost_packet_cnt;
                printf("%d,", packets_lost_per_second);
                badblocks = t->damaged_block_cnt;
		badblocks_per_second = (badblocks - badblocks_last) * 5;
		badblocks_last = t->damaged_block_cnt;
                printf("%d,", badblocks_per_second);
		for (i=0;i<5;i++) {
		    printf("%d,", t->adapter[i].current_signal_dbm);
                    packets_received[i] = t->adapter[i].received_packet_cnt;
		    pps[i] = (packets_received[i] - packets_received_last[i]) * 5;
		    packets_received_last[i] = t->adapter[i].received_packet_cnt;
                    printf("%d,", pps[i]);
		}
		printf("\n");
		fflush(stdout);
		usleep(200000);
		counter = counter + 0.2;
	}
	return 0;
}
