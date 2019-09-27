#include <limits.h>
#include <netdb.h>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <vector>

#include <pthread.h>

#include  "AddressManager.h"



#define BUFLEN 1500 
#define SERVER "192.168.2.20"

int PORT_COMMAND = 9120;
int PORT = 5621;
int PORTOUT = 5601;

using namespace std;

void *StartSenderThread(void *arg);
static void *SenderThread(void* p);
int StopSenderThread = 0;
int InitVideoReceiverSocket();
int IniCommandReceiverSocket();

struct sockaddr_in si_VideoReceiver;
struct sockaddr_in si_CommandReceiver;
int VideoReceiveSocket;
int CommandReceiverSocket;


int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		fprintf(stderr, "Usage: ./UDPSplitter PORT_COMMAND  PORT_IN PORT_OUT\n");
		fprintf(stderr, "PORT_COMMAND  accept two commands as UDP message:\n");
		fprintf(stderr, "add 192.168.2.20\n");
		fprintf(stderr, "del 192.168.2.20\n");
		fprintf(stderr, "Usage example: ./UDPSplitter 9120 5621 5601\n");
		exit(1);
		
	}
	
	PORT_COMMAND=atoi(argv[1]);
	PORT=atoi(argv[2]);
	PORTOUT=atoi(argv[3]);
	//fprintf(stderr, "\n");
	StopSenderThread = 0;
	AddressManager addressManager;
	addressManager.SetUDPForwardPort(PORTOUT);
	//addressManager.AddItem("192.168.2.20");
	//addressManager.AddItem("192.168.2.15");
	
	if (InitVideoReceiverSocket() != 0)
		exit(1);
	if (IniCommandReceiverSocket() != 0)
		exit(1);
	
	//std::thread t(StartSenderThread, &addressManager);
	pthread_t hSenderThread = NULL;  
	pthread_create(&hSenderThread, NULL, StartSenderThread, (void *)&addressManager); 

	
	char CommandBuffer[40];
	int CommandReceiveLen = 0;
	while (1)
	{
		if ((CommandReceiveLen = recvfrom(CommandReceiverSocket, CommandBuffer, 40, 0, NULL, NULL)) != -1)
		{
			
			if (CommandReceiveLen > 10)
			{
				
				fprintf(stderr, "Received message: %s", CommandBuffer);
				
				char *TempCommand = new char[3]();
				memcpy(TempCommand, &CommandBuffer[0], 3);
				
				char *TempIP = new char[CommandReceiveLen - 4]();
				memcpy(TempIP, &CommandBuffer[4], CommandReceiveLen - 4);
				
				
				if (strncmp(TempCommand, "add", 3) == 0)
				{
					StopSenderThread = 1;
					pthread_join(hSenderThread, NULL);
					StopSenderThread = 0;
					
						
					addressManager.AddItem(TempIP);
					pthread_create(&hSenderThread, NULL, StartSenderThread, (void *)&addressManager); 
					
					
				}
				
				if (strncmp(TempCommand, "del", 3) == 0)
				{
					StopSenderThread = 1;
					pthread_join(hSenderThread, NULL);
					StopSenderThread = 0;
						
					addressManager.DelItem(TempIP);
					pthread_create(&hSenderThread, NULL, StartSenderThread, (void *)&addressManager); 
				}
				
				
				
				delete[] TempIP;
				delete[] TempCommand;
			}
		}
	}
	

	
	
	
	//Receiver



	
	
	
	
	return 0;
}


void *StartSenderThread(void *arg) 
{
	AddressManager *addressManager = (AddressManager *)arg;
	socklen_t  recv_len;
	int i = 0;
	char buf[BUFLEN];
	
	int VectorSize = addressManager->AddressInfoVector.size();
	
	while (StopSenderThread == 0)
	{
		if ((recv_len = recvfrom(VideoReceiveSocket, buf, BUFLEN, 0, NULL,NULL)) == -1)
		{
			
		}
		else
		{
			for (i = 0; i < VectorSize; i++)
			{
				
				if (sendto(addressManager->AddressInfoVector[i].Socket,
					buf,
					recv_len,
					0, 
					(struct sockaddr *) &addressManager->AddressInfoVector[i].si_DestInfo,
					addressManager->AddressInfoVector[i].si_len) == -1)
				{
					fprintf(stderr, "sendto() error");
					exit(1);
				}
			}
			
		}
		
	}
	
}

int InitVideoReceiverSocket()
{
	
	//create a UDP socket
	if((VideoReceiveSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		return 1;
	}
	
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 250000;
	if (setsockopt(VideoReceiveSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		return 1;
	}
     
	// zero out the structure
	memset((char *) &si_VideoReceiver, 0, sizeof(si_VideoReceiver));
     
	si_VideoReceiver.sin_family = AF_INET;
	si_VideoReceiver.sin_port = htons(PORT);
	si_VideoReceiver.sin_addr.s_addr = htonl(INADDR_ANY);
     
	//bind socket to port
	if(bind(VideoReceiveSocket, (struct sockaddr*)&si_VideoReceiver, sizeof(si_VideoReceiver)) == -1)
	{
		return 1;
	} 
	return 0;
}

int IniCommandReceiverSocket()
{
	
	//create a UDP socket
	if((CommandReceiverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		return 1;
	}
	 
	// zero out the structure
	memset((char *) &si_CommandReceiver, 0, sizeof(si_CommandReceiver));
     
	si_CommandReceiver.sin_family = AF_INET;
	si_CommandReceiver.sin_port = htons(PORT_COMMAND);
	si_CommandReceiver.sin_addr.s_addr = htonl(INADDR_ANY);
     
	//bind socket to port
	if(bind(CommandReceiverSocket, (struct sockaddr*)&si_CommandReceiver, sizeof(si_CommandReceiver)) == -1)
	{
		return 1;
	} 
	return 0;
}