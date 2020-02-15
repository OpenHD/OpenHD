#include "/tmp/rctx.h"

#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <string.h>
#define SWITCH_COUNT 16

//int main (int argc, char *argv[]) {
void main (void) {
    int done = 1;
	uint16_t *rcdata = 0; // 
	void *retval;
	
	int fd = shm_open("/wifibroadcast_rc_channels", O_RDWR, S_IRUSR | S_IWUSR);
	if(fd >= 0) {
		if (ftruncate(fd, 9 * sizeof(uint16_t)) != -1) {
			retval = mmap(NULL, 9 * sizeof(uint16_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
			if (retval != MAP_FAILED) {
				rcdata = (uint16_t *)retval;
			}
		}
	}
	if (rcdata == 0)	{	//Error
		printf("-1\n");
	} else {
		printf("%d\n",rcdata[8]);
	}
}