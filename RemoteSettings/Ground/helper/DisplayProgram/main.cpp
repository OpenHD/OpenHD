#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "fontinfo.h"
#include "shapes.h"
#include <vector>
#include <string>
#include <cstring>
#include <deque>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>

std::deque<std::string> ScreenBuf;


//0 - Joystick, 1 - Awaiting, 2 - Downloading

char InitPath[] = {"/usr/bin/omxplayer --loop /home/pi/RemoteSettings/Ground/helper/DisplayProgram/video/InitVideo.mp4 > /dev/null 2>/dev/null &"};

char AwaitingPath[] = {"/usr/bin/omxplayer --loop /home/pi/RemoteSettings/Ground/helper/DisplayProgram/video/AwaitingVideo.mp4 > /dev/null 2>/dev/null &"};

char DownloadPath[] = {"/usr/bin/omxplayer --loop /home/pi/RemoteSettings/Ground/helper/DisplayProgram/video/DownloadVideo.mp4 > /dev/null 2>/dev/null &"};

int CurrentMode = 0;

void DrawText(int width, int height, int fontscale)
{
	
	int fw = TextHeight(SansTypeface, width / fontscale);
	int positiony =  0;
	int positionx = 0;
	Fill(255, 255, 255, 1);
	int ScreenSize = ScreenBuf.size();
	
	if(ScreenSize > 0)
		ScreenSize--;
	for(int i=ScreenSize; i>=0;i--)
        {
		std::string str  = ScreenBuf[i];
                char *TextBuf = new char[str.length() + 1];
                strcpy(TextBuf, str.c_str());
                //DrawText(width, height, 65, cstr );

		positiony +=  fw;
		Fill(255, 255, 255, 1);
		Text( positionx,  positiony ,TextBuf, SansTypeface, width / fontscale);
                delete [] TextBuf;
        }

}

void RenderScren(int width, int height)
{
	//Start(width/2, height/2);
        Start(width, height);
//	Background(5, 5, 5);
//	DrawImage(width, height);
	DrawText(width, height, 65);

	End();
}

int KillDisplayProgram()
{
	system("killall omxplayer  > /dev/null 2>/dev/null");
	system("killall omxplayer.bin  > /dev/null 2>/dev/null");
	return 0;
}

void StartVideo(int VideoID)
{
	//0 - Joystick, 1 - Awaiting, 2 - Downloading

	if( CurrentMode != VideoID)
	{
		KillDisplayProgram();
		switch(VideoID)
		{
			case 0:
			system(InitPath);
			CurrentMode = 0;
			break;

                	case 1:
              		system(AwaitingPath);
			CurrentMode = 1;
                	break;

                	case 2:
                	system(DownloadPath);
			CurrentMode = 2;
                	break;

			default:
			break; 
		}
	}
}

void *UDPThread(void *arg)
{

	char InBuffer[1500];
	struct sockaddr_in servaddr, cliaddr;
	int sockfd = 0;

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
	{
        	perror("socket creation failed");
        	exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	servaddr.sin_family    = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(1379);

	if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	{
        	perror("bind failed");
        	exit(EXIT_FAILURE);
	} 

	int EndThread=0;
	socklen_t len = 0;
	int ret = 0;
	char InMsgAwaiting[] = "SwitchToAwaiting";
	char InMsgJoystick[] = "SwitchToJoystick";
	char InMsgDownload[] = "SwitchToDownload";
	do
	{
		ret = recvfrom(sockfd, (char *)InBuffer, 1500, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
		if(ret > 0)
		{
			std::string str(InBuffer,ret);
			if(strncmp(&InBuffer[0], "SwitchTo",8) != 0)
				ScreenBuf.push_back(str);

			if(ret == 16)
			{
				if( strncmp(&InBuffer[0],&InMsgAwaiting[0],16) ==0 )
				{
                                        StartVideo(1);
				}

				if( strncmp(&InBuffer[0],&InMsgJoystick[0],16) ==0 )
                                {
					StartVideo(0);
                                }

				if( strncmp(&InBuffer[0],&InMsgDownload[0],16) ==0 )
                                {
                                        StartVideo(2);
                                }
			}

		} 

	}while(EndThread == 0);


	return NULL;
}

pthread_t hUDPThread=0;

int main(int argc, char *argv[]) {

	KillDisplayProgram();
	system(InitPath);
	CurrentMode = 0;

	int width, height;
	ScreenBuf.push_back("SSync screen started");



	init(&width, &height);
	width = width;
	height = height;

	int result = pthread_create(&(hUDPThread), NULL, &UDPThread, NULL);
        if (result != 0)
            printf("\nThread error: [%s]", strerror(result));

	for(int i=0;i<360000;i++)
	{
		RenderScren(width, height);
		sleep(0.1);
	}

 
    exit(0);
}
