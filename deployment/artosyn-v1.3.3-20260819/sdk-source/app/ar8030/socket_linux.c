#include "socketfd_port.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>

int set_recv_timeout(SOCKETFD sockfd, int ms)
{
    // LINUX
    struct timeval tv;
    tv.tv_sec  = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    return 0;
}

int connect_timeout(SOCKETFD connectSOCK, struct sockaddr_in* addrsrv, int timeoutms)
{
    int flg;

    // Set socket to non-blocking mode
    flg = fcntl(connectSOCK, F_GETFL, NULL);
    if (fcntl(connectSOCK, F_SETFL, flg | O_NONBLOCK) == -1) {
        fprintf(stderr, "Failed to set non-blocking mode: %s\n", strerror(errno));
        return -1;
    }

    int ret = connect(connectSOCK, (struct sockaddr*)addrsrv, sizeof(struct sockaddr));
    if (ret < 0) {
        if (errno == EINPROGRESS) {  // Connection in progress
            // Create epoll instance
            int epoll_fd = epoll_create1(0);
            if (epoll_fd == -1) {
                fprintf(stderr, "epoll_create failed: %s\n", strerror(errno));
                return -1;
            }

            // Configure epoll event: monitor write events
            struct epoll_event ev;
            struct epoll_event events[1];  // To store returned events
            ev.events = EPOLLOUT;          // Trigger after connect
            ev.data.fd = connectSOCK;

            // Add socket to epoll monitoring
            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connectSOCK, &ev) == -1) {
                fprintf(stderr, "epoll_ctl add failed: %s\n", strerror(errno));
                close(epoll_fd);
                return -1;
            }

            // Wait for event or timeout
            int epoll_ret = epoll_wait(epoll_fd, events, 1, timeoutms);
            if (epoll_ret < 0 && errno != EINTR) {
                fprintf(stderr, "epoll_wait error: %s\n", strerror(errno));
                close(epoll_fd);
                return -1;
            } else if (epoll_ret > 0) {
                // Check if the event is for our socket
                if (events[0].data.fd == connectSOCK && (events[0].events & EPOLLOUT)) {
                    // Get socket error status to check connection result
                    socklen_t lon = sizeof(int);
                    int valopt;
                    if (getsockopt(connectSOCK, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) {
                        fprintf(stderr, "getsockopt error: %s\n", strerror(errno));
                        close(epoll_fd);
                        return -2;
                    }
                    if (valopt != 0) {
                        fprintf(stderr, "Connection failed: %s\n", strerror(valopt));
                        close(epoll_fd);
                        return -3;
                    }
                }
            } else {  // epoll_ret == 0 means timeout
                fprintf(stderr, "Connection timeout\n");
                close(epoll_fd);
                return -4;
            }

            // Cleanup epoll resources
            close(epoll_fd);
        } else {  // Other errors during connect
            fprintf(stderr, "Connect failed immediately: %s\n", strerror(errno));
            return -1;
        }
    }

    // Restore socket to blocking mode
    flg = fcntl(connectSOCK, F_GETFL, NULL);
    if (fcntl(connectSOCK, F_SETFL, flg & (~O_NONBLOCK)) == -1) {
        fprintf(stderr, "Failed to restore blocking mode: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}