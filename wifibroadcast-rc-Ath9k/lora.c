#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <stdio.h>	// for printf()
#include <stdlib.h> // for exit(0);
#include <string.h> // for memset()
#include <sys/socket.h>

#include <pthread.h>



#define BUFLEN 21

//LoRa UDP in
#define PORT 5566



pthread_t hThread = 0;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
unsigned char RCValues[BUFLEN];
int IsNewDataToSend = 0;



int set_interface_attribs(int fd, int speed) {
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    //cfsetospeed(&tty, (speed_t)speed);
    //cfsetispeed(&tty, (speed_t)speed);


    /*
     * Ignore modem controls
     */
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;

    /* 
     * 8-bit characters
     * 
     */
    tty.c_cflag |= CS8;

    /* 
     * Only need 1 stop bit
     * 
     */
    tty.c_cflag |= CSTOPB;
    
    /* 
     * No hardware flowcontrol 
     * 
     */
    tty.c_cflag &= ~CRTSCTS;


    /* 
     * Setup for non-canonical mode 
     * 
     */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* 
     * Fetch bytes as they become available 
     * 
     */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;


    /* 
     * Commit new settings and check for errors
     * 
     */
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));

        return -1;
    }

    return 0;
}



void *StartUDPServer(void *arg) {
    struct sockaddr_in si_me, si_other;

    int s, i, slen = sizeof(si_other), recv_len;
    char buf[BUFLEN];

    /* 
     * Create a UDP socket 
     * 
     */
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        printf("socket error");
    }

    /* 
     * Zero out the structure 
     * 
     */
    memset((char *)&si_me, 0, sizeof(si_me));

    /* 
     * Set the socket type, port and bind address
     * 
     */
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 
     * Bind socket
     * 
     */
    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me)) == -1) {
        printf("bind error");
    }

    /* 
     * Listen for new data
     * 
     */
    while (1) {
        //printf("Waiting for data...");
        //fflush(stdout);

        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)) == -1) {
            printf("recvfrom()");
        } else {
            pthread_mutex_lock(&mutex1);

            memcpy(RCValues, buf, BUFLEN);
            IsNewDataToSend = 1;

            pthread_mutex_unlock(&mutex1);
        }
    }

    close(s);

    return NULL;
}


void RepackMessage(unsigned char *UDPRecv, unsigned char *SendBuffer) {
    // Convert two UDP char to int
    unsigned short RC_int[16];

    RC_int[0] = (UDPRecv[1] << 8) + UDPRecv[0];
    RC_int[1] = (UDPRecv[3] << 8) + UDPRecv[2];
    RC_int[2] = (UDPRecv[5] << 8) + UDPRecv[4];
    RC_int[3] = (UDPRecv[7] << 8) + UDPRecv[6];
    RC_int[4] = (UDPRecv[9] << 8) + UDPRecv[8];
    RC_int[5] = (UDPRecv[11] << 8) + UDPRecv[10];
    RC_int[6] = (UDPRecv[13] << 8) + UDPRecv[12];
    RC_int[7] = (UDPRecv[15] << 8) + UDPRecv[14];

    short DigitalChannels = (UDPRecv[20] << 8) + UDPRecv[19];


    //printf("DigitalChannels: %d \n", DigitalChannels);
    short byte = DigitalChannels;
    int i = 0;
    int RcNumber = 15;


    for (i = 7; i >= 0; i--) {
        if ((DigitalChannels >> i) & 1) {
            RC_int[RcNumber] = 1800;
        } else {
            RC_int[RcNumber] = 1000;
        }
        RcNumber--;
    }

    for (i = 0; i < 7; i++) {
        RC_int[i] = (16 * RC_int[i]) / 10 - 1398; //1408
    }

    /*
    printf("RC_int: 8 - %d, 9 - %d, 10 - %d, 11  - %d, 12 - %d, 13  - %d , 14  - %d, 15  - %d, 15  - %d \n",
            RC_int[7],
            RC_int[8],
            RC_int[9],
            RC_int[10],
            RC_int[11],
            RC_int[12],
            RC_int[13],
            RC_int[14],
            RC_int[15]);
    */
    
    //int size = sizeof(SendBuffer[1]);
    //printf("Sizeof char: %d \n", size);


    // Header
    SendBuffer[0] = 0x0F; 


    // Int to LoRa
    SendBuffer[1] = (unsigned char)((RC_int[0] & 0x07FF));
    SendBuffer[2] = (unsigned char)((RC_int[0] & 0x07FF) >> 8 | (RC_int[1] & 0x07FF) << 3);
    SendBuffer[3] = (unsigned char)((RC_int[1] & 0x07FF) >> 5 | (RC_int[2] & 0x07FF) << 6);
    SendBuffer[4] = (unsigned char)((RC_int[2] & 0x07FF) >> 2);
    SendBuffer[5] = (unsigned char)((RC_int[2] & 0x07FF) >> 10 | (RC_int[3] & 0x07FF) << 1);
    SendBuffer[6] = (unsigned char)((RC_int[3] & 0x07FF) >> 7 | (RC_int[4] & 0x07FF) << 4);
    SendBuffer[7] = (unsigned char)((RC_int[4] & 0x07FF) >> 4 | (RC_int[5] & 0x07FF) << 7);
    SendBuffer[8] = (unsigned char)((RC_int[5] & 0x07FF) >> 1);
    SendBuffer[9] = (unsigned char)((RC_int[5] & 0x07FF) >> 9 | (RC_int[6] & 0x07FF) << 2);
    SendBuffer[10] = (unsigned char)((RC_int[6] & 0x07FF) >> 6 | (RC_int[7] & 0x07FF) << 5);
    SendBuffer[11] = (unsigned char)((RC_int[7] & 0x07FF) >> 3);

    SendBuffer[12] = (unsigned char)((RC_int[8] & 0x07FF));
    SendBuffer[13] = (unsigned char)((RC_int[8] & 0x07FF) >> 8 | (RC_int[9] & 0x07FF) << 3);
    SendBuffer[14] = (unsigned char)((RC_int[9] & 0x07FF) >> 5 | (RC_int[10] & 0x07FF) << 6);
    SendBuffer[15] = (unsigned char)((RC_int[10] & 0x07FF) >> 2);
    SendBuffer[16] = (unsigned char)((RC_int[10] & 0x07FF) >> 10 | (RC_int[11] & 0x07FF) << 1);
    SendBuffer[17] = (unsigned char)((RC_int[11] & 0x07FF) >> 7 | (RC_int[12] & 0x07FF) << 4);
    SendBuffer[18] = (unsigned char)((RC_int[12] & 0x07FF) >> 4 | (RC_int[13] & 0x07FF) << 7);
    SendBuffer[19] = (unsigned char)((RC_int[13] & 0x07FF) >> 1);
    SendBuffer[20] = (unsigned char)((RC_int[13] & 0x07FF) >> 9 | (RC_int[14] & 0x07FF) << 2);
    SendBuffer[21] = (unsigned char)((RC_int[14] & 0x07FF) >> 6 | (RC_int[15] & 0x07FF) << 5);
    SendBuffer[22] = (unsigned char)((RC_int[15] & 0x07FF) >> 3);
    // Header end
    SendBuffer[23] = 0; 
    SendBuffer[24] = 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: ./lora /dev/ttyUSB0 \n");

        return -1;
    }


    int err = pthread_create(&hThread, NULL, &StartUDPServer, NULL);
    if (err != 0) {
        printf("\ncan't create thread :[%s]", strerror(err));
    } else {
        printf("\n UDP in thread created successfully\n");
    }


    int fd;
    fd = open(argv[1], O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        printf("Error opening: %s\n", strerror(errno));

        return -1;
    }


    /* 
     * Speed ignored by custom driver
     * 
     * ch341.c must be 100000
     * 
     */
    set_interface_attribs(fd, B115200); 


    unsigned char SendBuffer[25];

    //0F E5 03 1F F8 C0 07 3E F0 81 0F 7C E0 03 06 F8 80 91 3D F0 81 0F 7C 00 00
    //0F C0 00 05 30 80 01 0C 60 00 03 18 C0 00 06 30 80 01 0C 60 00 03 18 00 00

    
    int wlen;

    int done = 0;
    while (done != 1) {
        RepackMessage(&RCValues[0], &SendBuffer[0]);
        
        wlen = write(fd, &SendBuffer, 25);
        
        if (wlen != 25) {
            printf("Write to TTY - error. stop.");
            done = 1;
        }

        //IsNewDataToSend = 0;
        //for(int i=0;i<25;i++) {
        //    printf(" %x", SendBuffer[i] & 0xff);
        //}
        //printf(" end.\n");
        //printf("Error from write: %d, %d\n", wlen, errno);

        /*
         * Delay for output
         * 
         */
        tcdrain(fd);

        usleep(20);
    }
}
