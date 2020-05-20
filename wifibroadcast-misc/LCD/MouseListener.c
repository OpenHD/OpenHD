#include <fcntl.h>
#include <linux/input.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>



/*
 * Lighter area
 */
int LighterTopLeftX = 225;
int LighterTopLeftY = 1;

int LighterBottomRigthX = 525;
int LighterBottomRigthY = 150;


/*
 * Darker area
 */
int DarkerTopLeftX = 225;
int DarkerTopLeftY = 330;

int DarkerBottomRigthX = 525;
int DarkerBottomRigthY = 480;


/*
 * On/Off area
 */
int OnOffTopLeftX = 1;
int OnOffTopLeftY = 1;

int OnOffBottomRigthX = 150;
int OnOffBottomRigthY = 480;


/*
 * Next area
 */
int NextTopLeftX = 650;
int NextTopLeftY = 1;

int NextBottomRigthX = 800;
int NextBottomRigthY = 480;


/*
 * IncreaseBrightnessBy
 */
const int IncreaseBrightnessBy = 50;
const int LightOn = 255;
const int LightOff = 0;
int LightCurrent = 255;
int Dim = 128;
int IsOn = 1;



/* 
 * AutoDim
 */
int DifAfter = 10;
time_t DimTimerLast = 0;
time_t DimTimeNow = 0;



/*
 * Mouse state structure
 */
typedef struct {
    int fd;
    struct input_event ev;
    int x, y;
} mouse_t;



/* 
 * Global mouse state
 */
mouse_t mouse; 




char PathToDev[256];
char PathToExec[256];



void CheckArea() {

    /*
     * Check lighter area
     */
    if (mouse.x >= LighterTopLeftX & mouse.y >= LighterTopLeftY) {
        if (mouse.x <= LighterBottomRigthX & mouse.y <= LighterBottomRigthY) {

            if (LightCurrent < 255) {
                LightCurrent += IncreaseBrightnessBy;

                char LightStr[4];
                memset(LightStr, '\0', sizeof(LightStr));
                snprintf(LightStr, 10, "%d", LightCurrent);

                //echo n > /sys/class/backlight/rpi_backlight/brightness

                char ExecStr[255];
                memset(ExecStr, '\0', sizeof(ExecStr));
                strcpy(ExecStr, "echo ");

                int LightStrLen = strlen(LightStr);
                strncpy(&ExecStr[5], LightStr, LightStrLen);
                int Offset = 5 + LightStrLen;
                strcpy(&ExecStr[Offset], " > /sys/class/backlight/rpi_backlight/brightness");
                //strcpy(&ExecStr[Offset], " > /tmp/testfile");

                //printf("\n ExecStr is: %s\n", ExecStr);

                system(ExecStr);
            }
        }
    }



    /* 
     * Check darker area
     */
    if (mouse.x >= DarkerTopLeftX & mouse.y >= DarkerTopLeftY) {
        if (mouse.x <= DarkerBottomRigthX & mouse.y <= DarkerBottomRigthY) {

            if (LightCurrent > 50) {
                LightCurrent -= IncreaseBrightnessBy;

                char LightStr[4];
                memset(LightStr, '\0', sizeof(LightStr));
                snprintf(LightStr, 10, "%d", LightCurrent);

                //echo n > /sys/class/backlight/rpi_backlight/brightness

                char ExecStr[255];
                memset(ExecStr, '\0', sizeof(ExecStr));
                strcpy(ExecStr, "echo ");

                int LightStrLen = strlen(LightStr);
                strncpy(&ExecStr[5], LightStr, LightStrLen);
                int Offset = 5 + LightStrLen;
                strcpy(&ExecStr[Offset], " > /sys/class/backlight/rpi_backlight/brightness");
                //strcpy(&ExecStr[Offset], " > /tmp/testfile");

                //printf("\n ExecStr is: %s\n", ExecStr);
                
                system(ExecStr);
            }
        }
    }



    /* 
     * Check on/off area
     */
    if (mouse.x >= OnOffTopLeftX & mouse.y >= OnOffTopLeftY) {
        if (mouse.x <= OnOffBottomRigthX & mouse.y <= OnOffBottomRigthY) {

            if (IsOn == 1) {
                IsOn = 0;

                char LightStr[4];
                memset(LightStr, '\0', sizeof(LightStr));
                snprintf(LightStr, 10, "%d", 0);

                //echo n > /sys/class/backlight/rpi_backlight/brightness

                char ExecStr[255];
                memset(ExecStr, '\0', sizeof(ExecStr));
                strcpy(ExecStr, "echo ");

                int LightStrLen = strlen(LightStr);
                strncpy(&ExecStr[5], LightStr, LightStrLen);
                int Offset = 5 + LightStrLen;
                strcpy(&ExecStr[Offset], " > /sys/class/backlight/rpi_backlight/brightness");
                //strcpy(&ExecStr[Offset], " > /tmp/testfile");

                //printf("\n ExecStr is: %s\n", ExecStr);
                
                system(ExecStr);
            } else {
                IsOn = 1;

                LightCurrent = 255;

                char LightStr[4];
                memset(LightStr, '\0', sizeof(LightStr));
                snprintf(LightStr, 10, "%d", 255);

                //echo n > /sys/class/backlight/rpi_backlight/brightness

                char ExecStr[255];
                memset(ExecStr, '\0', sizeof(ExecStr));
                strcpy(ExecStr, "echo ");

                int LightStrLen = strlen(LightStr);
                strncpy(&ExecStr[5], LightStr, LightStrLen);
                int Offset = 5 + LightStrLen;
                strcpy(&ExecStr[Offset], " > /sys/class/backlight/rpi_backlight/brightness");
                //strcpy(&ExecStr[Offset], " > /tmp/testfile");
                
                //printf("\n ExecStr is: %s\n", ExecStr);
                
                system(ExecStr);
            }
        } else {
            /* 
             * Turn on screen in case if any area touched and screen is off
             */
            if (IsOn == 0) {
                IsOn = 1;

                LightCurrent = 255;

                //printf("\n\nTurnOn from any area\n\n");

                char LightStr[4];
                memset(LightStr, '\0', sizeof(LightStr));
                snprintf(LightStr, 10, "%d", LightCurrent);

                //echo n > /sys/class/backlight/rpi_backlight/brightness

                char ExecStr[255];
                memset(ExecStr, '\0', sizeof(ExecStr));
                strcpy(ExecStr, "echo ");

                int LightStrLen = strlen(LightStr);
                strncpy(&ExecStr[5], LightStr, LightStrLen);
                int Offset = 5 + LightStrLen;
                strcpy(&ExecStr[Offset], " > /sys/class/backlight/rpi_backlight/brightness");
                //strcpy(&ExecStr[Offset], " > /tmp/testfile");

                //printf("\n ExecStr is: %s\n", ExecStr);
                
                system(ExecStr);
            }
        }
    }


    /*
     * Check next area
     */
    if (mouse.x >= NextTopLeftX & mouse.y >= NextTopLeftY) {
        if (mouse.x <= NextBottomRigthX & mouse.y <= NextBottomRigthY) {
            //printf("\n\nInNextArea\n\n");

            /*
             * To run external program:
             */
            //system("/usr/bin/MyProgName param");
        }
    }
}

/*
 * evenThread reads from the mouse input file
 */
void *eventThread(void *arg) {
    int IfUp = 0;

    /*
     * Open mouse driver
     */
    if ((mouse.fd = open(PathToDev, O_RDONLY)) < 0) {
        fprintf(stderr, "Error opening Mouse!\n");

    } else {
        while (1) {
            read(mouse.fd, &mouse.ev, sizeof(struct input_event));
            
            /* 
             * Check events
             *
             */
            if (mouse.ev.type == EV_ABS) {
                if (mouse.ev.code == ABS_X) {
                    mouse.x = mouse.ev.value;

                    //printf("ABS_X %d\n", mouse.ev.value);
                } else if (mouse.ev.code == ABS_Y) {
                    mouse.y = mouse.ev.value;

                    //printf("ABS_Y %d\n", mouse.ev.value);
                }
            }

            if (mouse.ev.type == EV_KEY) {
                if (mouse.ev.code == BTN_TOUCH) {
                    if (IfUp == 0) {
                        IfUp = 1;

                    } else {
                        IfUp = 0;

                        time(&DimTimerLast);

                        CheckArea();
                    }
                }

                if (mouse.ev.code == BTN_RIGHT) {
                    //printf("Right button\n");
                }
            }

            usleep(50000);
        }
    }
}



int mouseinit() {
    pthread_t inputThread;

    return pthread_create(&inputThread, NULL, &eventThread, NULL);
}



int main(int argc, char *argv[]) {
    time(&DimTimerLast);


    if (argc < 3) {
        printf("Usage: ./MouseListener 360 130 /dev/input/event0\n");

        exit(1);
    } else {

        char IsDimEnabledStr[10];
        size_t IsDimEnabledStrLen = strlen(argv[1]) + 1;
        memcpy(IsDimEnabledStr, argv[1], IsDimEnabledStrLen);
        //printf("IsEnabledStr: &s", IsDimEnabledStr);

        int tmp = atoi(IsDimEnabledStr);

        DifAfter = tmp;

        printf("Dim After: %d second(s)\n", tmp);

        char DimValueStr[10];
        size_t DimValueLen = strlen(argv[2]) + 1;
        memcpy(DimValueStr, argv[2], DimValueLen);
        //printf("IsEnabledStr: &s", IsDimEnabledStr);

        tmp = atoi(DimValueStr);

        printf("Dim value: %d\n", tmp);

        Dim = tmp;

        size_t len1 = strlen(argv[3]) + 1;
        memcpy(PathToDev, argv[3], len1);

        printf(PathToDev);
    }

    if (mouseinit() != 0) {
        fprintf(stderr, "Unable to initialize the mouse\n");

        exit(1);
    }

    double IdleTime = 0;
    int IsDim = 0;

    while (1) {
        time(&DimTimeNow);

        IdleTime = difftime(DimTimeNow, DimTimerLast);

        /*
         * 2 days of idle
         */
        if (IdleTime >= 172800) {
            //rest Timer in case of long idle time
            time(&DimTimerLast);
        }


        /* 
         * Don`t dim if:
         * 
         * 1. Display off
         * 2. LightCurrent value is lower than Dim value
         */
        if (IdleTime >= DifAfter & LightCurrent > Dim & IsOn == 1 & IsDim != 1 & DifAfter != 0) {
            IsDim = 1;

            //printf("Dim is on\n");
            char LightStr[4];
            memset(LightStr, '\0', sizeof(LightStr));
            snprintf(LightStr, 10, "%d", Dim);

            //echo n > /sys/class/backlight/rpi_backlight/brightness

            char ExecStr[255];
            memset(ExecStr, '\0', sizeof(ExecStr));
            strcpy(ExecStr, "echo ");

            int LightStrLen = strlen(LightStr);
            strncpy(&ExecStr[5], LightStr, LightStrLen);
            int Offset = 5 + LightStrLen;
            strcpy(&ExecStr[Offset], " > /sys/class/backlight/rpi_backlight/brightness");

            //strcpy(&ExecStr[Offset], " > /tmp/testfile");

            //printf("\n ExecStr is: %s\n", ExecStr);

            system(ExecStr);
        }



        if (IsDim == 1 & IdleTime < DifAfter & DifAfter != 0) {
            /*
             * Restore LightCurrent
             */
            IsDim = 0;

            //printf("resore");
            char LightStr[4];
            memset(LightStr, '\0', sizeof(LightStr));
            snprintf(LightStr, 10, "%d", LightCurrent);

            //echo n > /sys/class/backlight/rpi_backlight/brightness

            char ExecStr[255];
            memset(ExecStr, '\0', sizeof(ExecStr));
            strcpy(ExecStr, "echo ");

            int LightStrLen = strlen(LightStr);
            strncpy(&ExecStr[5], LightStr, LightStrLen);
            int Offset = 5 + LightStrLen;
            strcpy(&ExecStr[Offset], " > /sys/class/backlight/rpi_backlight/brightness");

            //strcpy(&ExecStr[Offset], " > /tmp/testfile");
            
            //printf("\n ExecStr is: %s\n", ExecStr);
            
            system(ExecStr);
        }

        sleep(2);
    }

    exit(0);
}
