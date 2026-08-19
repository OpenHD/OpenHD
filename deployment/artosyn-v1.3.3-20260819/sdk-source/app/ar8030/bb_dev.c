#include "bb_dev.h"
#include "ar8030.h"
#include "session.h"
#include "session_callback.h"
#include "session_hotplug.h"
#include "socket_fd.h"
#include "socketfd_port.h"
#include "usbpack.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int set_recv_timeout(SOCKETFD sockfd, int ms);
int create_tcp_connect(const char* addr, int port, SOCKETFD* listenfd);

int get_workid_list(SOCKETFD sockfd, uint32_t* ids)
{
    uint8_t buff[1024];

    usbpack pack = {
        .datalen  = 0,
        .datapack = 0,
        .msgid    = 0,
        .reqid    = BB_RPC_GET_LIST,
        .sta      = 0,
    };
    int sdlen = make_usbpack2buff(buff, sizeof(buff), &pack);

    send(sockfd, (char*)buff, sdlen, 0);

    int recvlen = recv(sockfd, (char*)buff, sizeof(buff), 0);

    if (recvlen <= 0) {
        return -1;
    }

    int ret = unpack_usb_pack(buff, recvlen, 0, &pack);
    if (ret < 0) {
        return -2;
    }

    memcpy(ids, pack.datapack, pack.datalen);

    return pack.sta;
}

static int server_test(SOCKETFD sockfd, int timeout_in_ms)
{
    uint8_t buff[128];
    usbpack pack = {
        .data_v  = NULL,
        .datalen = 0,
        .reqid   = BB_RPC_TEST,
        .sta     = 0,
        .msgid   = 0,
    };

    set_recv_timeout(sockfd, timeout_in_ms);

    int sdlen = make_usbpack2buff(buff, sizeof(buff), &pack);

    send(sockfd, (char*)buff, sdlen, 0);

    int recvlen = recv(sockfd, (char*)buff, sizeof(buff), 0);
    if (recvlen < 0) {
        printf("recv timeout\n");
        return -1;
    }

    usbpack retpack;
    int     ret = unpack_usb_pack(buff, recvlen, 0, &retpack);
    if (ret) {
        printf("data err\n");
        return -1;
    }
    return 0;
}

AR8030_API int bb_host_connect(bb_host_t** devs, const char* addr, int port)
{
    bb_host_t* phost = malloc(sizeof(bb_host_t));
#ifdef WIN32
    strcpy_s(phost->remote_addr, sizeof(phost->remote_addr), addr);
#ifdef ENABLE_UDS
    strcpy_s(phost->uds_addr, sizeof(phost->uds_addr), "./1");
#endif
#else
    strcpy(phost->remote_addr, addr);
#ifdef ENABLE_UDS
    strcpy(phost->uds_addr, "./1");
#endif
#endif
    phost->port = port;

    INIT_LIST_HEAD(&phost->open_handle_list);

    INIT_LIST_HEAD(&phost->hotplug_change_cb);
    pthread_rwlock_init(&phost->hotplug_change_lk, NULL);

    SOCKETFD sockfd;
    int      ret = create_tcp_connect(addr, port, &sockfd);
    if (ret) {
        printf("connect failed\n");
        free(phost);
        return -1;
    }

    ret = server_test(sockfd, 100);
    socke_close(sockfd);
    if (ret) {
        free(phost);
        return -2;
    }

    *devs = phost;
    return 0;
}

AR8030_API int bb_host_connect_test(const char* addr, int port)
{
    SOCKETFD sockfd;
    int      ret = create_tcp_connect(addr, port, &sockfd);
    if (ret) {
        printf("connect failed\n");
        return -1;
    }

    ret = server_test(sockfd, 100);
    socke_close(sockfd);
    if (ret) {
        return -2;
    }

    return 0;
}

AR8030_API int bb_dev_freelist(bb_dev_list_t* plist)
{
    if (plist) {
        free(container_of(plist, struct bb_dev_list, dev_list));
        return 0;
    }
    return -1;
}

static int get_remote_mac(uint32_t workid, SOCKETFD sockfd, bb_dev_info_t* dev_info)
{
    uint8_t buff[1024];
    usbpack pack = {
        .datalen  = 0,
        .datapack = 0,
        .msgid    = workid,
        .reqid    = BB_RPC_GET_MAC,
        .sta      = 0,
    };

    int sdlen = make_usbpack2buff(buff, sizeof(buff), &pack);

    send(sockfd, (char*)buff, sdlen, 0);

    int recvlen = recv(sockfd, (char*)buff, sizeof(buff), 0);

    if (recvlen <= 0) {
        return -1;
    }

    int ret = unpack_usb_pack(buff, recvlen, 0, &pack);
    if (ret < 0) {
        return -2;
    }

    if (pack.datalen > 0) {
        memcpy(dev_info->mac, pack.datapack, pack.datalen);
        dev_info->mac[pack.datalen] = 0;
        dev_info->maclen            = pack.datalen;
    } else {
        dev_info->maclen = 0;
    }

    return 0;
}

AR8030_API int bb_dev_getinfo(bb_dev_t* pdev, bb_dev_info_t* dev_info)
{
    SOCKETFD sockfd;
    int      ret = create_tcp_connect(pdev->phost->remote_addr, pdev->phost->port, &sockfd);
    if (ret) {
        printf("connect failed\n");
        return -1;
    }
    set_recv_timeout(sockfd, 100);

    int len = get_remote_mac(pdev->id, sockfd, dev_info);
    socke_close(sockfd);

    if (len < 0) {
        return -1;
    }

    return 0;
}

AR8030_API int bb_host_disconnect(bb_host_t* phost)
{
    while (!list_empty(&phost->open_handle_list)) {
        bb_dev_handle_t* op_dev = list_first_entry(&phost->open_handle_list, bb_dev_handle_t, bb_dev_handle_list);
        bb_dev_close(op_dev);
    }

    bb_dev_del_all_hotplug_cb(phost);
    pthread_rwlock_destroy(&phost->hotplug_change_lk);

    free(phost);

    return 0;
}

static int bb_dev_list_getnum(bb_dev_list_t* devs)
{
    struct bb_dev_list* list0 = container_of(devs, struct bb_dev_list, dev_list);
    return list0->dev_num;
}

AR8030_API bb_dev_t* bb_dev_getlist_index(bb_dev_list_t* plist, int index)
{
    if (!plist || index < 0) {
        return NULL;
    }

    int maxnum = bb_dev_list_getnum(plist);
    if (index < maxnum) {
        return plist[index];
    }
    return NULL;
}

AR8030_API int bb_dev_getlist(bb_host_t* phost, bb_dev_list_t** devs)
{
    SOCKETFD sockfd;
    int      ret = create_tcp_connect(phost->remote_addr, phost->port, &sockfd);
    if (ret) {
        printf("connect failed\n");
        return -1;
    }
    set_recv_timeout(sockfd, 100);

    uint32_t tmpdev[100];
    int      sz = get_workid_list(sockfd, tmpdev);
    socke_close(sockfd);

    if (sz <= 0) {
        return sz;
    }

    struct bb_dev_list* tmplist = malloc(sizeof(struct bb_dev_list) + sz * sizeof(bb_dev_tmp_t));

    tmplist->dev_num   = sz;
    bb_dev_t** devlist = (bb_dev_t**)tmplist->dev_list;
    bb_dev_t*  realdat = (bb_dev_t*)(((char*)devlist) + sz * sizeof(void*));

    for (int i = 0; i < sz; i++) {
        devlist[i] = &realdat[i];

        realdat[i].id    = tmpdev[i];
        realdat[i].phost = phost;
    }

    *devs = devlist;
    return sz;
}

static int test_workid(bb_host_t* phost, uint32_t workid)
{
    SOCKETFD sockfd;
    int      ret = create_tcp_connect(phost->remote_addr, phost->port, &sockfd);
    if (ret) {
        printf("connect failed\n");
        return -1;
    }
    set_recv_timeout(sockfd, 100);

    uint8_t buff[1024];
    usbpack pack = {
        .datalen  = 0,
        .datapack = 0,
        .msgid    = workid,
        .reqid    = BB_RPC_SEL_ID,
        .sta      = 0,
    };
    int sdlen = make_usbpack2buff(buff, sizeof(buff), &pack);

    send(sockfd, (char*)buff, sdlen, 0);

    int recvlen = recv(sockfd, (char*)buff, sizeof(buff), 0);

    if (recvlen <= 0) {
        return -1;
    }

    ret = unpack_usb_pack(buff, recvlen, 0, &pack);
    if (ret < 0) {
        return -2;
    }

    if (pack.sta != 0 && pack.reqid != BB_RPC_SEL_ID) {
        return -3;
    }

    socke_close(sockfd);

    return 0;
}

bb_dev_handle_t* bb_dev_open(bb_dev_t* pdev)
{
    int ret = test_workid(pdev->phost, pdev->id);
    if (ret) {
        return NULL;
    }

    bb_dev_handle_t* pdh = malloc(sizeof(bb_dev_handle_t));

    pdh->sel_id = pdev->id;
    pdh->phost  = pdev->phost;

    pdh->ioctl_sess = NULL;

    pthread_mutex_init(&pdh->cbmtx, NULL);
    pthread_cond_init(&pdh->cbcv, NULL);
    pthread_mutex_init(&pdh->ioctl_lk, NULL);

    INIT_LIST_HEAD(&pdh->cblshead);

    list_add(&pdh->bb_dev_handle_list, &pdh->phost->open_handle_list);

    return pdh;
}

CB_SESSION* dev_find_node(bb_dev_handle_t* pdev, cb_chk chk, int evtid)
{
    CB_SESSION* ret = NULL;
    pthread_mutex_lock(&pdev->cbmtx);

    struct list_head* phead;
    list_for_each (phead, &pdev->cblshead) {
        CB_SESSION* pcb = get_cb_from_node(phead);
        if (chk(pcb, &evtid)) {
            ret = pcb;
            break;
        }
    }

    pthread_mutex_unlock(&pdev->cbmtx);
    return ret;
}

void dev_insert_session(bb_dev_handle_t* pdev, struct CB_SESSION* cbsess)
{
    pthread_mutex_lock(&pdev->cbmtx);

    list_add(get_node_from_cb(cbsess), &pdev->cblshead);
    pthread_mutex_unlock(&pdev->cbmtx);
}

void dev_del_nod(bb_dev_handle_t* pdev, cb_chk chk, struct CB_SESSION* pcb)
{
    pthread_mutex_lock(&pdev->cbmtx);

    for (struct list_head* test = pdev->cblshead.next; test != &pdev->cblshead;) {
        CB_SESSION* ptest = get_cb_from_node(test);
        if (chk(ptest, pcb)) {
            struct list_head* tmp = test;
            test                  = test->next;
            list_del(tmp);
        } else {
            test = test->next;
        }
    }

    pthread_cond_broadcast(&pdev->cbcv);
    pthread_mutex_unlock(&pdev->cbmtx);
}

static void cb_trig_all_close(bb_dev_handle_t* pdev)
{
    pthread_mutex_lock(&pdev->cbmtx);

    struct list_head* phead;
    list_for_each (phead, &pdev->cblshead) {
        BASE_SESSION* pcb = get_bs_from_node(phead);
        bs_trig_close(pcb);
    }

    while (!list_empty(&pdev->cblshead)) {
        pthread_cond_wait(&pdev->cbcv, &pdev->cbmtx);
    }

    pthread_mutex_unlock(&pdev->cbmtx);
}

AR8030_API int bb_dev_close(bb_dev_handle_t* pdev)
{
    if (!pdev) {
        return -1;
    }

    if (pdev->ioctl_sess) {
        BASE_SESSION* base = (BASE_SESSION*)pdev->ioctl_sess;
        bs_trig_close(base);
        pdev->ioctl_sess = NULL;
    }

    cb_trig_all_close(pdev);

    socket_del_dev(pdev);
    pthread_mutex_destroy(&pdev->cbmtx);
    pthread_cond_destroy(&pdev->cbcv);
    pthread_mutex_destroy(&pdev->ioctl_lk);

    list_del(&pdev->bb_dev_handle_list);

    free(pdev);
    return 0;
}
