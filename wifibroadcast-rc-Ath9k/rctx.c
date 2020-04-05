// rctx by Rodizio
// Based on JS2Serial by Oliver Mueller and wbc all-in-one tx by Anemostec.
// Thanks to dino_de for the Joystick switches and mavlink code
// Licensed under GPL2


#include "openhdlib.h"
#include <SDL/SDL.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netpacket/packet.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <termios.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#include "/tmp/rctx.h"


// read Joystick every 2 ms or 500x per second
#define UPDATE_INTERVAL 2000

// check if joystick disconnected every 400th time or 200ms or 5x per second
#define JOY_CHECK_NTH_TIME 400 

#define JOYSTICK_N 0

#define JOY_DEV "/sys/class/input/js0"

#define SERVER "127.0.0.1"

#define BUFLEN 2

// Encrypted RC out via SVPCom
#define PORT 5566 

// BandSwitch py script in
#define PORT2 1258

// IP or USB camera switch py script in
#define PORT3 1259 


// Encrypted RC Message
char messageRCEncrypt[40]; 

wifibroadcast_rx_status_t *telemetry_wbc_status_memory_open(void);

int NICCount = 0;

int TrimChannel = 0;
int Action = 0;
int PWMCount = 0;
int ActivateChannel = 0;
int IsTrimDone[8] = {0};


static uint16_t *rcData = NULL;
static uint16_t lastValidCh0 = AXIS0_INITIAL;
static uint16_t lastValidCh1 = AXIS1_INITIAL;
static uint16_t lastValidCh2 = AXIS2_INITIAL;
static uint16_t lastValidCh3 = AXIS3_INITIAL;
static uint16_t lastValidCh4 = AXIS4_INITIAL;
static uint16_t lastValidCh5 = AXIS5_INITIAL;
static uint16_t lastValidCh6 = AXIS6_INITIAL;
static uint16_t lastValidCh7 = AXIS7_INITIAL;

static uint16_t validButton1 = 0;
static uint16_t validButton2 = 0;
static uint16_t validButton3 = 0;
static uint16_t validButton4 = 0;
static uint16_t validButton5 = 0;
bool validButtons = false;
int discardCounter = 0;


static SDL_Joystick *js;
char *ifname = NULL;
int flagHelp = 0;
int sock = 0;
int socks[5];
int type[5];



struct framedata_s {
    // 88 bits of data (11 bits per channel * 8 channels) = 11 bytes
    uint8_t rt1;
    uint8_t rt2;
    uint8_t rt3;
    uint8_t rt4;
    uint8_t rt5;
    uint8_t rt6;
    uint8_t rt7;
    uint8_t rt8;

    uint8_t rt9;
    uint8_t rt10;
    uint8_t rt11;
    uint8_t rt12;

    uint8_t fc1;
    uint8_t fc2;
    uint8_t dur1;
    uint8_t dur2;

    uint8_t seqnumber;

    unsigned int chan1 : 11;
    unsigned int chan2 : 11;
    unsigned int chan3 : 11;
    unsigned int chan4 : 11;
    unsigned int chan5 : 11;
    unsigned int chan6 : 11;
    unsigned int chan7 : 11;
    unsigned int chan8 : 11;
    // 16 bits for rc channels 9 - 24 as switches
    unsigned int switches : SWITCH_COUNT; 
} __attribute__((__packed__));

struct framedata_s framedatas;


struct framedata_n {
    // 88 bits of data (11 bits per channel * 8 channels) = 11 bytes
    uint8_t rt1;
    uint8_t rt2;
    uint8_t rt3;
    uint8_t rt4;
    uint8_t rt5;
    uint8_t rt6;
    uint8_t rt7;
    uint8_t rt8;

    uint8_t rt9;
    uint8_t rt10;
    uint8_t rt11;
    uint8_t rt12;
    uint8_t rt13;

    uint8_t fc1;
    uint8_t fc2;
    uint8_t dur1;
    uint8_t dur2;

    uint8_t seqnumber;

    unsigned int chan1 : 11;
    unsigned int chan2 : 11;
    unsigned int chan3 : 11;
    unsigned int chan4 : 11;
    unsigned int chan5 : 11;
    unsigned int chan6 : 11;
    unsigned int chan7 : 11;
    unsigned int chan8 : 11;
    // 16 bits for rc channels 9 - 24  as switches
    unsigned int switches : SWITCH_COUNT; 

} __attribute__((__packed__));

struct framedata_n framedatan;



uint16_t *rc_channels_memory_open(void) {
    int fd = shm_open("/wifibroadcast_rc_channels", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if (fd < 0) {
        fprintf(stderr, "rc shm_open\n");
        exit(1);
    }

    if (ftruncate(fd, 9 * sizeof(uint16_t)) == -1) {
        fprintf(stderr, "rc ftruncate\n");
        exit(1);
    }

    void *retval = mmap(NULL, 9 * sizeof(uint16_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (retval == MAP_FAILED) {
        fprintf(stderr, "rc mmap\n");
        exit(1);
    }

    return (uint16_t *)retval;
}


void usage(void) {
    printf("rctx by Rodizio. Based on JS2Serial by Oliver Mueller and wbc all-in-one tx by Anemostec. GPL2\n"
           "\n"
           "Usage: rctx rctx ChannelToListen2 ChannelIPCamera IsBandSwitcherEnabled(1\\0) IsIPCameraSwitcherEnabled(1\\0) IsEncrypt(1\\0) $TrimChannel $Action $PWMCount $ActivateChannel $PrimaryCardMAC \n"
           "\n"
           "Example:\n"
           "  rctx 2 3 1 1 0 wlan0\n"
           "\n");

    exit(1);
}


static int open_sock(char *ifname) {
    struct sockaddr_ll ll_addr;
    struct ifreq ifr;

    NICCount++;

    sock = socket(AF_PACKET, SOCK_RAW, 0);
    if (sock == -1) {
        fprintf(stderr, "Error:\tSocket failed\n");
        exit(1);
    }


    ll_addr.sll_family = AF_PACKET;
    ll_addr.sll_protocol = 0;
    ll_addr.sll_halen = ETH_ALEN;

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        fprintf(stderr, "Error:\tioctl(SIOCGIFINDEX) failed\n");
        exit(1);
    }


    ll_addr.sll_ifindex = ifr.ifr_ifindex;

    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
        fprintf(stderr, "Error:\tioctl(SIOCGIFHWADDR) failed\n");
        exit(1);
    }

    memcpy(ll_addr.sll_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

    if (bind(sock, (struct sockaddr *)&ll_addr, sizeof(ll_addr)) == -1) {
        fprintf(stderr, "Error:\tbind failed\n");
        close(sock);
        exit(1);
    }

    if (sock == -1) {
        fprintf(stderr, 
                "Error:\tCannot open socket\n" 
                "Info:\tMust be root with an 802.11 card with RFMON enabled\n");
        exit(1);
    }

    return sock;
}


int16_t parsetoMultiWii(Sint16 value) {
    return (int16_t)(((((double)value) + 32768.0) / 65.536) + 1000);
}


void readAxis(SDL_Event *event) {
    SDL_Event myevent = (SDL_Event)*event;

    switch (myevent.jaxis.axis) {
        case ROLL_AXIS: {
            rcData[0] = parsetoMultiWii(myevent.jaxis.value);
            IsTrimDone[0] = 0;
            break;
        }
        case PITCH_AXIS: {
            rcData[1] = parsetoMultiWii(myevent.jaxis.value);
            IsTrimDone[1] = 0;
            break;
        }
        case THROTTLE_AXIS: {
            rcData[2] = parsetoMultiWii(myevent.jaxis.value);
            IsTrimDone[2] = 0;
            break;
        }
        case YAW_AXIS: {
            rcData[3] = parsetoMultiWii(myevent.jaxis.value);
            IsTrimDone[3] = 0;
            break;
        }
        case AUX1_AXIS: {
            rcData[4] = parsetoMultiWii(myevent.jaxis.value);
            IsTrimDone[4] = 0;
            break;
        }
        case AUX2_AXIS: {
            rcData[5] = parsetoMultiWii(myevent.jaxis.value);
            IsTrimDone[5] = 0;
            break;
        }
        case AUX3_AXIS: {
            rcData[6] = parsetoMultiWii(myevent.jaxis.value);
            IsTrimDone[6] = 0;
            break;
        }
        case AUX4_AXIS: {
            rcData[7] = parsetoMultiWii(myevent.jaxis.value);
            IsTrimDone[7] = 0;
            break;
        }
        default: {
            break; //do nothing
        }
    }
}



static int eventloop_joystick(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_JOYAXISMOTION: {
                //printf ("Joystick %d, Axis %d moved to %d\n", event.jaxis.which, event.jaxis.axis, event.jaxis.value);
                readAxis(&event);
                return 2;
            }
            case SDL_JOYBUTTONDOWN: {
                if (event.jbutton.button < SWITCH_COUNT) {
                    // newer Taranis software can send 24 buttons - we use 16
                    rcData[8] |= 1 << event.jbutton.button;

                    validButton1 = rcData[8];
                }
                return 5;
            }
            case SDL_JOYBUTTONUP: {
                if (event.jbutton.button < SWITCH_COUNT) {
                    rcData[8] &= ~(1 << event.jbutton.button);
                }
                return 4;
            }
            case SDL_QUIT: {
                return 0;
            }
            default: {
                // 2020-4-5: added default case during cleanup to prevent jumping off the end of the switch
                break;
            }
        }
        usleep(100);
    }
    return 1;
}


void sendRC(unsigned char seqno, telemetry_data_t *td) {
    uint8_t i;
    uint8_t z;

    framedatas.seqnumber = seqno;
    framedatas.chan1 = rcData[0];
    framedatas.chan2 = rcData[1];
    framedatas.chan3 = rcData[2];
    framedatas.chan4 = rcData[3];
    framedatas.chan5 = rcData[4];
    framedatas.chan6 = rcData[5];
    framedatas.chan7 = rcData[6];
    framedatas.chan8 = rcData[7];
    // channels 9 - 24 as switches
    framedatas.switches = rcData[8]; 

    //printf ("rcdata0:%d\n",rcData[0]);

    framedatan.seqnumber = seqno;
    framedatan.chan1 = rcData[0];
    framedatan.chan2 = rcData[1];
    framedatan.chan3 = rcData[2];
    framedatan.chan4 = rcData[3];
    framedatan.chan5 = rcData[4];
    framedatan.chan6 = rcData[5];
    framedatan.chan7 = rcData[6];
    framedatan.chan8 = rcData[7];
    // channels 9 - 24 as switches
    framedatan.switches = rcData[8]; 

    //printf ("rcdata0:%d\n",rcData[0]);

    int best_adapter = 0;


    if (td->rx_status != NULL) {
        int j = 0;
        int ac = td->rx_status->wifi_adapter_cnt;
        int best_dbm = -1000;

        // find out which card has best signal and ignore ralink (type=1) ones
        for (j = 0; j < ac; ++j) {
            if ((best_dbm < td->rx_status->adapter[j].current_signal_dbm) && (type[j] != 0)) {
                best_dbm = td->rx_status->adapter[j].current_signal_dbm;
                best_adapter = j;
                //printf ("best_adapter: :%d\n",best_adapter);
            }
        }
        
        if (NICCount == 1) {
            if (type[0] == 2) {
                if (write(socks[0], &framedatan, sizeof(framedatan)) < 0) {
                    fprintf(stderr, "!");

                    exit(1);
                }
            } else {
                if (write(socks[0], &framedatas, sizeof(framedatas)) < 0) {
                    fprintf(stderr, "!");

                    exit(1);
                }
            }
        } else {
            if (type[best_adapter] == 2) {
                if (write(socks[best_adapter], &framedatan, sizeof(framedatan)) < 0) {
                    fprintf(stderr, "!");

                    exit(1);
                }
            } else {
                if (write(socks[best_adapter], &framedatas, sizeof(framedatas)) < 0) {
                    fprintf(stderr, "!");

                    exit(1);
                }
            }
        }
    } else {
        printf("ERROR: Could not open rx status memory!");
    }
}


wifibroadcast_rx_status_t *telemetry_wbc_status_memory_open(void) {
    int fd = 0;
    int sharedmem = 0;

    while (sharedmem == 0) {
        fd = shm_open("/wifibroadcast_rx_status_0", O_RDONLY, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            fprintf(stderr, "Could not open wifibroadcast rx status - will try again ...\n");
        } else {
            sharedmem = 1;
        }

        usleep(100000);
    }

    //if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t)) == -1) {
    //    perror("ftruncate");
    //    exit(1);
    //}

    void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t), PROT_READ, MAP_SHARED, fd, 0);
    if (retval == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    return (wifibroadcast_rx_status_t *)retval;
}


void telemetry_init(telemetry_data_t *td) {
    td->rx_status = telemetry_wbc_status_memory_open();
}


void packMessage(int seqno) {
    messageRCEncrypt[0] = rcData[0] & 0xFF;
    messageRCEncrypt[1] = rcData[0] >> 8;

    messageRCEncrypt[2] = rcData[1] & 0xFF;
    messageRCEncrypt[3] = rcData[1] >> 8;

    messageRCEncrypt[4] = rcData[2] & 0xFF;
    messageRCEncrypt[5] = rcData[2] >> 8;

    messageRCEncrypt[6] = rcData[3] & 0xFF;
    messageRCEncrypt[7] = rcData[3] >> 8;

    messageRCEncrypt[8] = rcData[4] & 0xFF;
    messageRCEncrypt[9] = rcData[4] >> 8;

    messageRCEncrypt[10] = rcData[5] & 0xFF;
    messageRCEncrypt[11] = rcData[5] >> 8;

    messageRCEncrypt[12] = rcData[6] & 0xFF;
    messageRCEncrypt[13] = rcData[6] >> 8;

    messageRCEncrypt[14] = rcData[7] & 0xFF;
    messageRCEncrypt[15] = rcData[7] >> 8;

    //int sizeinbyte = sizeof(ChannelToListen);
    //unsigned int under RPi2 = 2 byte

    messageRCEncrypt[16] = seqno & 0xFF;
    messageRCEncrypt[17] = seqno >> 8;
    messageRCEncrypt[18] = 0;
    messageRCEncrypt[18] = 1;
    messageRCEncrypt[19] = rcData[8] & 0xFF;
    messageRCEncrypt[20] = rcData[8] >> 8;
}


void CheckTrimChannel(int Channel) {
    if (TrimChannel >= 0 && ActivateChannel >= 0) {
        if (Channel == TrimChannel) {
            if (rcData[ActivateChannel] >= 1850 && IsTrimDone[Channel] == 0) {
                IsTrimDone[Channel] = 1;

                if (Action == 1) {
                    rcData[Channel] += PWMCount;

                    if (rcData[Channel] > 1999) {
                        rcData[Channel] = 1999;
                    }
                }
                
                if (Action == 0) {
                    rcData[Channel] -= PWMCount;

                    if (rcData[Channel] < 1000) {
                        rcData[Channel] = 1000;
                    }
                }
            }

            // Reverse is unselected
            if (rcData[ActivateChannel] <= 1700 && IsTrimDone[Channel] == 1) {
                IsTrimDone[Channel] = 0;

                if (Action == 1) {
                    rcData[Channel] -= PWMCount;
                    
                    if (rcData[Channel] < 1000) {
                        rcData[Channel] = 1000;
                    }
                }

                if (Action == 0) {
                    rcData[Channel] += PWMCount;

                    if (rcData[Channel] > 1999) {
                        rcData[Channel] = 1999;
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int done = 1;
    int joy_connected = 0;
    int joy = 1;
    int update_nth_time = 0;
    int shmid;
    char *shared_memory;
    int Channel = 0;
    int ChannelIPCamera = 0;
    int IsEncrypt = 0;
    int IsIPCameraSwitcherEnabled = 0;
    int IsBandSwitcherEnabled = 0;
    char ShmBuf[2];
    int tmp = 0;

    TrimChannel = 0;
    Action = 0;
    PWMCount = 0;
    ActivateChannel = 0;

    char line[100], path[100];
    FILE *procfile;

    while (1) {
        int nOptionIndex;

        static const struct option optiona[] = {
            { "help", no_argument, &flagHelp, 1 },
            {      0,           0,         0, 0 }
        };

        int c = getopt_long(argc, 
                            argv, 
                            "h:",
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
                // help
                usage();
                break;
            }
            default: {
                fprintf(stderr, "unknown switch %c\n", c);
                usage();
            }
        }
    }

    if (optind >= argc) {
        usage();
    }

    int x = optind;
    x += 9;

    Channel = atoi(argv[1]);
    ChannelIPCamera = atoi(argv[2]);

    IsBandSwitcherEnabled = atoi(argv[3]);
    IsIPCameraSwitcherEnabled = atoi(argv[4]);
    IsEncrypt = atoi(argv[5]);

    TrimChannel = atoi(argv[6]);
    Action = atoi(argv[7]);
    PWMCount = atoi(argv[8]);
    ActivateChannel = atoi(argv[9]);
    TrimChannel--;
    ActivateChannel--;


    if (IsEncrypt == 0) {
        int num_interfaces = 0;

        while (x < argc && num_interfaces < 8) {
            snprintf(path, 45, "/sys/class/net/%s/device/uevent", argv[x]);
            procfile = fopen(path, "r");
            
            if (!procfile) {
                fprintf(stderr, "ERROR: opening %s failed!\n", path);
                return 0;
            }

            // read the first line
            fgets(line, 100, procfile); 

            // read the 2nd line
            fgets(line, 100, procfile); 
            
            if (strncmp(line, "DRIVER=ath9k_htc", 16) == 0 ||
               (strncmp(line, "DRIVER=8812au", 13) == 0 ||
                strncmp(line, "DRIVER=8814au", 13) == 0 ||
                strncmp(line, "DRIVER=rtl8812au", 16) == 0 ||
                strncmp(line, "DRIVER=rtl8814au", 16) == 0 ||
                strncmp(line, "DRIVER=rtl88xxau", 16) == 0)) {
                /*
                 * Atheros/Realtek
                 * 
                 */
                if (strncmp(line, "DRIVER=ath9k_htc", 16) == 0) {
                    fprintf(stderr, "rctx: Atheros card detected\n");

                    type[num_interfaces] = 1;
                } else {
                    fprintf(stderr, "rctx: Realtek card detected\n");

                    type[num_interfaces] = 2;
                }
            } else {
                /*
                 * Ralink/Mediatek
                 * 
                 */
                fprintf(stderr, "rctx: Ralink card detected\n");

                type[num_interfaces] = 0;
            }

            socks[num_interfaces] = open_sock(argv[x]);
            ++num_interfaces;
            ++x;

            fclose(procfile);

            // wait a bit between configuring interfaces to reduce Atheros and Pi USB flakiness
            usleep(20000); 
        }
    }

    // UDP RC Encrypted init
    struct sockaddr_in si_otherRCEncrypt;
    int sRCEncrypt, iRCEncrypt, slenRCEncrypt = sizeof(si_otherRCEncrypt);

    if ((sRCEncrypt = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("sRCEncrypt");
        exit(1);
    }

    memset((char *)&si_otherRCEncrypt, 0, sizeof(si_otherRCEncrypt));
    si_otherRCEncrypt.sin_family = AF_INET;
    si_otherRCEncrypt.sin_port = htons(PORT);


    if (inet_aton(SERVER, &si_otherRCEncrypt.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    // UDP RC Encrypted end


    // UDP Band switcher init
    struct sockaddr_in si_other2;
    int s2, slen2 = sizeof(si_other2);
    char message2[BUFLEN];

    if ((s2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        exit(1);
    }

    memset((char *)&si_other2, 0, sizeof(si_other2));
    si_other2.sin_family = AF_INET;
    si_other2.sin_port = htons(PORT2);

    if (inet_aton(SERVER, &si_other2.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    // UDP Band switcher init end


    // UDP IP or USB camera sender init
    struct sockaddr_in si_other3;
    int s3, slen3 = sizeof(si_other3);
    char message3[BUFLEN];

    if ((s3 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        exit(1);
    }

    memset((char *)&si_other3, 0, sizeof(si_other3));
    si_other3.sin_family = AF_INET;
    si_other3.sin_port = htons(PORT3);

    if (inet_aton(SERVER, &si_other3.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    // UDP IP or USB camera sender end


 
    framedatan.rt1  =   0; // <-- radiotap version      (0x00)
    framedatan.rt2  =   0; // <-- radiotap version      (0x00)

    framedatan.rt3  =  13; // <- radiotap header length (0x0d)
    framedatan.rt4  =   0; // <- radiotap header length (0x00)

    framedatan.rt5  =   0; // <-- radiotap present flags(0x00)
    framedatan.rt6  = 128; // <-- RADIOTAP_TX_FLAGS +   (0x80)
    framedatan.rt7  =   8; // <-- RADIOTAP_MCS          (0x08)
    framedatan.rt8  =   0; //                           (0x00)

    framedatan.rt9  =   8; // <-- RADIOTAP_F_TX_NOACK   (0x08)
    framedatan.rt10 =   0; //                           (0x00)
    framedatan.rt11 =  55; // <-- bitmap                (0x37)
    framedatan.rt12 =  48; // <-- flags                 (0x30)
    framedatan.rt13 =   0; // <-- mcs_index             (0x00)

    framedatan.fc1  = 180; // <-- frame control field   (0xb4)
    framedatan.fc2  = 191; // <-- frame control field   (0xbf)
    framedatan.dur1 =   0; // <-- duration
    framedatan.dur2 =   0; // <-- duration

    framedatas.rt1  =   0; // <-- radiotap version
    framedatas.rt2  =   0; // <-- radiotap version

    framedatas.rt3  =  12; // <- radiotap header length
    framedatas.rt4  =   0; // <- radiotap header length

    framedatas.rt5  =   4; // <-- radiotap present flags
    framedatas.rt6  = 128; // <-- radiotap present flags
    framedatas.rt7  =   0; // <-- radiotap present flags
    framedatas.rt8  =   0; // <-- radiotap present flags

    framedatas.rt9  =  24; // <-- radiotap rate
    framedatas.rt10 =   0; // <-- radiotap stuff
    framedatas.rt11 =   0; // <-- radiotap stuff
    framedatas.rt12 =   0; // <-- radiotap stuff

    framedatas.fc1  = 180; // <-- frame control field   (0xb4)
    framedatas.fc2  = 191; // <-- frame control field   (0xbf)
    framedatas.dur1 =   0; // <-- duration
    framedatas.dur2 =   0; // <-- duration


    fprintf(stderr, "Waiting for joystick ...");
    

    while (joy) {
        joy_connected = access(JOY_DEV, F_OK);
        
        fprintf(stderr, ".");

        if (joy_connected == 0) {
            fprintf(stderr, "connected!\n");

            joy = 0;
        }

        usleep(100000);
    }


    
    rcData = rc_channels_memory_open();

    /*
     * We need to prefill channels, since we have no values for them as
     * long as the corresponding axis has not been moved yet
     *
     */
    rcData[0] = AXIS0_INITIAL;
    rcData[1] = AXIS1_INITIAL;
    rcData[2] = AXIS2_INITIAL;
    rcData[3] = AXIS3_INITIAL;
    rcData[4] = AXIS4_INITIAL;
    rcData[5] = AXIS5_INITIAL;
    rcData[6] = AXIS6_INITIAL;
    rcData[7] = AXIS7_INITIAL;
    // Switches
    rcData[8] = 0;
    

    if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO) != 0) {
        printf("ERROR: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    atexit(SDL_Quit);
    
    js = SDL_JoystickOpen(JOYSTICK_N);
    
    if (js == NULL) {
        printf("Couldn't open desired Joystick: %s\n", SDL_GetError());

        done = 0;
    } else {
        printf("\tName:       %s\n", SDL_JoystickName(JOYSTICK_N));
        printf("\tAxis:       %i\n", SDL_JoystickNumAxes(js));
        printf("\tTrackballs: %i\n", SDL_JoystickNumBalls(js));
        printf("\tButtons:   %i\n", SDL_JoystickNumButtons(js));
        printf("\tHats: %i\n", SDL_JoystickNumHats(js));
    }

    // init RSSI shared memory
    telemetry_data_t td;
    telemetry_init(&td);

    int counter = 0;
    int seqno = 0;
    int k = 0;

    while (done) {
        done = eventloop_joystick();

        //fprintf(stderr, "eventloop_joystick\n");

        if (counter % UPDATE_NTH_TIME == 0) {
            #if defined(FIX_USB_JOYSTICK_INTERRUPT_QUIRK)
            if (rcData[0] == 1000) {
                rcData[0] = lastValidCh0;
                //printf("Channel 1 currupt, replaced: " "%" PRIu16 "\n", rcData[0]);
            }

            if (rcData[1] == 1000) {
                rcData[1] = lastValidCh1;
                //printf("Channel 2 currupt, replaced: " "%" PRIu16 "\n", rcData[1]);
            }

            if (rcData[2] == 1000) {
                rcData[2] = lastValidCh2;
                //printf("Channel 3 currupt, replaced: " "%" PRIu16 "\n", rcData[2]);
            }
            
            if (rcData[3] == 1000) {
                rcData[3] = lastValidCh3;
                //printf("Channel 4 currupt, replaced: " "%" PRIu16 "\n", rcData[3]);
            }
            
            if (rcData[4] == 1000) {
                rcData[4] = lastValidCh4;
                //printf("Channel 5 currupt, replaced: " "%" PRIu16 "\n", rcData[4]);
            }
            
            if (rcData[5] == 1000) {
                rcData[5] = lastValidCh5;
                //printf("Channel 6 currupt, replaced: " "%" PRIu16 "\n", rcData[5]);
            }
            
            if (rcData[6] == 1000) {
                rcData[6] = lastValidCh6;
                //printf("Channel 7 currupt, replaced: " "%" PRIu16 "\n", rcData[6]);
            }
            
            if (rcData[7] == 1000) {
                rcData[7] = lastValidCh7;
                //printf("Channel 8 currupt, replaced: " "%" PRIu16 "\n", rcData[7]);
            }

            lastValidCh0 = rcData[0];
            lastValidCh1 = rcData[1];
            lastValidCh2 = rcData[2];
            lastValidCh3 = rcData[3];
            lastValidCh4 = rcData[4];
            lastValidCh5 = rcData[5];
            lastValidCh6 = rcData[6];
            lastValidCh7 = rcData[7];
            #endif

            validButton3 = validButton2;
            validButton2 = validButton1;

            tmp = TrimChannel;

            //fprintf(stderr, "TrimChannelPWMBefore: %d \n", rcData[tmp]);

            CheckTrimChannel(tmp);

            //fprintf(stderr, "TrimChannelPWMAfter: %d \n", rcData[tmp]);
            //fprintf(stderr, "SendRC\n");


            for (k = 0; k < TRANSMISSIONS; ++k) {
                if (IsEncrypt == 1) {
                    /*
                     * Using SVPCOM to send the RC packet
                     * 
                     */
                    packMessage(seqno);
                    
                    if (sendto(sRCEncrypt, messageRCEncrypt, 21, 0, (struct sockaddr *)&si_otherRCEncrypt, slenRCEncrypt) == -1) {
                        fprintf(stderr, "sendto() error");
                        
                        exit(1);
                    }

                } else {
                    /*
                     * Using normal wifibroadcast RC system to send RC packet
                     * 
                     */

                    if (validButton1 == validButton2 && validButton1 == validButton3 && validButton2 == validButton3) {
                        // Buttons really did change, so we send the packet out
                        sendRC(seqno, &td);
                        
                        validButton1 = rcData[8];
                        
                        //printf("OK  : " "%" PRIu16 "\n", rcData[8]);

                    } else {
                        // Buttons only changed briefly, likely the result of hardware or OS issues so we ignore it

                        discardCounter++;

                        printf("RC Blocks discarded: "
                               "%" PRIu16 "\n",
                               discardCounter);

                        //printf("FAIL: " "%" PRIu16 "\n", rcData[8]);

                    }
                }

                // Wait 2ms between sending multiple frames to lower collision probability
                usleep(2000); 
            }

            seqno++;

            if (Channel >= 1 && Channel <= 8 && IsBandSwitcherEnabled == 1) {
                message2[0] = 0;
                message2[1] = 0;
                tmp = Channel;
                tmp--;
                message2[0] = rcData[tmp] & 0xFF;
                message2[1] = rcData[tmp] >> 8;

                if (sendto(s2, message2, 2, 0, (struct sockaddr *)&si_other2, slen2) == -1) {
                    //printf("sendto() error");
                }
            }

            if (ChannelIPCamera >= 1 && ChannelIPCamera <= 8 && IsIPCameraSwitcherEnabled == 1) {
                message3[0] = 0;
                message3[1] = 0;
                tmp = ChannelIPCamera;
                tmp--;
                message3[0] = rcData[tmp] & 0xFF;
                message3[1] = rcData[tmp] >> 8;

                if (sendto(s3, message3, 2, 0, (struct sockaddr *)&si_other3, slen3) == -1) {
                    //printf("sendto() error");
                }
            } else if (ChannelIPCamera > 8 && IsIPCameraSwitcherEnabled == 1) {
                /* 
                 * This treats 2 consecutive button channels as a fake axis, if ChannelIPCamera is set to 9-24.
                 * 
                 */

                message3[0] = 0;
                message3[1] = 0;

                uint8_t static_offset = 8;

                /*
                 * Buttons are channel 9-24 in a uint16_t bitpattern in rcData[8], so we bitshift to get the correct bit
                 * for the configure camera switching channel, along with the +1 channel as the 2nd button.
                 * 
                 */
                uint8_t button_a = 1 << (ChannelIPCamera - static_offset - 1);
                uint8_t button_b = 1 << (ChannelIPCamera + 1 - static_offset - 1);

                // Because rcData[0-7] holds Axis data and rcData[8] holds all Buttondata in a bitpattern
                tmp = 8; 

                if (((rcData[tmp] & 0xFF) & button_a) == button_a && ((rcData[tmp] & 0xFF) & button_b) == button_b) {
                    /*
                     * BTNA=High & BTNB=High
                     *
                     */

                    // Decimal payload = 2000 => Hex 07D0
                    message3[0] = 0xD0; 
                    message3[1] = 0x07;
                }

                if (((rcData[tmp] & 0xFF) & button_a) == button_a && ((rcData[tmp] & 0xFF) & button_b) != button_b) {
                    /*
                     * BTNA=High & BTNB=Low
                     *
                     */

                    // Decimal payload = 1500 => Hex 05DC
                    message3[0] = 0xDC; 
                    message3[1] = 0x05;
                }

                if (((rcData[tmp] & 0xFF) & button_a) != button_a && ((rcData[tmp] & 0xFF) & button_b) != button_b) {
                    /*
                     * BTNA=Low & BTNB=Low
                     *
                     */

                    // Decimal payload = 1000 => Hex 03E8
                    message3[0] = 0xE8; 
                    message3[1] = 0x03;
                }

                if (sendto(s3, message3, 2, 0, (struct sockaddr *)&si_other3, slen3) == -1) {
                    //printf("sendto() error");
                }
            }
        }

        if (counter % JOY_CHECK_NTH_TIME == 0) {
            
            joy_connected = access(JOY_DEV, F_OK);

            if (joy_connected != 0) {
                fprintf(stderr, "joystick disconnected, exiting\n");
                done = 0;
            }
        }

        usleep(UPDATE_INTERVAL);
        counter++;
    }

    SDL_JoystickClose(js);

    close(s2);

    return EXIT_SUCCESS;
}
