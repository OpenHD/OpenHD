#include <stdio.h>

#include <fcntl.h>
#include <getopt.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netpacket/packet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include <openhd/mavlink.h>


#define SERVER "127.0.0.1"

//Max length of buffer
#define BUFLEN 2

//The port on which to send data
#define PORT 1257 



int main(int argc, char *argv[]) {
    char fBrokenSocket = 0;

    /*
     * Data read from stdin
     * 
     */
    uint8_t buf[402]; 

    mavlink_status_t status;
    mavlink_message_t msg;

    uint16_t chValue;


    int param_telemetry_protocol = 0;


    int ChannelToListen = atoi(argv[1]);

    /*
     * UDP init
     * 
     */
    struct sockaddr_in si_other;
    int s, i, slen = sizeof(si_other);
    char message[BUFLEN];


    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror(s);

        exit(1);
    }


    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);

    if (inet_aton(SERVER, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");

        exit(1);
    }
    // UDP init end



    while (!fBrokenSocket) {
        int inl = read(STDIN_FILENO, buf, 350); 


        if (inl < 0) {
            return 1;
        }
        
        
        if (inl > 350) {
            continue;
        }
        
        
        if (inl == 0) {
            usleep(1e5);

            continue;
        } // EOF


        if (inl > 0) {
            write(STDOUT_FILENO, &buf, inl);
        }


        if (param_telemetry_protocol == 0) {
            // parse Mavlink

            int i = 0;
            for (i = 0; i < inl; i++) {
                uint8_t c = buf[i];


                if (mavlink_parse_char(0, c, &msg, &status)) {
                    switch (msg.msgid) {
                        //printf( "mavlink id: %d \n", msg.msgid);

                        case MAVLINK_MSG_ID_RC_CHANNELS: {
                            //mavlink_rc_channels_t chinfo;
                            //mavlink_msg_rc_channels_decode(&msg, &chinfo);
                            //printf("MAVLINK_MSG_ID_RC_CHANNELS\n");

                            if (ChannelToListen >= 1 && ChannelToListen <= 18) {
                                //printf(" in range 1 - 18\n");

                                if (ChannelToListen == 1) {
                                    chValue = mavlink_msg_rc_channels_get_chan1_raw(&msg);
                                }

                                if (ChannelToListen == 2) {
                                    chValue = mavlink_msg_rc_channels_get_chan2_raw(&msg);
                                }

                                if (ChannelToListen == 3) {
                                    chValue = mavlink_msg_rc_channels_get_chan3_raw(&msg);
                                }

                                if (ChannelToListen == 4) {
                                    chValue = mavlink_msg_rc_channels_get_chan4_raw(&msg);
                                }

                                if (ChannelToListen == 5) {
                                    chValue = mavlink_msg_rc_channels_get_chan5_raw(&msg);
                                }

                                if (ChannelToListen == 6) {
                                    chValue = mavlink_msg_rc_channels_get_chan6_raw(&msg);
                                }

                                if (ChannelToListen == 7) {
                                    chValue = mavlink_msg_rc_channels_get_chan7_raw(&msg);
                                }

                                if (ChannelToListen == 8) {
                                    chValue = mavlink_msg_rc_channels_get_chan8_raw(&msg);
                                }

                                if (ChannelToListen == 9) {
                                    chValue = mavlink_msg_rc_channels_get_chan9_raw(&msg);
                                }

                                if (ChannelToListen == 10) {
                                    chValue = mavlink_msg_rc_channels_get_chan10_raw(&msg);
                                }

                                if (ChannelToListen == 11) {
                                    chValue = mavlink_msg_rc_channels_get_chan11_raw(&msg);
                                }

                                if (ChannelToListen == 12) {
                                    chValue = mavlink_msg_rc_channels_get_chan12_raw(&msg);
                                }

                                if (ChannelToListen == 13) {
                                    chValue = mavlink_msg_rc_channels_get_chan13_raw(&msg);
                                }

                                if (ChannelToListen == 14) {
                                    chValue = mavlink_msg_rc_channels_get_chan14_raw(&msg);
                                }

                                if (ChannelToListen == 15) {
                                    chValue = mavlink_msg_rc_channels_get_chan15_raw(&msg);
                                }

                                if (ChannelToListen == 16) {
                                    chValue = mavlink_msg_rc_channels_get_chan16_raw(&msg);
                                }

                                if (ChannelToListen == 17) {
                                    chValue = mavlink_msg_rc_channels_get_chan17_raw(&msg);
                                }

                                if (ChannelToListen == 18) {
                                    chValue = mavlink_msg_rc_channels_get_chan18_raw(&msg);
                                }

                                //int sizeinbyte = sizeof(ChannelToListen);

                                //unsigned int	under RPi2 = 2 byte

                                message[0] = chValue & 0xFF;
                                message[1] = chValue >> 8;

                                if (sendto(s, message, 2, 0, (struct sockaddr *)&si_other, slen) == -1) {
                                    //printf("sendto() error");
                                } else {
                                    //printf("sendto() - ok, chval: %d \n", chValue);
                                }
                            }

                            break;
                        }
                        default: {
                            break;
                        }
                    }
                }
            }
        }
    }


    close(s);


    return 0;
}
