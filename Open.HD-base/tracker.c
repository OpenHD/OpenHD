// tracker.c (c) 2017 by Rodizio. Very dirty code, just a quick proof-of-concept. Licensed under GPL2
// drives a stepper motor through a stepper motor driver connected to the GPIO pins
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

#include <wiringPi.h>

wifibroadcast_rx_status_t *status_memory_open(char* shm_file, char* line) {
	
	int fd;

	for(;;) {
		fd = shm_open(shm_file, O_RDWR, S_IRUSR | S_IWUSR);
		if(fd > 0) {
			break;
		}
		// Goto line/row 1/1
		printf("\033[%s;1H",line);
		printf("Waiting for rx to be started ...\n");
		usleep(100000);
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


void turnleft() {
	digitalWrite (23, LOW);
	usleep(2);
	digitalWrite (17, HIGH);

	usleep(3);
	digitalWrite (27, HIGH);
	usleep(3);
	digitalWrite (27, LOW);

//	usleep(10000);
//	digitalWrite (23, HIGH);
	usleep(3);
}

void turnleftbrake() {
	digitalWrite (23, LOW);
	usleep(2);
	digitalWrite (17, HIGH);

	usleep(3);
	digitalWrite (27, HIGH);
	usleep(3);
	digitalWrite (27, LOW);

	usleep(10000);
	//digitalWrite (23, HIGH);
	usleep(2);
}

void turnright() {
	digitalWrite (23, LOW);
	usleep(2);
	digitalWrite (17, LOW);

	usleep(3);
	digitalWrite (27, HIGH);
	usleep(3);
	digitalWrite (27, LOW);

//	usleep(10000);
//	digitalWrite (23, HIGH);
	usleep(3);
}

void turnrightbrake() {
	digitalWrite (23, LOW);
	usleep(2);
	digitalWrite (17, LOW);

	usleep(3);
	digitalWrite (27, HIGH);
	usleep(3);
	digitalWrite (27, LOW);

	usleep(10000);
//	digitalWrite (23, HIGH);
	usleep(2);
}


int main(int argc, char *argv[]) {

	wiringPiSetupGpio();
	pinMode(17, OUTPUT); // DIR
	pinMode(27, OUTPUT); // STEP

	pinMode(22, OUTPUT); // RESET?
	pinMode(4, OUTPUT); // SLEEP?

	pinMode(23, OUTPUT); // ENABLE

	// set reset and sleep to high to enable DRV
	digitalWrite (22, HIGH);
	usleep(2);
	digitalWrite (4, HIGH);
	usleep(2);

	// set enable to high to disable FETs
	digitalWrite (23, HIGH);
	usleep(3);


	wifibroadcast_rx_status_t *t = status_memory_open(argv[1],argv[2]);


	int left = 0;
	int right = 0;
	int center= 0;
	int diff = 0;
	int lastmove = 0;
	int move = 0;
	int samedircount = 0;
	int idlecount = 0;

	for(;;) {

		    left = t->adapter[2].current_signal_dbm;
		    right = t->adapter[1].current_signal_dbm;
		    center = t->adapter[0].current_signal_dbm;
	
		    lastmove = move;
		    move = 0;

		    diff = left - right;
		    printf("left: %d  center: %d  right:%d  diff:%d  samecnt:%d  ", left, center, right, diff, samedircount);

		    if (left < right) {
			printf("left < right  ");
		    }

		    if ((left > center) && (center > right)) {
			printf("left better center better right -  ");
			//turnleft();
			move = 1;
		    }

		    if ((right > center) && (center > left)) {
			printf("right worse center worse left -  ");
			//turnright();
			move = 2;
		    }

		    if ((center < right) || (center < left)) { // in case left and right is the same but center is worse
			printf("center worse right or left  ");
			
			// check which is better
			if (right > left) { // if right signal is better
			    printf("rightbetter - ");
			    //turnright();
			    move = 2;
			}

			if (left > right) { // if left signal is better
			    printf("leftbetter -  ");
			    //turnleft();
			    move = 1;
			}

			if (left == right) { // if left and right are the same
			    printf("left/right the same -  ");
			    //turnleft();
			    move = 0;
			}

		    }

		    if ((diff < -2) || (diff > 2)) {
			printf("diff over 2  ");
			if (diff < -2) {
			    //turnright();
			    move = 2;
			    printf("rightbetter -  ");
			} else {
			    //turnleft();
			    move = 1;
			    printf("lefttbetter -  ");
			}
		    }


		    if ((move != 0) && ((diff < -2) || (diff > 2))) {

			idlecount = 0;

			if ((move == 1) && (lastmove != 2)) {
			    if (samedircount < 5) {
				printf(" turn left");
				turnleft();
			    } else {
				printf(" turn left samedir");
				turnleft();
			    }
			    samedircount++;
			}

			if ((move == 2) && (lastmove != 1)) {
			    if (samedircount < 5) {
				printf(" turn right");
				turnright();
			    } else {
				printf(" turn right samedir");
				turnright();
			    }
			    samedircount++;
			}

			if ((move == 1) && (lastmove == 2)) {
			    printf(" doing nothing, last dir was different");
			    samedircount = 0;
			}

			if ((move == 2) && (lastmove == 1)) {
			    printf(" doing nothing, last dir was different");
			    samedircount = 0;
			}
		    } else {
			    idlecount++;

			    if (idlecount > 10) {
				printf("idlecount > 10 switching FETs off");
				digitalWrite (23, HIGH);
			    }
			    samedircount = 0;
		    }


		    printf("\n");
		    usleep(15000);

	}



	return 0;
}
