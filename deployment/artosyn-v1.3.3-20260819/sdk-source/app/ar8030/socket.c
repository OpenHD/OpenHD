#include "socketfd_port.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int connect_timeout(SOCKETFD connectSOCK, struct sockaddr_in* addrsrv, int timeoutms);

int create_tcp_connect(const char* addr, int port, SOCKETFD* listenfd)
{
    SOCKETFD connectSOCK = -1;
#ifdef WIN32
    WSADATA wsaData;
    int     iResult;
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
#endif

    // Create a SOCKET for the server to listen for client connections.
    connectSOCK = socket(AF_INET, SOCK_STREAM, 0);
    if (connectSOCK == -1) {
#ifdef WIN32
        printf("socket failed with error: %d\n", (int)WSAGetLastError());
        WSACleanup();
#else
        printf("socket failed with error: %d\n", errno);
#endif
        return 2;
    }
    // Setup the TCP connect
    struct sockaddr_in addrsrv;
    addrsrv.sin_addr.s_addr = inet_addr(addr);
    addrsrv.sin_family      = AF_INET;
    addrsrv.sin_port        = htons(port);

    int flg = 1;
    setsockopt(connectSOCK, IPPROTO_TCP, TCP_NODELAY, (char*)&flg, sizeof(flg));

    int ret = connect_timeout(connectSOCK, &addrsrv, 200);

    if (ret) {
        return ret;
    }

    *listenfd = connectSOCK;
    return 0;
}
