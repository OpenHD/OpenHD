#include <arpa/inet.h>
#include <stdio.h>	//printf
#include <stdlib.h> //exit(0);
#include <string.h> //memset
#include <sys/socket.h>

#include <getopt.h>
#include <sys/mman.h>

#include <fcntl.h> // serialport
#include <sys/resource.h>
#include <termios.h> // serialport

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pcap.h>
#include <resolv.h>
#include <sodium.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#include <openhd/mavlink.h>

#include "openhdlib.h"


/* 
 * Max length of buffer
 */
#define BUFLEN 21 

/* 
 * The port on which to listen for incoming data
 */
#define PORT 5565

int rc_received_yet = 0;
int serialport = 0;
int param_baudrate = 0;
int param_rc_protocol = 0;
char *param_serialport = "none";

int flagHelp = 0;
uint16_t sumdcrc = 0;
uint16_t ibuschecksum = 0;

int rssii;

wifibroadcast_rx_status_t_rc *rx_status_rc = NULL;



const uint16_t sumdcrc16_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, 0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0};


struct rcdata_s {
    unsigned int chan1 : 11;
    unsigned int chan2 : 11;
    unsigned int chan3 : 11;
    unsigned int chan4 : 11;
    unsigned int chan5 : 11;
    unsigned int chan6 : 11;
    unsigned int chan7 : 11;
    unsigned int chan8 : 11;
    unsigned int Is16 : 8;
    unsigned int switches : SWITCH_COUNT;
} __attribute__((__packed__));

struct rcdata_s rcdata;



void status_memory_init_rc(wifibroadcast_rx_status_t_rc *s) {
    s->received_block_cnt = 0;
    s->damaged_block_cnt = 0;
    s->received_packet_cnt = 0;
    s->lost_packet_cnt = 0;
    s->tx_restart_cnt = 0;
    s->wifi_adapter_cnt = 0;
    s->kbitrate = 0;

    int i;
    for (i = 0; i < 8; ++i) {
        s->adapter[i].received_packet_cnt = 0;
        s->adapter[i].wrong_crc_cnt = 0;
        s->adapter[i].current_signal_dbm = -126;
        s->adapter[i].signal_good = 0;
    }
}



wifibroadcast_rx_status_t_rc *status_memory_open_rc(void) {
    char buf[128];
    int fd;

    sprintf(buf, "/wifibroadcast_rx_status_rc");

    fd = shm_open(buf, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        perror("shm_open");
        exit(1);
    }

    void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t_rc), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (retval == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    wifibroadcast_rx_status_t_rc *tretval = (wifibroadcast_rx_status_t_rc *)retval;
    status_memory_init_rc(tretval);

    return tretval;
}



void usage(void) {
    printf("Usage: processUDP [options] \n"
           "-b <baudrate>   Serial port baudrate\n"
           "-s <serialport> Serial port to use\n"
           "-r <protocol>   R/C protocol to output. 0 = MSP. 1 = Mavlink. 2 = SUMD. 3 = IBUS. 4 = SRXL/XBUS. 99 = disable R/C\n"
           "-p <port>        for telemetry data. Default = 1\n"
           "\n"
           "Example:\n"
           "  processUDP -b 19200 -s /dev/serial0 -r 1 \n"
           "\n");
    exit(1);
}



void die(char *s) {
    perror(s);
    exit(1);
}



void initSerialPort() {
    if (param_baudrate != 0) {
        serialport = open(param_serialport, O_WRONLY | O_NOCTTY | O_NDELAY);

        if (serialport == -1) {
            /*
             * For some strange reason this doesn't work, although strace and the above fprintf shows -1
             */
            
            printf("RX_RC_TELEMETRY ERROR: Unable to open UART. Ensure it is not in use by another application\n");
        }

        switch (param_baudrate) {
            case 2400: {
                struct termios options;
                
                tcgetattr(serialport, &options);
                cfmakeraw(&options);
                cfsetospeed(&options, B2400);

                options.c_cflag &= ~CSIZE;
                options.c_cflag |= CS8;      // Set 8 data bits
                options.c_cflag &= ~PARENB;  // Set no parity
                options.c_cflag &= ~CSTOPB;  // 1 stop bit
                options.c_lflag &= ~ECHO;    // no echo
                options.c_cflag &= ~CRTSCTS; // no RTS/CTS Flow Control
                options.c_cflag |= CLOCAL;   // Set local mode on
                
                tcsetattr(serialport, TCSANOW, &options);
                
                printf("UART %s output set to %d baud\n", param_serialport, param_baudrate);
                break;
            }
            case 4800: {
                struct termios options;

                tcgetattr(serialport, &options);
                cfmakeraw(&options);
                cfsetospeed(&options, B4800);

                options.c_cflag &= ~CSIZE;
                options.c_cflag |= CS8;      // Set 8 data bits
                options.c_cflag &= ~PARENB;  // Set no parity
                options.c_cflag &= ~CSTOPB;  // 1 stop bit
                options.c_lflag &= ~ECHO;    // no echo
                options.c_cflag &= ~CRTSCTS; // no RTS/CTS Flow Control
                options.c_cflag |= CLOCAL;   // Set local mode on
                
                tcsetattr(serialport, TCSANOW, &options);

                printf("UART %s output set to %d baud\n", param_serialport, param_baudrate);
                break;
            }
            case 9600: {
                struct termios options;

                tcgetattr(serialport, &options);
                cfmakeraw(&options);
                cfsetospeed(&options, B9600);

                options.c_cflag &= ~CSIZE;
                options.c_cflag |= CS8;       // Set 8 data bits
                options.c_cflag &= ~PARENB;   // Set no parity
                options.c_cflag &= ~CSTOPB;   // 1 stop bit
                options.c_lflag &= ~ECHO;     // no echo
                options.c_cflag &= ~CRTSCTS;  // no RTS/CTS Flow Control
                options.c_cflag |= CLOCAL;    // Set local mode on

                tcsetattr(serialport, TCSANOW, &options);
                
                printf("UART %s output set to %d baud\n", param_serialport, param_baudrate);
                break;
            }
            case 19200: {
                struct termios options;

                tcgetattr(serialport, &options);
                cfmakeraw(&options);
                cfsetospeed(&options, B19200);
                
                options.c_cflag &= ~CSIZE;
                options.c_cflag |= CS8;      // Set 8 data bits
                options.c_cflag &= ~PARENB;  // Set no parity
                options.c_cflag &= ~CSTOPB;  // 1 stop bit
                options.c_lflag &= ~ECHO;    // no echo
                options.c_cflag &= ~CRTSCTS; // no RTS/CTS Flow Control
                options.c_cflag |= CLOCAL;   // Set local mode on
                
                tcsetattr(serialport, TCSANOW, &options);
                
                printf("UART %s output set to %d baud\n", param_serialport, param_baudrate);
                break;
            }
            case 38400: {
                struct termios options;

                tcgetattr(serialport, &options);
                cfmakeraw(&options);
                cfsetospeed(&options, B38400);
                
                options.c_cflag &= ~CSIZE;
                options.c_cflag |= CS8;      // Set 8 data bits
                options.c_cflag &= ~PARENB;  // Set no parity
                options.c_cflag &= ~CSTOPB;  // 1 stop bit
                options.c_lflag &= ~ECHO;    // no echo
                options.c_cflag &= ~CRTSCTS; // no RTS/CTS Flow Control
                options.c_cflag |= CLOCAL;   // Set local mode on
                
                tcsetattr(serialport, TCSANOW, &options);
                
                printf("UART %s output set to %d baud\n", param_serialport, param_baudrate);
                break;
            }
            case 57600: {
                struct termios options;

                tcgetattr(serialport, &options);
                cfmakeraw(&options);
                cfsetospeed(&options, B57600);
                
                options.c_cflag &= ~CSIZE;
                options.c_cflag |= CS8;       // Set 8 data bits
                options.c_cflag &= ~PARENB;   // Set no parity
                options.c_cflag &= ~CSTOPB;   // 1 stop bit
                options.c_lflag &= ~ECHO;     // no echo
                options.c_cflag &= ~CRTSCTS;  // no RTS/CTS Flow Control
                options.c_cflag |= CLOCAL;    // Set local mode on
                
                tcsetattr(serialport, TCSANOW, &options);
                
                printf("UART %s output set to %d baud\n", param_serialport, param_baudrate);
                break;
            }
            case 115200: {
                struct termios options;

                tcgetattr(serialport, &options);
                cfmakeraw(&options);
                cfsetospeed(&options, B115200);
                
                options.c_cflag &= ~CSIZE;
                options.c_cflag |= CS8;        // Set 8 data bits
                options.c_cflag &= ~PARENB;    // Set no parity
                options.c_cflag &= ~CSTOPB;    // 1 stop bit
                options.c_lflag &= ~ECHO;      // no echo
                options.c_cflag &= ~CRTSCTS;   // no RTS/CTS Flow Control
                options.c_cflag |= CLOCAL;     // Set local mode on
                
                tcsetattr(serialport, TCSANOW, &options);
                
                printf("UART %s output set to %d baud\n", param_serialport, param_baudrate);
                break;
            }
            default: {
                printf("ERROR: unsupported baudrate: %d\n", param_baudrate);

                exit(1);
            }
        }
    }
    else {
        printf("No baudrate param");
        exit(1);
    }
}


int main(int argc, char *argv[]) {
    rx_status_rc = status_memory_open_rc();
    rx_status_rc->wifi_adapter_cnt = 1;

    struct sockaddr_in si_me, si_other;
    int s, i, slen = sizeof(si_other), recv_len;
    
    uint8_t seqno_rc = 0;
    uint8_t seqnolast_rc = 0;
    
    char buf[BUFLEN];

    while (1) {
        int nOptionIndex;
        
        static const struct option optiona[] = {
            { "help", no_argument, &flagHelp, 1 },
            {      0,           0,         0, 0 }
        };
        
        int c = getopt_long(argc, 
                            argv,
                            "h:o:b:s:r:p:",
                            optiona,
                            &nOptionIndex);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 0: {
                // long option
                break;
            }
            case 'h': {
                usage();
            }
            case 'b': {
                param_baudrate = atoi(optarg);
                break;
            }
            case 's': {
                param_serialport = optarg;
                break;
            }
            case 'r': {
                param_rc_protocol = atoi(optarg);
                break;
            }
            default: {
                fprintf(stderr, "unknown switch %c\n", c);
                usage();
            }
        }
    }

    printf(" -b param_baudrate:%d\n", param_baudrate);
    printf(" -r param_rc_protocol:%d\n", param_rc_protocol);
    printf(" -s param_serialport:%s\n", param_serialport);


    /* 
     * Create a UDP socket
     */
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("socket");
    }

    /* 
     * Zero out the structure
     */
    memset((char *)&si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    /*
     * Bind socket to port
     */
    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me)) == -1) {
        die("bind");
    }

    initSerialPort();

    /*
     * Listen for new data
     */
    while (1) {
        //printf("Waiting for data...");
        //fflush(stdout);

        /* 
         * Try to receive some data, this is a blocking call
         */
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)) == -1) {
            die("recvfrom()");
        }

        //seqno_rc = (buf[17] << 8) | buf[16];

        rcdata.chan1 = (buf[1] << 8) | buf[0];
        rcdata.chan2 = (buf[3] << 8) | buf[2];
        rcdata.chan3 = (buf[5] << 8) | buf[4];
        rcdata.chan4 = (buf[7] << 8) | buf[6];
        rcdata.chan5 = (buf[9] << 8) | buf[8];
        rcdata.chan6 = (buf[11] << 8) | buf[10];
        rcdata.chan7 = (buf[13] << 8) | buf[12];
        rcdata.chan8 = (buf[15] << 8) | buf[14];

        rcdata.Is16 = buf[18];
        rcdata.switches = (buf[20] << 8) | buf[19];

        //printf ("rcdata1:%d\n",rcdata.chan1);
        //printf ("rcdata2:%d\n",rcdata.chan2);
        //printf ("rcdata3:%d\n",rcdata.chan3);
        //printf ("rcdata4:%d\n",rcdata.chan4);
        //printf ("rcdata5:%d\n",rcdata.chan5);
        //printf ("rcdata6:%d\n",rcdata.chan6);
        //printf ("rcdata7:%d\n",rcdata.chan7);
        //printf ("rcdata8:%d\n",rcdata.chan8);
        //printf ("seqno_rc:%d\n", seqno_rc);
        //printf ("is16 :%d\n", rcdata.Is16);
        //printf ("switches :%d\n", rcdata.switches);

        int len = 0;
        int lostpackets = 0;
        mavlink_message_t msg;
        uint8_t checksum = 0;
        uint8_t outputbuffer[100];

        seqnolast_rc = seqno_rc;

        seqno_rc = buf[16];

        rx_status_rc->adapter[0].received_packet_cnt++;
        rx_status_rc->last_update = time(NULL);

        if (seqno_rc == seqnolast_rc) {
            /* 
             * We already received that frame, do nothing
             */

            //fprintf(stderr,"seqno_rc = seqnolast_rc\n");
        } else {
            if ((seqno_rc - seqnolast_rc) != 1) {
                /*
                 * We've either lost packets, or the counter wrapped or packets received out of order
                 */

                if ((seqno_rc - seqnolast_rc) < 0) {
                    /* 
                     * Counter wrapped or out of order reception
                     */                    
                    lostpackets = seqno_rc - seqnolast_rc + 255;

                    //fprintf (stderr,"seqno_rc wrapped or out of order reception!\n");
                } else {
                    if (rc_received_yet == 0) {
                        /*
                         * If the tx is already running and rx has just started
                         * 
                         * We set the last seqno to the current to avoid wrong packetloss numbers
                         */
                        
                        seqnolast_rc = seqno_rc;
                        
                        rc_received_yet = 1;
                    } else {
                        lostpackets = seqno_rc - seqnolast_rc - 1;
                    }
                }
            }

            if (lostpackets > 0) {
                rx_status_rc->lost_packet_cnt = rx_status_rc->lost_packet_cnt + lostpackets;
                lostpackets = 0;
                
                fprintf(stderr, "rx_status_rc->lost_packet_cnt: %d\n", rx_status_rc->lost_packet_cnt);
            }

            switch (param_rc_protocol) {
                case 0: {
                    /* 
                     * MSP
                     */
                    checksum ^= 16;
                    checksum ^= 200;

                    // MSP header
                    outputbuffer[0] = '$';

                    // MSP header
                    outputbuffer[1] = 'M';

                    // MSP header (direction)
                    outputbuffer[2] = '<';

                    // Size
                    outputbuffer[3] = 16;

                    // Message type
                    outputbuffer[4] = 200; 

                    // Channel data
                    outputbuffer[5] = (uint8_t)(rcdata.chan1 & 0xFF);
                    checksum ^= outputbuffer[5];
                    outputbuffer[6] = (uint8_t)(rcdata.chan1 >> 8);
                    checksum ^= outputbuffer[6];
                    outputbuffer[7] = (uint8_t)(rcdata.chan2 & 0xFF);
                    checksum ^= outputbuffer[7];
                    outputbuffer[8] = (uint8_t)(rcdata.chan2 >> 8);
                    checksum ^= outputbuffer[8];
                    outputbuffer[9] = (uint8_t)(rcdata.chan3 & 0xFF);
                    checksum ^= outputbuffer[9];
                    outputbuffer[10] = (uint8_t)(rcdata.chan3 >> 8);
                    checksum ^= outputbuffer[10];
                    outputbuffer[11] = (uint8_t)(rcdata.chan4 & 0xFF);
                    checksum ^= outputbuffer[11];
                    outputbuffer[12] = (uint8_t)(rcdata.chan4 >> 8);
                    checksum ^= outputbuffer[12];
                    outputbuffer[13] = (uint8_t)(rcdata.chan5 & 0xFF);
                    checksum ^= outputbuffer[13];
                    outputbuffer[14] = (uint8_t)(rcdata.chan5 >> 8);
                    checksum ^= outputbuffer[14];
                    outputbuffer[15] = (uint8_t)(rcdata.chan6 & 0xFF);
                    checksum ^= outputbuffer[15];
                    outputbuffer[16] = (uint8_t)(rcdata.chan6 >> 8);
                    checksum ^= outputbuffer[16];
                    outputbuffer[17] = (uint8_t)(rcdata.chan7 & 0xFF);
                    checksum ^= outputbuffer[17];
                    outputbuffer[18] = (uint8_t)(rcdata.chan7 >> 8);
                    checksum ^= outputbuffer[18];
                    outputbuffer[19] = (uint8_t)(rcdata.chan8 & 0xFF);
                    checksum ^= outputbuffer[19];
                    outputbuffer[20] = (uint8_t)(rcdata.chan8 >> 8);
                    checksum ^= outputbuffer[20];

                    // checksum
                    outputbuffer[21] = checksum; 
                    
                    len = 22;

                    break;
                }
                case 1: {
                    /* 
                     * Mavlink
                     * 
                     * We send 8 real axis channels and 10 buttons to the flight controller 
                     * 
                     * The buttons are provided to the FC as 10 additional axis channels that only have 
                     * low/high values.
                     */
                    mavlink_msg_rc_channels_override_pack(255, 1, &msg, 1, 1,
                                                        rcdata.chan1, rcdata.chan2, rcdata.chan3, rcdata.chan4,
                                                        rcdata.chan5, rcdata.chan6, rcdata.chan7, rcdata.chan8,
                                                        (rcdata.switches & 1) ? 2000 : 1000, (rcdata.switches & 2) ? 2000 : 1000,
                                                        (rcdata.switches & 4) ? 2000 : 1000, (rcdata.switches & 8) ? 2000 : 1000,
                                                        (rcdata.switches & 16) ? 2000 : 1000, (rcdata.switches & 32) ? 2000 : 1000,
                                                        (rcdata.switches & 64) ? 2000 : 1000, (rcdata.switches & 128) ? 2000 : 1000,
                                                        (rcdata.switches & 256) ? 2000 : 1000, (rcdata.switches & 512) ? 2000 : 1000);

                    len = mavlink_msg_to_send_buffer(outputbuffer, &msg);
                    
                    break;
                }
                case 2: {
                    /*
                     * SUMD
                     */
                    sumdcrc = 0;
                    
                    // SUMD header Vendor_ID
                    outputbuffer[0] = 0xa8; 
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[0]];

                    // SUMD header Status
                    outputbuffer[1] = 0x01;
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[1]];

                    // SUMD header num channels (8)
                    outputbuffer[2] = 0x08;

                    // channel data
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[2]];
                    outputbuffer[3] = (uint8_t)((rcdata.chan1 * 8) >> 8);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[3]];
                    outputbuffer[4] = (uint8_t)((rcdata.chan1 * 8) & 0xFF);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[4]];
                    outputbuffer[5] = (uint8_t)((rcdata.chan2 * 8) >> 8);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[5]];
                    outputbuffer[6] = (uint8_t)((rcdata.chan2 * 8) & 0xFF);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[6]];
                    outputbuffer[7] = (uint8_t)((rcdata.chan3 * 8) >> 8);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[7]];
                    outputbuffer[8] = (uint8_t)((rcdata.chan3 * 8) & 0xFF);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[8]];
                    outputbuffer[9] = (uint8_t)((rcdata.chan4 * 8) >> 8);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[9]];
                    outputbuffer[10] = (uint8_t)((rcdata.chan4 * 8) & 0xFF);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[10]];
                    outputbuffer[11] = (uint8_t)((rcdata.chan5 * 8) >> 8);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[11]];
                    outputbuffer[12] = (uint8_t)((rcdata.chan5 * 8) & 0xFF);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[12]];
                    outputbuffer[13] = (uint8_t)((rcdata.chan6 * 8) >> 8);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[13]];
                    outputbuffer[14] = (uint8_t)((rcdata.chan6 * 8) & 0xFF);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[14]];
                    outputbuffer[15] = (uint8_t)((rcdata.chan7 * 8) >> 8);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[15]];
                    outputbuffer[16] = (uint8_t)((rcdata.chan7 * 8) & 0xFF);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[16]];
                    outputbuffer[17] = (uint8_t)((rcdata.chan8 * 8) >> 8);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[17]];
                    outputbuffer[18] = (uint8_t)((rcdata.chan8 * 8) & 0xFF);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[18]];

                    // crc16
                    outputbuffer[19] = (uint8_t)(sumdcrc >> 8);
                    outputbuffer[20] = (uint8_t)(sumdcrc & 0xFF);

                    len = 21;
                    break;
                }
                case 3: {
                    /*
                     * IBUS
                     */

                    ibuschecksum = 0xFFFF;

                    // Header
                    outputbuffer[0] = 0x20;
                    ibuschecksum -= outputbuffer[0];
                    outputbuffer[1] = 0x40;
                    ibuschecksum -= outputbuffer[1];

                    // Channel data
                    outputbuffer[2] = (uint8_t)(rcdata.chan1 & 0xFF);
                    ibuschecksum -= outputbuffer[2];
                    outputbuffer[3] = (uint8_t)(rcdata.chan1 >> 8);
                    ibuschecksum -= outputbuffer[3];
                    outputbuffer[4] = (uint8_t)(rcdata.chan2 & 0xFF);
                    ibuschecksum -= outputbuffer[4];
                    outputbuffer[5] = (uint8_t)(rcdata.chan2 >> 8);
                    ibuschecksum -= outputbuffer[5];
                    outputbuffer[6] = (uint8_t)(rcdata.chan3 & 0xFF);
                    ibuschecksum -= outputbuffer[6];
                    outputbuffer[7] = (uint8_t)(rcdata.chan3 >> 8);
                    ibuschecksum -= outputbuffer[7];
                    outputbuffer[8] = (uint8_t)(rcdata.chan4 & 0xFF);
                    ibuschecksum -= outputbuffer[8];
                    outputbuffer[9] = (uint8_t)(rcdata.chan4 >> 8);
                    ibuschecksum -= outputbuffer[9];
                    outputbuffer[10] = (uint8_t)(rcdata.chan5 & 0xFF);
                    ibuschecksum -= outputbuffer[10];
                    outputbuffer[11] = (uint8_t)(rcdata.chan5 >> 8);
                    ibuschecksum -= outputbuffer[11];
                    outputbuffer[12] = (uint8_t)(rcdata.chan6 & 0xFF);
                    ibuschecksum -= outputbuffer[12];
                    outputbuffer[13] = (uint8_t)(rcdata.chan6 >> 8);
                    ibuschecksum -= outputbuffer[13];
                    outputbuffer[14] = (uint8_t)(rcdata.chan7 & 0xFF);
                    ibuschecksum -= outputbuffer[14];
                    outputbuffer[15] = (uint8_t)(rcdata.chan7 >> 8);
                    ibuschecksum -= outputbuffer[15];
                    outputbuffer[16] = (uint8_t)(rcdata.chan8 & 0xFF);
                    ibuschecksum -= outputbuffer[16];
                    outputbuffer[17] = (uint8_t)(rcdata.chan8 >> 8);
                    ibuschecksum -= outputbuffer[17];

                    outputbuffer[18] = 0xdc;
                    ibuschecksum -= outputbuffer[18];
                    outputbuffer[19] = 0x05;
                    ibuschecksum -= outputbuffer[19];
                    outputbuffer[20] = 0xdc;
                    ibuschecksum -= outputbuffer[20];
                    outputbuffer[21] = 0x05;
                    ibuschecksum -= outputbuffer[21];
                    outputbuffer[22] = 0xdc;
                    ibuschecksum -= outputbuffer[22];
                    outputbuffer[23] = 0x05;
                    ibuschecksum -= outputbuffer[23];
                    outputbuffer[24] = 0xdc;
                    ibuschecksum -= outputbuffer[24];
                    outputbuffer[25] = 0x05;
                    ibuschecksum -= outputbuffer[25];
                    outputbuffer[26] = 0xdc;
                    ibuschecksum -= outputbuffer[26];
                    outputbuffer[27] = 0x05;
                    ibuschecksum -= outputbuffer[27];
                    outputbuffer[28] = 0xdc;
                    ibuschecksum -= outputbuffer[28];
                    outputbuffer[29] = 0x05;
                    ibuschecksum -= outputbuffer[29];

                    outputbuffer[30] = (uint8_t)(ibuschecksum & 0xFF);
                    outputbuffer[31] = (uint8_t)(ibuschecksum >> 8);

                    len = 32;
                    break;
                }
                case 4: {
                    /*
                     * Multiplex SRXL V1 / XBUS Mode B
                     * 
                     * Protocol uses the same checksum as sumd so we use the sumd variables and checksum functions
                     */

                    sumdcrc = 0;

                    // http://www.wolframalpha.com/input/?i=linear+fit+%7B800,+0%7D,+%7B1500,2048%7D,+%7B2200,+4095%7D
                    // 2.925 * rcdata.chanX - 2339.83 = SRXL 12bit channel format
                    rcdata.chan1 = (2.925 * rcdata.chan1) - 2339.83;
                    rcdata.chan2 = (2.925 * rcdata.chan2) - 2339.83;
                    rcdata.chan3 = (2.925 * rcdata.chan3) - 2339.83;
                    rcdata.chan4 = (2.925 * rcdata.chan4) - 2339.83;
                    rcdata.chan5 = (2.925 * rcdata.chan5) - 2339.83;
                    rcdata.chan6 = (2.925 * rcdata.chan6) - 2339.83;
                    rcdata.chan7 = (2.925 * rcdata.chan7) - 2339.83;
                    rcdata.chan8 = (2.925 * rcdata.chan8) - 2339.83;
                    
                    // Header
                    outputbuffer[0] = 0xa1;
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[0]];

                    // Channel data
                    outputbuffer[1] = (uint8_t)(rcdata.chan1 >> 8);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[1]];
                    outputbuffer[2] = (uint8_t)(rcdata.chan1 & 0xFF);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[2]];
                    outputbuffer[3] = (uint8_t)(rcdata.chan2 >> 8);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[3]];
                    outputbuffer[4] = (uint8_t)(rcdata.chan2 & 0xFF);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[4]];
                    outputbuffer[5] = (uint8_t)(rcdata.chan3 >> 8);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[5]];
                    outputbuffer[6] = (uint8_t)(rcdata.chan3 & 0xFF);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[6]];
                    outputbuffer[7] = (uint8_t)(rcdata.chan4 >> 8);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[7]];
                    outputbuffer[8] = (uint8_t)(rcdata.chan4 & 0xFF);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[8]];
                    outputbuffer[9] = (uint8_t)(rcdata.chan5 >> 8);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[9]];
                    outputbuffer[10] = (uint8_t)(rcdata.chan5 & 0xFF);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[10]];
                    outputbuffer[11] = (uint8_t)(rcdata.chan6 >> 8);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[11]];
                    outputbuffer[12] = (uint8_t)(rcdata.chan6 & 0xFF);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[12]];
                    outputbuffer[13] = (uint8_t)(rcdata.chan7 >> 8);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[13]];
                    outputbuffer[14] = (uint8_t)(rcdata.chan7 & 0xFF);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[14]];
                    outputbuffer[15] = (uint8_t)(rcdata.chan8 >> 8);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[15]];
                    outputbuffer[16] = (uint8_t)(rcdata.chan8 & 0xFF);
                    sumdcrc = (sumdcrc << 8) ^ sumdcrc16_table[(sumdcrc >> 8) ^ outputbuffer[16]];

                    outputbuffer[17] = 0x08;
                    ibuschecksum -= outputbuffer[17];
                    outputbuffer[18] = 0x00;
                    ibuschecksum -= outputbuffer[18];
                    outputbuffer[19] = 0x08;
                    ibuschecksum -= outputbuffer[19];
                    outputbuffer[20] = 0x00;
                    ibuschecksum -= outputbuffer[20];
                    outputbuffer[21] = 0x08;
                    ibuschecksum -= outputbuffer[21];
                    outputbuffer[22] = 0x00;
                    ibuschecksum -= outputbuffer[22];
                    outputbuffer[23] = 0x08;
                    ibuschecksum -= outputbuffer[23];
                    outputbuffer[24] = 0x00;
                    ibuschecksum -= outputbuffer[24];

                    // CRC16
                    outputbuffer[25] = (uint8_t)(sumdcrc >> 8);
                    outputbuffer[26] = (uint8_t)(sumdcrc & 0xFF);

                    len = 27;
                    break;
                }
            }

            if (param_baudrate != 0) {
                /*
                 * Only write to serialport if selected via commandline parameter
                 */

                write(serialport, outputbuffer, len);

                // write(STDOUT_FILENO, outputbuffer, len);
            }

            /* 
             * End of new packet recv
             */
        }
    }

    close(s);

    return 0;
}
