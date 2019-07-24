#pragma once


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

struct AddressInfo
{
	int Socket;
	struct sockaddr_in si_DestInfo;
	int si_len;
		
};