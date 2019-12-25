#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>


// Lighter area
int LighterTopLeftX = 225;
int LighterTopLeftY = 1;

int LighterBottomRigthX = 525;
int LighterBottomRigthY = 150;

//Darker area
int DarkerTopLeftX = 225;
int DarkerTopLeftY = 330;

int DarkerBottomRigthX = 525;
int DarkerBottomRigthY = 480;

//OnOff area
int OnOffTopLeftX = 1;
int OnOffTopLeftY = 1;

int OnOffBottomRigthX = 150;
int OnOffBottomRigthY = 480;

//Next area
int NextTopLeftX = 650;
int NextTopLeftY = 1;

int NextBottomRigthX = 800;
int NextBottomRigthY = 480;

//IncreaseBrightnessBy
const int IncreaseBrightnessBy = 50;
const int LightOn = 255;
const int LightOff = 0;
int LightCurrent = 255;
int Dim = 128;
int IsOn = 1;
//AutoDim
int DifAfter = 10;
time_t DimTimerLast = 0;
time_t DimTimeNow = 0;

// Mouse state structure
typedef struct {
    int fd;
    struct input_event ev;
    int x, y;
} mouse_t;

mouse_t mouse;			// global mouse state
char PathToDev[256];
char PathToExec[256];

void CheckArea()
{
    //remove if not debug
    //Notebook scale
    //        double XScale = 5.25;
    //	mouse.x =  mouse.x - 1275;
    //        mouse.x = mouse.x / XScale;

    //        double YScale = 8;
    //	mouse.y = mouse.y - 1044;
    //	mouse.y =  mouse.y / YScale;

    //Notebook scale end
    //	printf("X is : %d \n", mouse.x);
    //	printf("Y is : %d \n", mouse.y);
    //	printf("LighterTopLeftX is %d LighterTopLeftY is %d\n", LighterTopLeftX, LighterTopLeftY);
    //	printf("LighterBottomRigthX is %d  LighterBottomRigthY is %d\n", LighterBottomRigthX, LighterBottomRigthY);

    //check Lighter
    if(mouse.x  >= LighterTopLeftX & mouse.y >= LighterTopLeftY)
    {
        if(mouse.x <= LighterBottomRigthX & mouse.y <= LighterBottomRigthY)
        {
            //printf("\n \n InLighterArea\n\n");
            if(LightCurrent < 255)
            {
                LightCurrent += IncreaseBrightnessBy;
                char LightStr[4];
                memset(LightStr, '\0', sizeof(LightStr));
                snprintf(LightStr, 10, "%d", LightCurrent);
                //echo n > /sys/class/backlight/rpi_backlight/brightness
                char ExecStr[255];
                memset(ExecStr, '\0', sizeof(ExecStr));
                strcpy(ExecStr, "echo ");
                int LightStrLen =  strlen(LightStr);
                strncpy(&ExecStr[5], LightStr, LightStrLen );
                int Offset = 5 + LightStrLen;
                strcpy(&ExecStr[Offset], " > /sys/class/backlight/rpi_backlight/brightness");
                //strcpy(&ExecStr[Offset], " > /tmp/testfile");
                //printf("\n ExecStr is: %s\n", ExecStr);
                system(ExecStr);

            }
        }
    }

    //check Darker area
    if(mouse.x  >= DarkerTopLeftX & mouse.y >= DarkerTopLeftY)
    {
        if(mouse.x <= DarkerBottomRigthX & mouse.y <= DarkerBottomRigthY)
        {
            //printf("\n\nInDarkerArea\n\n");
            if(LightCurrent > 50)
            {
                LightCurrent -= IncreaseBrightnessBy;
                char LightStr[4];
                memset(LightStr, '\0', sizeof(LightStr));
                snprintf(LightStr, 10, "%d", LightCurrent);
                //echo n > /sys/class/backlight/rpi_backlight/brightness
                char ExecStr[255];
                memset(ExecStr, '\0', sizeof(ExecStr));
                strcpy(ExecStr, "echo ");
                int LightStrLen =  strlen(LightStr);
                strncpy(&ExecStr[5], LightStr, LightStrLen );
                int Offset = 5 + LightStrLen;
                strcpy(&ExecStr[Offset], " > /sys/class/backlight/rpi_backlight/brightness");
                //strcpy(&ExecStr[Offset], " > /tmp/testfile");
                //printf("\n ExecStr is: %s\n", ExecStr);
                system(ExecStr);
            }


        }
    }

    //check OnOff area
    if(mouse.x  >= OnOffTopLeftX & mouse.y >= OnOffTopLeftY)
    {
        if(mouse.x <= OnOffBottomRigthX & mouse.y <= OnOffBottomRigthY)
        {
            //printf("\n\nInOnOffArea\n\n");
            if(IsOn == 1)
            {
                IsOn = 0;
                char LightStr[4];
                memset(LightStr, '\0', sizeof(LightStr));
                snprintf(LightStr, 10, "%d", 0);
                //echo n > /sys/class/backlight/rpi_backlight/brightness
                char ExecStr[255];
                memset(ExecStr, '\0', sizeof(ExecStr));
                strcpy(ExecStr, "echo ");
                int LightStrLen =  strlen(LightStr);
                strncpy(&ExecStr[5], LightStr, LightStrLen );
                int Offset = 5 + LightStrLen;
                strcpy(&ExecStr[Offset], " > /sys/class/backlight/rpi_backlight/brightness");
                //strcpy(&ExecStr[Offset], " > /tmp/testfile");
                //printf("\n ExecStr is: %s\n", ExecStr);
                system(ExecStr);
            }
            else
            {
                IsOn = 1;
                LightCurrent=255;
                char LightStr[4];
                memset(LightStr, '\0', sizeof(LightStr));
                snprintf(LightStr, 10, "%d", 255);
                //echo n > /sys/class/backlight/rpi_backlight/brightness
                char ExecStr[255];
                memset(ExecStr, '\0', sizeof(ExecStr));
                strcpy(ExecStr, "echo ");
                int LightStrLen =  strlen(LightStr);
                strncpy(&ExecStr[5], LightStr, LightStrLen );
                int Offset = 5 + LightStrLen;
                strcpy(&ExecStr[Offset], " > /sys/class/backlight/rpi_backlight/brightness");
                //strcpy(&ExecStr[Offset], " > /tmp/testfile");
                //printf("\n ExecStr is: %s\n", ExecStr);
                system(ExecStr);
            }
        }
        else
        {
            //TurnOn Screen in case if any area touched and screen is off.
            if(IsOn == 0)
            {
                IsOn = 1;
                LightCurrent=255;
                //printf("\n\nTurnOn from any area\n\n");
                char LightStr[4];
                memset(LightStr, '\0', sizeof(LightStr));
                snprintf(LightStr, 10, "%d", LightCurrent);
                //echo n > /sys/class/backlight/rpi_backlight/brightness
                char ExecStr[255];
                memset(ExecStr, '\0', sizeof(ExecStr));
                strcpy(ExecStr, "echo ");
                int LightStrLen =  strlen(LightStr);
                strncpy(&ExecStr[5], LightStr, LightStrLen );
                int Offset = 5 + LightStrLen;
                strcpy(&ExecStr[Offset], " > /sys/class/backlight/rpi_backlight/brightness");
                //strcpy(&ExecStr[Offset], " > /tmp/testfile");
                //printf("\n ExecStr is: %s\n", ExecStr);
                system(ExecStr);
            }
        }


    }

    //check Next area
    if(mouse.x  >= NextTopLeftX & mouse.y >= NextTopLeftY)
    {
        if(mouse.x <= NextBottomRigthX & mouse.y <= NextBottomRigthY)
        {
            //printf("\n\nInNextArea\n\n");
            //to run external program:
            //system("/usr/bin/MyProgName param");
        }
    }

}

// evenThread reads from the mouse input file
void *eventThread(void *arg) {
    int IfUp = 0;

    // Open mouse driver
    if ((mouse.fd = open(PathToDev, O_RDONLY)) < 0)
    {
        fprintf(stderr, "Error opening Mouse!\n");

    }
    else
    {
        while (1)
        {
            read(mouse.fd, &mouse.ev, sizeof(struct input_event));
            // Check events
            if (mouse.ev.type == EV_ABS)
            {
                if (mouse.ev.code == ABS_X)
                {
                    //printf("ABS_X %d\n", mouse.ev.value);
                    mouse.x = mouse.ev.value;
                }
                else if (mouse.ev.code == ABS_Y)
                {
                    //printf("ABS_Y %d\n", mouse.ev.value);
                    mouse.y = mouse.ev.value;
                }
            }

            if (mouse.ev.type == EV_KEY)
            {
                if (mouse.ev.code == BTN_TOUCH)
                {
                    if(IfUp == 0)
                    {
                        IfUp = 1;
                    }
                    else
                    {
                        IfUp = 0;
                        time(&DimTimerLast);
                        CheckArea();
                    }
                }

                if (mouse.ev.code == BTN_RIGHT)
                {
                    //printf("Right button\n");
                }
            }
        }
    }
}

int mouseinit() {
    pthread_t inputThread;
    return pthread_create(&inputThread, NULL, &eventThread, NULL);
}


int main(int argc, char *argv[]) {
    time(&DimTimerLast);
    if(argc < 3)
    {
        printf("Usage: ./MouseListener 360 130 /dev/input/event0\n");
        exit(1);
    }
    else
    {

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
        Dim=tmp;

        size_t len1 = strlen(argv[3]) + 1;
        memcpy(PathToDev, argv[3], len1);
        printf(PathToDev);

    }

    if (mouseinit() != 0)
    {
        fprintf(stderr, "Unable to initialize the mouse\n");
        exit(1);
    }

    double IdleTime = 0;
    int IsDim = 0;

    while (1)
    {
        time(&DimTimeNow);
        IdleTime = difftime(DimTimeNow, DimTimerLast);
        //printf("Idle time is: %f \n", IdleTime );
        if (IdleTime >= 172800) //2 days of idle
        {
            //rest Timer in case of long idle time
            time(&DimTimerLast);
        }
        //Don`t dim if:
        //1. Display off
        //2. LightCurrent value is lower than Dim value
        if (IdleTime >= DifAfter & LightCurrent > Dim & IsOn == 1 & IsDim != 1 & DifAfter != 0)
        {
            IsDim = 1;
            //printf("Dim is on\n");
            char LightStr[4];
            memset(LightStr, '\0', sizeof(LightStr));
            snprintf(LightStr, 10, "%d", Dim);
            //echo n > /sys/class/backlight/rpi_backlight/brightness
            char ExecStr[255];
            memset(ExecStr, '\0', sizeof(ExecStr));
            strcpy(ExecStr, "echo ");
            int LightStrLen =  strlen(LightStr);
            strncpy(&ExecStr[5], LightStr, LightStrLen );
            int Offset = 5 + LightStrLen;
            strcpy(&ExecStr[Offset], " > /sys/class/backlight/rpi_backlight/brightness");
            //strcpy(&ExecStr[Offset], " > /tmp/testfile");
            //printf("\n ExecStr is: %s\n", ExecStr);
            system(ExecStr);

        }

        if (IsDim == 1 & IdleTime < DifAfter & DifAfter != 0)
        {
            //resore LightCurrent
            IsDim = 0;
            //printf("resore");
            char LightStr[4];
            memset(LightStr, '\0', sizeof(LightStr));
            snprintf(LightStr, 10, "%d", LightCurrent);
            //echo n > /sys/class/backlight/rpi_backlight/brightness
            char ExecStr[255];
            memset(ExecStr, '\0', sizeof(ExecStr));
            strcpy(ExecStr, "echo ");
            int LightStrLen =  strlen(LightStr);
            strncpy(&ExecStr[5], LightStr, LightStrLen );
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
