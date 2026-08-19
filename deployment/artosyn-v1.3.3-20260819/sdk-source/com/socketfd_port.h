#ifndef __SOCKETFD_PORT_H__
#define __SOCKETFD_PORT_H__

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef ENABLE_UDS
#include <afunix.h>
#endif
typedef SOCKET SOCKETFD;
#define AR8030_API __declspec(dllexport)

#else
#include <sys/socket.h>
#ifdef ENABLE_UDS
#include <sys/un.h>
#endif
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/tcp.h>
typedef int SOCKETFD;
#define AR8030_API
#endif

void socke_close(SOCKETFD sock);

#endif