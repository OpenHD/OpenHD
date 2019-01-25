// channelscan by Rodizio. Quick&Dirty Channel scanner. GPL2 licensed
// does not work properly due to packet timing issues
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

wifibroadcast_rx_status_t *status_memory_open(void) {
	int fd;
	for(;;) {
		fd = shm_open("/wifibroadcast_rx_status_0", O_RDWR, S_IRUSR | S_IWUSR);
		if(fd > 0) { break; } usleep(200000); }
	if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t)) == -1) { perror("ftruncate"); exit(1); }
	void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (retval == MAP_FAILED) { perror("mmap"); exit(1); }
	return (wifibroadcast_rx_status_t*)retval;
}

int main(int argc, char *argv[]) {
    int band = atoi(argv[1]);
    char* nic = argv[2];
    int blocks1 = 0;
    int blocks2 = 0;
    int freq = 0;
    char cmd[50];

    wifibroadcast_rx_status_t *t = status_memory_open();

    if (band == 2324) {
	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2484", nic);
	system(cmd);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2484"); exit(0); }

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2477", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2477"); exit(0); }

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2482", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2482"); exit(0); }

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2477", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2477"); exit(0); }

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2487", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2487"); exit(0); }

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2489", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2489"); exit(0); }

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2492", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2492"); exit(0); }

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2494", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2494"); exit(0); }

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2497", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2497"); exit(0); }

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2499", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2499"); exit(0); }

	for (freq=2407; freq >= 2312; freq = freq - 5) {
	    snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq %d", nic, freq);
	    system(cmd);
	    usleep(20000);
	    blocks1 = t->received_packet_cnt;
	    usleep(30000);
	    blocks2 = t->received_packet_cnt;
	    if (blocks1 != blocks2) { printf("%d",freq); exit(0); }
	}

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2512", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2512"); exit(0); }

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2532", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2532"); exit(0); }

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2572", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2572"); exit(0); }

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2592", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2592"); exit(0); }

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2612", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2612"); exit(0); }

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2632", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2632"); exit(0); }

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2672", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2672"); exit(0); }

	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2692", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2692"); exit(0); }
    }

    if (band == 245) {
	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2484", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2484"); exit(0); }

	for (freq=2472; freq >= 2412; freq = freq - 5) {
	    snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq %d", nic, freq);
	    system(cmd);
	    usleep(20000);
	    blocks1 = t->received_packet_cnt;
	    usleep(30000);
	    blocks2 = t->received_packet_cnt;
	    if (blocks1 != blocks2) { printf("%d",freq); exit(0); }
	}

	for (freq=5825; freq >= 5745; freq = freq - 20) {
	    snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq %d", nic, freq);
	    system(cmd);
	    usleep(20000);
	    blocks1 = t->received_packet_cnt;
	    usleep(30000);
	    blocks2 = t->received_packet_cnt;
	    if (blocks1 != blocks2) {
		printf("%d",freq);
    		exit(0);
	    }
	}

	for (freq=5700; freq >= 5500; freq = freq - 20) {
	    snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq %d", nic, freq);
	    system(cmd);
	    usleep(20000);
	    blocks1 = t->received_packet_cnt;
	    usleep(30000);
	    blocks2 = t->received_packet_cnt;
	    if (blocks1 != blocks2) {
		printf("%d",freq);
		exit(0);
	    }
	}

	for (freq=5320; freq >= 5180; freq = freq - 20) {
	    snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq %d", nic, freq);
	    system(cmd);
	    usleep(20000);
	    blocks1 = t->received_packet_cnt;
	    usleep(30000);
	    blocks2 = t->received_packet_cnt;
	    if (blocks1 != blocks2) {
		printf("%d",freq);
		exit(0);
	    }
	}
    }

    if (band == 24) {
	snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq 2484", nic);
	system(cmd);
	usleep(20000);
	blocks1 = t->received_packet_cnt;
	usleep(30000);
	blocks2 = t->received_packet_cnt;
	if (blocks1 != blocks2) { printf("2484"); exit(0); }

	for (freq=2472; freq >= 2412; freq = freq - 5) {
	    snprintf(cmd, sizeof cmd, "/usr/sbin/iw dev %s set freq %d", nic, freq);
	    system(cmd);
	    usleep(20000);
	    blocks1 = t->received_packet_cnt;
	    usleep(30000);
	    blocks2 = t->received_packet_cnt;
	    if (blocks1 != blocks2) { printf("%d",freq); exit(0); }
	}
    }

    // if nothing found, return zero as channel
    printf("0");
    return 0;
}
