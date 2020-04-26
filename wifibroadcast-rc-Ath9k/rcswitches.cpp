#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "openhdlib.h"



int main() {
    int done = 1;
    uint16_t *rcdata = 0; //
    void *retval;

    int fd = shm_open("/wifibroadcast_rc_channels", O_RDWR, S_IRUSR | S_IWUSR);
    if (fd >= 0) {
        if (ftruncate(fd, 9 * sizeof(uint16_t)) != -1) {
            retval = mmap(NULL, 9 * sizeof(uint16_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (retval != MAP_FAILED) {
                rcdata = (uint16_t *)retval;
            }
        }
    }

    if (rcdata == 0) {
        //Error
        printf("-1\n");
    } else {
        printf("%d\n", rcdata[8]);
    }
}