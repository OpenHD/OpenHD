#include "socketfd_port.h"

void socke_close(SOCKETFD sock)
{
#ifdef WIN32
    shutdown(sock, SD_SEND);
    closesocket(sock);
#else
    shutdown(sock, SHUT_RDWR);
    close(sock);
#endif
}
