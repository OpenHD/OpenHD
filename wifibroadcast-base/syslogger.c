// syslogger.c (c) 2017 by Rodizio. Logger for system and tx injection stats Licensed under GPL2
// usage: ./syslogger <shared memory file>
// example: ./syslogger /wifibroadcast_rx_status_sysair
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

wifibroadcast_rx_status_t_sysair *status_memory_open_sysair(char* shm_file) {
	int fd;
	for(;;) {
		fd = shm_open(shm_file, O_RDWR, S_IRUSR | S_IWUSR);
		if(fd > 0) {
			break;
		}
//		fprintf(stderr,"syslogger: Waiting for rssirx to be started ...\n");
		usleep(3000000);
	}
	if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t_sysair)) == -1) {
		perror("ftruncate");
		exit(1);
	}
	void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t_sysair), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (retval == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}
	return (wifibroadcast_rx_status_t_sysair*)retval;
}

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    return milliseconds;
}

int main(int argc, char *argv[]) {
	wifibroadcast_rx_status_t_sysair *t = status_memory_open_sysair(argv[1]);

	int skipped_fec, skipped_fec_last, skipped_fec_per_second = 0;
	int injected_block, injected_block_last, injected_block_per_second = 0;
	int injection_fail, injection_fail_last, injection_fail_per_second = 0;

	float counter = 0;

	FILE *fp;
	FILE *fp2;

	int cpuload_gnd = 0;
	int temp_gnd = 0;
	long double a[4], b[4];

	for(;;) {
		// .csv format is:
		// counter, cpuload air, temp air, cpuload gnd, temp gnd, injection_time_block, skipped_fec_cnt/s, injected_block_cnt/s, injection_fail_cnt/s
		printf("%.1f,%d,%d,", counter,t->cpuload, t->temp);

		fp2 = fopen("/sys/class/thermal/thermal_zone0/temp","r");
		fscanf(fp2,"%d",&temp_gnd);
		fclose(fp2);
//		fprintf(stderr,"temp gnd:%d\n",temp_gnd/1000);
		fp = fopen("/proc/stat","r");
		fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
		fclose(fp);
		cpuload_gnd = (((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]))) * 100;
//		fprintf(stderr,"cpuload gnd:%d\n",cpuload_gnd);
		fp = fopen("/proc/stat","r");
		fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&b[0],&b[1],&b[2],&b[3]);
		fclose(fp);
		printf("%d,%d",cpuload_gnd,temp_gnd);

		printf("%lli,",t->injection_time_block);

                skipped_fec = t->skipped_fec_cnt;
		skipped_fec_per_second = (skipped_fec - skipped_fec_last);
		skipped_fec_last = t->skipped_fec_cnt;
                injected_block = t->injected_block_cnt;
		injected_block_per_second = (injected_block - injected_block_last);
		injected_block_last = t->injected_block_cnt;
                injection_fail = t->injection_fail_cnt;
		injection_fail_per_second = (injection_fail - injection_fail_last);
		injection_fail_last = t->injection_fail_cnt;
                printf("%d,%d,%d\n", skipped_fec_per_second,injected_block_per_second, injection_fail_per_second);

		fflush(stdout);
		usleep(1000000);
		counter = counter + 1;
	}
	return 0;
}
