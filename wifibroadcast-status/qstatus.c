#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    uint8_t level;
    int message[1024];
} __attribute__((packed)) localmessage_t;


#define MESSAGE_FIFO "/tmp/messagefifo1"

int main(int argc, char **argv) {
    if (argc < 3) {
        exit(1);
    }

    int fifoFP = open(MESSAGE_FIFO, O_WRONLY);
    if (fifoFP == -1) {
        return 1;
    }

    int level = atoi(argv[2]);

    localmessage_t message;

    message.level = level;
    strncpy(message.message, argv[1], 1024);
    write(fifoFP, &message, sizeof(message));
    close(fifoFP);
    return 0;
}
