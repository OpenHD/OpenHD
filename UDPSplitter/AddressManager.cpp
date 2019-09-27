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


#include "AddressManager.h"
#include "AddressInfo.h"


using namespace std;

void AddressManager::AddItem(char *IP)
{
	if (FindIP(IP) == -1)//No need to add it second time
	{
		AddressInfo tmp;
		tmp.si_len = sizeof(tmp.si_DestInfo);
			
		if ((tmp.Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		{
			fprintf(stderr, "Failed to create socket");
			//exit(1);
		}
			
		memset((char *) &tmp.si_DestInfo, 0, sizeof(tmp.si_DestInfo));
		tmp.si_DestInfo.sin_family = AF_INET;
		tmp.si_DestInfo.sin_port = htons(UDPForwardPort);

		if (inet_aton(IP, &tmp.si_DestInfo.sin_addr) == 0)
		{
			fprintf(stderr, "inet_aton() failed\n");
			//exit(1);
		}
		//Add info to vector
			
			
			
		AddressInfoVector.push_back(tmp);
	}
	else
	{
		fprintf(stderr, "Address exists. No need to add second time.");
	}
	
}

void AddressManager::DelItem(char *IP)
{
	int Position = FindIP(IP);
	if (Position != -1)
	{
		close(AddressInfoVector[Position].Socket);//close socket
		AddressInfoVector.erase(AddressInfoVector.begin() + Position);	
	}
	else
	{
		fprintf(stderr, "Address not exists. No need to remove it.");
	}
	
}

int AddressManager::FindIP(char *IP)
{
	int result = -1;
	if (AddressInfoVector.empty() == false)
	{
			
		struct in_addr inAddress;
		if (inet_aton(IP, &inAddress) == true)
		{
			
			int size = AddressInfoVector.size();	
			for (int i = 0; i < size; i++)
			{
				if (AddressInfoVector[i].si_DestInfo.sin_addr.s_addr == inAddress.s_addr)
				{
					return i;
				}
			}
		}
	}
		
	return result;
}

void AddressManager::SetUDPForwardPort(int port)
{
	UDPForwardPort = port;
}