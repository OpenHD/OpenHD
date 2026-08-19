#include "ar8030.h"
#include "bb_dev.h"
#include "com_cfg.h"
#include "session.h"
#include "session_callback.h"
#include "session_ioctl.h"
#include "socketfd_port.h"
#include "usbpack.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

usbpack make_usb_pack(uint32_t reqid, uint8_t* data, uint32_t len)
{
    usbpack pack = {
        .reqid   = reqid,
        .data_v  = data,
        .datalen = len,
    };

    return pack;
}

#define MAX_TXBUF (SOCK_LEN_APP_TO_DAEMON)

int max_payload(void)
{
    return (MAX_TXBUF - get_fix_usblen());
}

typedef struct BB_HANDLE {
    SOCKETFD rpcfd;

    int bg_running_flg;

    pthread_t pbg;

    BASE_SESSION* session;

    uint32_t workid;

    int initflg;

    char txbuff[MAX_TXBUF];

    pthread_mutex_t handle_lk;
} BB_HANDLE;

#define RX_BUF_SZ (SOCK_LEN_DAEMON_TO_APP * 2)

void* bb_read_thread(void* p)
{
    BB_HANDLE* phandle = (BB_HANDLE*)p;

    unsigned char* rd_buff = malloc(RX_BUF_SZ);
    int            bufflen = 0;

    while (rd_buff) {
        int rdlen = 0;

        pthread_mutex_lock(&phandle->handle_lk);
        if (!phandle->bg_running_flg) {
            break;
        }
        pthread_mutex_unlock(&phandle->handle_lk);

        rdlen = recv(phandle->rpcfd, (char*)rd_buff + bufflen, RX_BUF_SZ - bufflen, 0);
        if (rdlen == 0) {
#ifdef WIN32
            printf("%I64d Connection closing...\n", phandle->rpcfd);
#else
            printf("%d Connection closing...\n", phandle->rpcfd);
#endif
            break;
        } else if (rdlen < 0) {
#ifdef WIN32
            printf("%I64d shutdown failed with error: %d\n", phandle->rpcfd, WSAGetLastError());
#else
            printf("%d shutdown failed with error: %s\n", phandle->rpcfd, strerror(errno));
#endif
            break;
        }
        bufflen += rdlen;
        // unpack buffer

        int usedlen = 0;
        while (1) {
            int     curusedlen;
            usbpack pack;
            int     ret = unpack_usb_pack(rd_buff + usedlen, bufflen - usedlen, &curusedlen, &pack);

            if (ret < 0) {
                break;
            }

            if (pack.reqid == BB_RPC_SEL_ID) {
                pthread_mutex_lock(&phandle->handle_lk);
                if (phandle->rpcfd >= 0) {
                    SOCKETFD fd = phandle->rpcfd;
                    phandle->rpcfd = -1;
                    socke_close(fd);
                }
                pthread_mutex_unlock(&phandle->handle_lk);
                printf("workid not found\n");
                break;
            }

            if (phandle->session) {
                phandle->session->fun->rdcb(phandle, &pack, phandle->session);
            }

            usedlen += curusedlen;
        }
        // update buffer
        if (usedlen >= bufflen) {
            bufflen = 0;
        } else {
            int leftcnt = bufflen - usedlen;

            // buff 数据过多 去除一半
            if (leftcnt > (RX_BUF_SZ - 1024)) {
                leftcnt -= (RX_BUF_SZ / 2);
            }

            memmove(rd_buff, rd_buff + usedlen, leftcnt);
            bufflen = leftcnt;
        }
    }
    free(rd_buff);
    pthread_mutex_lock(&phandle->handle_lk);
    if (phandle->rpcfd >= 0) {
        SOCKETFD fd = phandle->rpcfd;
        phandle->rpcfd = -1;
        socke_close(fd);
        printf("fd %d Connection closed\n", fd);
    }
    if (phandle->session) {
        if (phandle->session->fun->de_init) {
            phandle->session->fun->de_init(phandle->session);
        }
        phandle->session = NULL;
    }
    // 由于远程接口关闭 自动释放
    if (phandle->bg_running_flg) {
        phandle->bg_running_flg = 0;
        pthread_mutex_unlock(&phandle->handle_lk);
        pthread_detach(phandle->pbg);
        free(phandle);
    }
    else {
        pthread_mutex_unlock(&phandle->handle_lk);
    }

    return NULL;
}

int create_tcp_connect(const char* addr, int port, SOCKETFD* listenfd);

void bb_set_new_session(BB_HANDLE* pdev, BASE_SESSION* psess)
{
    pdev->session = psess;
}

BASE_SESSION* bb_get_session(BB_HANDLE* phd)
{
    return phd->session;
}

int send_usbpack(BB_HANDLE* pdev, usbpack* pack)
{
    pack->msgid = pdev->workid;

    int sdlen = make_usbpack2buff((unsigned char*)pdev->txbuff, MAX_TXBUF, pack);

    int ret = send(pdev->rpcfd, pdev->txbuff, sdlen, 0);
    if (ret <= 0) {
        printf("send error = %d\n", ret);
    }

    return ret;
}

BB_HANDLE* bb_gethandle_from_host(bb_host_t* phost)
{
    SOCKETFD sockfd;
    int      ret = create_tcp_connect(phost->remote_addr, phost->port, &sockfd);
    if (ret) {
        printf("connect failed\n");
        return NULL;
    }

    BB_HANDLE* phandle = (BB_HANDLE*)malloc(sizeof(BB_HANDLE));

    phandle->rpcfd          = sockfd;
    phandle->bg_running_flg = 1;
    phandle->session        = NULL;
    phandle->initflg        = 0;
    phandle->workid         = 0;
    pthread_mutex_init(&phandle->handle_lk, NULL);
    pthread_create(&phandle->pbg, NULL, bb_read_thread, phandle);

    return phandle;
}

BB_HANDLE* bb_gethandle(bb_dev_handle_t* pdh)
{
    BB_HANDLE* phandle = bb_gethandle_from_host(pdh->phost);

    if (phandle) {
        phandle->workid = pdh->sel_id;
    }
    return phandle;
}

void bs_trig_close(BASE_SESSION* psess)
{
    if (psess && psess->phd) {
        BB_HANDLE* phd = psess->phd;
        SOCKETFD fd = phd->rpcfd;

        pthread_mutex_lock(&phd->handle_lk);
        if (phd->bg_running_flg) {
            if (fd >= 0) {
                socke_close(fd);
                phd->rpcfd = -1;
            }
            phd->bg_running_flg = 0;
        }
        pthread_mutex_unlock(&phd->handle_lk);
    }
}

static int _ioctl_bb_ioctl(bb_dev_handle_t* pdev, uint32_t request, const void* input, void* output, int timeout)
{
    int ret = -1;
    pthread_mutex_lock(&pdev->ioctl_lk);
    ret = ioctl_bb_ioctl(pdev, request, input, output, timeout);
    pthread_mutex_unlock(&pdev->ioctl_lk);
    return ret;
}

AR8030_API int bb_ioctl(bb_dev_handle_t* pdh, uint32_t request, const void* input, void* output)
{
    if (!pdh) {
        return -1;
    }

    if (request == BB_SET_EVENT_SUBSCRIBE || request == BB_SET_EVENT_UNSUBSCRIBE) {
        return cb_bb_ioctl(pdh, request, input, -1);
    }

    return _ioctl_bb_ioctl(pdh, request, input, output, -1);
}

AR8030_API int bb_ioctl_ex(bb_dev_handle_t* pdh, uint32_t request, const void* input, void* output, int timeout)
{
    if (!pdh) {
        return -1;
    }

    if (request == BB_SET_EVENT_SUBSCRIBE || request == BB_SET_EVENT_UNSUBSCRIBE) {
        return cb_bb_ioctl(pdh, request, input, timeout);
    }

    return _ioctl_bb_ioctl(pdh, request, input, output, timeout);
}

AR8030_API int bb_start(bb_dev_handle_t* pdh)
{
    return bb_ioctl(pdh, BB_START_REQ, NULL, NULL);
}

AR8030_API int bb_stop(bb_dev_handle_t* pdh)
{
    return bb_ioctl(pdh, BB_STOP_REQ, NULL, NULL);
}

AR8030_API int bb_init(bb_dev_handle_t* pdh)
{
    return bb_ioctl(pdh, BB_INIT_REQ, NULL, NULL);
}

AR8030_API int bb_deinit(bb_dev_handle_t* pdh)
{
    return bb_ioctl(pdh, BB_DEINIT_REQ, NULL, NULL);
}
