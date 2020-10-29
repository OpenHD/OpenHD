#include <stdlib.h>
#include <stdio.h>
#include <sys/resource.h>
#include <SDL/SDL.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <string.h>
#include <getopt.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "openhdlib.h"

#define UPDATE_NTH_TIME 8 
#define AXIS_INITIAL 1500

#define UPDATE_INTERVAL 8000 // read Joystick every 2 ms or 500x per second
#define JOY_CHECK_NTH_TIME 400 // check if joystick disconnected every 400th time or 200ms or 5x per second
#define JOYSTICK_N 0
#define JOY_DEV "/sys/class/input/js0"

#define SERVER "127.0.0.1"
#define BUFLEN 2  //Max length of buffer
#define PORT 1260 //SettingsSync py script in

static SDL_Joystick *js;
char *ifname = NULL;
int flagHelp = 0;
uint16_t rcData[16];

int ROLL_AXIS = 0;
int PITCH_AXIS =  1;
int YAW_AXIS = 3;
int THROTTLE_AXIS = 2;
int AUX1_AXIS = 4;
int AUX2_AXIS = 5;
int AUX3_AXIS = 6;
int AUX4_AXIS = 7;

int16_t parsetoMultiWii(Sint16 value) {
	return (int16_t)(((((double)value)+32768.0)/65.536)+1000);
}


void readAxis(SDL_Event *event) {
	SDL_Event myevent = (SDL_Event)*event;
	if ( myevent.jaxis.axis == ROLL_AXIS)
		rcData[0]=parsetoMultiWii(myevent.jaxis.value);

	if ( myevent.jaxis.axis == PITCH_AXIS)
		rcData[1]=parsetoMultiWii(myevent.jaxis.value);

	if ( myevent.jaxis.axis == THROTTLE_AXIS)
		rcData[2]=parsetoMultiWii(myevent.jaxis.value);

	if ( myevent.jaxis.axis == YAW_AXIS)
		rcData[3]=parsetoMultiWii(myevent.jaxis.value);

	if ( myevent.jaxis.axis ==  AUX1_AXIS)
		rcData[4]=parsetoMultiWii(myevent.jaxis.value);

	if ( myevent.jaxis.axis == AUX2_AXIS)
		rcData[5]=parsetoMultiWii(myevent.jaxis.value);

	if ( myevent.jaxis.axis == AUX3_AXIS)
		rcData[6]=parsetoMultiWii(myevent.jaxis.value);

	if ( myevent.jaxis.axis == AUX4_AXIS)
		rcData[7]=parsetoMultiWii(myevent.jaxis.value);
}


static int eventloop_joystick (void) {
	SDL_Event event;
	while (SDL_PollEvent (&event)) {
		switch (event.type) {
			case SDL_JOYAXISMOTION:
				//printf ("Joystick %d, Axis %d moved to %d\n", event.jaxis.which, event.jaxis.axis, event.jaxis.value);
				readAxis(&event);
				return 2;
				break;
			case SDL_JOYBUTTONDOWN:
				if (event.jbutton.button < SWITCH_COUNT) { // newer Taranis software can send 24 buttons - we use 16
					rcData[8] |= 1 << event.jbutton.button;
				}
				return 5;
				break;
			case SDL_JOYBUTTONUP:
				if (event.jbutton.button < SWITCH_COUNT) {
					rcData[8] &= ~(1 << event.jbutton.button);
				}
				return 4;
				break;
			case SDL_QUIT:
				return 0;
		}
		usleep(100);
	}
  return 1;
}


int main (int argc, char *argv[]) {

    int tmp = 0;
    int done = 1;
    int joy_connected = 0;
    int joy = 1;
    int Channel = 1;
    int counter = 0;
    char message[BUFLEN];
        rcData[0]=AXIS_INITIAL;
        rcData[1]=AXIS_INITIAL;
        rcData[2]=AXIS_INITIAL;
        rcData[3]=AXIS_INITIAL;
        rcData[4]=AXIS_INITIAL;
        rcData[5]=AXIS_INITIAL;
        rcData[6]=AXIS_INITIAL;
        rcData[7]=AXIS_INITIAL;

	Channel = atoi(argv[1]);
	ROLL_AXIS =	atoi(argv[2]);
	PITCH_AXIS = 	atoi(argv[3]);
	YAW_AXIS = 	atoi(argv[4]);
	THROTTLE_AXIS = atoi(argv[5]);
	AUX1_AXIS = 	atoi(argv[6]);
	AUX2_AXIS = 	atoi(argv[7]);
	AUX3_AXIS = 	atoi(argv[8]);
	AUX4_AXIS = 	atoi(argv[9]);

	//udp init
        struct sockaddr_in si_other;
        int s, slen = sizeof(si_other);

        if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        {
                exit(1);
        }

        memset((char *) &si_other, 0, sizeof(si_other));
        si_other.sin_family = AF_INET;
        si_other.sin_port = htons(PORT);

        if (inet_aton(SERVER, &si_other.sin_addr) == 0)
        {
                fprintf(stderr, "inet_aton() failed\n");
                exit(1);
        }
        //udp init end


while (joy)
{
	joy_connected=access(JOY_DEV, F_OK);
	fprintf(stderr, ".");
	if (joy_connected == 0)
	{
		fprintf(stderr, "connected!\n");
		joy=0;
	}
	usleep(100000);
}


if (SDL_Init (SDL_INIT_JOYSTICK | SDL_INIT_VIDEO) != 0)
	{
		printf ("ERROR: %s\n", SDL_GetError ());
		return EXIT_FAILURE;
	}
	atexit (SDL_Quit);
	js = SDL_JoystickOpen (JOYSTICK_N);
	if (js == NULL)
	{
		printf("Couldn't open desired Joystick: %s\n",SDL_GetError());
		done=0;
	} else {
		printf ("\tName:       %s\n", SDL_JoystickName(JOYSTICK_N));
		printf ("\tAxis:       %i\n", SDL_JoystickNumAxes(js));
		printf ("\tTrackballs: %i\n", SDL_JoystickNumBalls(js));
		printf ("\tButtons:   %i\n",SDL_JoystickNumButtons(js));
		printf ("\tHats: %i\n",SDL_JoystickNumHats(js)); 
}


	while (done) {
		done = eventloop_joystick();
//		fprintf(stderr, "eventloop_joystick\n");
		if (counter % UPDATE_NTH_TIME == 0) {
//		    fprintf(stderr, "SendRC\n");

                    if( Channel >= 1 && Channel <= 8 )
		    {
			message[0] = 0;
			message[1] = 0;
			tmp = Channel;
			tmp--;
			message[0] = rcData[tmp] & 0xFF;
			message[1] = rcData[tmp] >> 8;
			
			printf("Value: %d \n", rcData[tmp]);
                        if (sendto(s, message, BUFLEN, 0, (struct sockaddr *) &si_other, slen) == -1)
			{
				//printf("sendto() error");
			}

                    }
		}
		if (counter % JOY_CHECK_NTH_TIME == 0) {
		    joy_connected=access(JOY_DEV, F_OK);
		    if (joy_connected != 0) {
			fprintf(stderr, "joystick disconnected, exiting\n");
			done=0;
		    }
		}
		usleep(UPDATE_INTERVAL);
		counter++;
	}
	SDL_JoystickClose (js);


}
