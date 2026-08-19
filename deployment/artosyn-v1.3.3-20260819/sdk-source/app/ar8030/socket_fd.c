#include "socket_fd.h"
#include "ar8030.h"
#include "bb_dev.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int              fd;
    SOCK_SESSION*    session;
    bb_dev_handle_t* pdev;
} FDTAB;
static pthread_rwlock_t fdlock;

FDTAB*     fd_head = NULL;
static int nxtfd;
static int fdcnt;
static int fdmax;

void socket_session_init(void)
{
    if (fd_head) {
        return;
    }

    pthread_rwlock_init(&fdlock, NULL);
    fdcnt = 0;
    fdmax = 10;

    fd_head = malloc(sizeof(FDTAB) * fdmax);
}

int socket_add_fd(bb_dev_handle_t* pdev, SOCK_SESSION* sock)
{
    socket_session_init();

    if (!fd_head || !pdev || !sock) {
        return -1;
    }

    pthread_rwlock_wrlock(&fdlock);
    int fdnum = nxtfd++;

    if (fdcnt >= fdmax) {
        fdmax *= 2;
        fd_head = realloc(fd_head, sizeof(FDTAB) * fdmax);
    }
    fd_head[fdcnt].fd      = fdnum;
    fd_head[fdcnt].session = sock;
    fd_head[fdcnt].pdev    = pdev;

    fdcnt++;
    pthread_rwlock_unlock(&fdlock);
    return fdnum;
}

SOCK_SESSION* socket_get_session(int fd)
{
    socket_session_init();

    if (!fd_head || fd < 0) {
        return NULL;
    }

    pthread_rwlock_rdlock(&fdlock);
    SOCK_SESSION* retsock = NULL;
    for (int i = 0; i < fdcnt; i++) {
        if (fd_head[i].fd == fd) {
            retsock = fd_head[i].session;
            break;
        }
    }
    pthread_rwlock_unlock(&fdlock);
    return retsock;
}

void socket_del_fd(int fd)
{
    socket_session_init();

    if (!fd_head || fd < 0) {
        return;
    }

    pthread_rwlock_wrlock(&fdlock);
    for (int i = 0; i < fdcnt; i++) {
        if (fd_head[i].fd == fd) {
            int sz = (fdcnt - i - 1) * sizeof(FDTAB);
            if (sz) {
                memmove(fd_head + i, fd_head + i + 1, sz);
            }
            fdcnt--;
            break;
        }
    }
    pthread_rwlock_unlock(&fdlock);
}

void socket_del_dev(bb_dev_handle_t* pdev)
{
    socket_session_init();

    if (!fd_head || !pdev || !fdcnt) {
        return;
    }

    pthread_rwlock_rdlock(&fdlock);
    int    cnt    = fdcnt;
    FDTAB* fd_tmp = malloc(sizeof(FDTAB) * cnt);
    memcpy(fd_tmp, fd_head, sizeof(FDTAB) * cnt);
    pthread_rwlock_unlock(&fdlock);

    for (int i = 0; i < cnt; i++) {
        if (fd_tmp[i].pdev == pdev) {
            bb_socket_close(fd_tmp[i].fd);
        }
    }
    free(fd_tmp);
}
