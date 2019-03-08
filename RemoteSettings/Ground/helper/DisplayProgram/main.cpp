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
VGImage Joystick = 0;

std::vector<VGImage> Awaiting;
std::vector<VGImage>::iterator AwaitingIter;

std::vector<VGImage> Download;
std::vector<VGImage>::iterator DownloadIter;

//VGImage *Current = 0;

//0 - Joystick, 1 - Awaiting, 2 - Downloading
int SelectedImage = 0;

char JoystickPath[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Joystick/0.jpg"};

char Awaiting0Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Awaiting/0.jpg"};
char Awaiting1Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Awaiting/1.jpg"};
char Awaiting2Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Awaiting/2.jpg"};
char Awaiting3Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Awaiting/3.jpg"};
char Awaiting4Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Awaiting/4.jpg"};
char Awaiting5Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Awaiting/5.jpg"};
char Awaiting6Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Awaiting/6.jpg"};
char Awaiting7Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Awaiting/7.jpg"};

char Download0Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Download/0.jpg"};
char Download1Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Download/1.jpg"};
char Download2Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Download/2.jpg"};
char Download3Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Download/3.jpg"};
char Download4Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Download/4.jpg"};
char Download5Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Download/5.jpg"};
char Download6Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Download/6.jpg"};
char Download7Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Download/7.jpg"};
char Download8Path[] = {"/home/pi/RemoteSettings/Ground/helper/DisplayProgram/Images/Download/8.jpg"};

time_t TimerLastSwitchAwaiting;
time_t TimerLastSwitchDownloading;

void SwitchImageByTimer()
{
	double SwitchAfterAwaiting = 0.5;
	double SwitchAfterDownload = 0.2;
	time_t now;
	time(&now);


	double diffAwaiting = difftime(now, TimerLastSwitchAwaiting);
	if(  diffAwaiting  > SwitchAfterAwaiting)
	{
		time(&TimerLastSwitchAwaiting);
		if(AwaitingIter == Awaiting.end()-1 )
			AwaitingIter = Awaiting.begin();
		else
			++AwaitingIter;
	}

        double diffDownload = difftime(now, TimerLastSwitchDownloading);
        if(  diffDownload  > SwitchAfterDownload)
        {
                time(&TimerLastSwitchDownloading);
                if(DownloadIter == Download.end()-1 )
                        DownloadIter = Download.begin();
                else
                        ++DownloadIter;
        }
}


void DrawImage(int width, int height)
{
	SwitchImageByTimer();

	int offsetx = width - ( width/2);
	int offsety =  height - (height/2);
	switch(SelectedImage)
	{
		case 0:
			DrawImageAtFit(offsetx, offsety  , width/2, height/2, Joystick);
		break;
		case 1:
			DrawImageAtFit(offsetx, offsety  , width/2, height/2, *AwaitingIter);
		break;

                case 2:
                        DrawImageAtFit(offsetx, offsety  , width/2, height/2, *DownloadIter);
                break;

		default:
			DrawImageAtFit(offsetx, offsety  , width/2, height/2, Joystick);



	}
       
	// DrawImageAtFit(offsetx, offsety  , width/2, height/2, *Current);                                    // Draw stretched across entire screen
}

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
	Background(5, 5, 5);
	DrawImage(width, height);
	DrawText(width, height, 65);

	End();
}

void InitImages()
{
	SelectedImage = 0;
	//Awaiting0
	Awaiting.push_back(CreateImageFromJpeg(Awaiting0Path) );
	Awaiting.push_back(CreateImageFromJpeg(Awaiting1Path) );
	Awaiting.push_back(CreateImageFromJpeg(Awaiting2Path) );
	Awaiting.push_back(CreateImageFromJpeg(Awaiting3Path) );
        Awaiting.push_back(CreateImageFromJpeg(Awaiting4Path) );
        Awaiting.push_back(CreateImageFromJpeg(Awaiting5Path) );
        Awaiting.push_back(CreateImageFromJpeg(Awaiting6Path) );
        Awaiting.push_back(CreateImageFromJpeg(Awaiting7Path) );

	AwaitingIter = Awaiting.begin();

        Download.push_back(CreateImageFromJpeg(Download0Path) );
        Download.push_back(CreateImageFromJpeg(Download1Path) );
        Download.push_back(CreateImageFromJpeg(Download2Path) );
        Download.push_back(CreateImageFromJpeg(Download3Path) );
        Download.push_back(CreateImageFromJpeg(Download4Path) );
        Download.push_back(CreateImageFromJpeg(Download5Path) );
        Download.push_back(CreateImageFromJpeg(Download6Path) );
        Download.push_back(CreateImageFromJpeg(Download7Path) );
	Download.push_back(CreateImageFromJpeg(Download8Path) );

	DownloadIter = Download.begin();


	Joystick = CreateImageFromJpeg(JoystickPath);



	time(&TimerLastSwitchAwaiting);
	time(&TimerLastSwitchDownloading);
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
					SelectedImage  = 1;
				}

				if( strncmp(&InBuffer[0],&InMsgJoystick[0],16) ==0 )
                                {
                                        SelectedImage  = 0;
                                }

				if( strncmp(&InBuffer[0],&InMsgDownload[0],16) ==0 )
                                {
                                        SelectedImage  = 2;
                                }
			}

		} 

	}while(EndThread == 0);


	return NULL;
}

pthread_t hUDPThread=0;

int main(int argc, char *argv[]) {
	int width, height;
	ScreenBuf.push_back("SSync screen started");

	init(&width, &height);
	InitImages();


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
