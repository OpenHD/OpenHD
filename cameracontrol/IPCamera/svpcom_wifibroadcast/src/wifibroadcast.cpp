// -*- C++ -*-
//
#include <stdlib.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <memory>

#include "wifibroadcast.hpp"

string string_format(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    size_t size = vsnprintf(nullptr, 0, format, args) + 1; // Extra space for '\0'
    va_end(args);
    unique_ptr<char[]> buf(new char[size]);
    va_start(args, format);
    vsnprintf(buf.get(), size, format, args);
    va_end(args);
    return string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

uint64_t get_time_ms(void) // in milliseconds
{
    struct timespec ts;
    int rc = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (rc < 0) throw runtime_error(string_format("Error getting time: %s", strerror(errno)));
    return ts.tv_sec * 1000LL + ts.tv_nsec / 1000000;
}

int open_udp_socket_for_rx(int port)
{
    struct sockaddr_in saddr;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) throw runtime_error(string_format("Error opening socket: %s", strerror(errno)));

    int optval = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    bzero((char *) &saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons((unsigned short)port);

    if (bind(fd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0)
    {
        throw runtime_error(string_format("Bind error: %s", strerror(errno)));
    }
    return fd;
}
