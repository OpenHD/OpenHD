#include "rpc_cmdline.h"
#include "base_node.h"
#include "bb_api.h"
#include "com_log.h"
#include "rpc_node.h"
#include "usb_event_list.h"
#include "usbpack.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    struct rpc_node  base;
    struct list_head node;
    void*            drv_ptr;
} cl_rpc;

typedef struct {
    basenode base;
    FILE*    log_fp;
    char     names[128];
    char     recvbuff[10240];

    char sendbuff[1024];
    int  sendlen;

    pthread_mutex_t  mtx;
    struct list_head head;

    int client_plug_event;
} cl_drv;

/**
 * @brief 删除远程节点
 *
 * @param cldrv
 */
static void cldrv_del_rpc(cl_drv* cldrv)
{
    pthread_mutex_lock(&cldrv->mtx);
    while (!list_empty(&cldrv->head)) {
        cl_rpc* clrpc  = list_first_entry(&cldrv->head, cl_rpc, node);
        clrpc->drv_ptr = NULL;
        socke_close(clrpc->base.tinfo->fd);
        list_del(&clrpc->node);
    }
    pthread_mutex_unlock(&cldrv->mtx);
}

static int cl_dev_cls(struct basenode* pnod)
{
    cl_drv* cldrv = container_of(pnod, cl_drv, base);

    cldrv_del_rpc(cldrv);

    bn_set_free(&cldrv->base);
    return 0;
}

static char* cl_dev_names(struct basenode* pnod)
{
    cl_drv* cldrv = container_of(pnod, cl_drv, base);
    if (cldrv->names[0] != 0) {
        return cldrv->names;
    }
    return "cl not start";
}

static int cl_deinit(struct basenode* pnod)
{
    cl_drv* cldrv = container_of(pnod, cl_drv, base);
    cldrv_del_rpc(cldrv);

    com_log(COM_CMDLINE, "cl close %s", cldrv->names);
    bnact.dev_deinit(pnod);
    if (cldrv->log_fp) {
        fclose(cldrv->log_fp);
    }
    free(cldrv);
    return 0;
}

static PROC_RET cl_dev_pack_chk(struct basenode* pnod, struct usbpack* pack)
{
    cl_drv* cldrv = container_of(pnod, cl_drv, base);
    if (pack->domainid != BB_REQ_DBG) {
        return proc_nxt;
    }

    return proc_already;
}

static void send_data_to_list(cl_drv* cldrv, struct usbpack* pack)
{
    int len = make_usbpack2buff((unsigned char*)cldrv->recvbuff, sizeof(cldrv->recvbuff), pack);

    pthread_mutex_lock(&cldrv->mtx);
    cl_rpc* clrpc;
    list_for_each_entry (clrpc, &cldrv->head, node, cl_rpc) {
        send(clrpc->base.tinfo->fd, cldrv->recvbuff, len, 0);
    }
    pthread_mutex_unlock(&cldrv->mtx);
}

static int cl_dev_pack_proc(struct basenode* pnod, struct usbpack* pack)
{
    cl_drv* cldrv = container_of(pnod, cl_drv, base);

    if (pack->datalen) {
        if (cldrv->log_fp) {
            fwrite(pack->data_v, sizeof(char), pack->datalen, cldrv->log_fp);
            fflush(cldrv->log_fp);
        }

        send_data_to_list(cldrv, pack);
    }

    return 0;
}

static int cl_need_write(struct basenode* pnod)
{
    cl_drv* cldrv = container_of(pnod, cl_drv, base);

    if (cldrv->client_plug_event) {
        return 1;
    }

    if (cldrv->sendlen) {
        return 1;
    }

    return 0;
}

static int cl_dev_pack_make(struct basenode* pnod, unsigned char* buff, int len)
{
    cl_drv* cldrv = container_of(pnod, cl_drv, base);

    if (cldrv->client_plug_event) {
        uint32_t client_num = 0;

        pthread_mutex_lock(&cldrv->mtx);
        struct list_head* phead;
        list_for_each (phead, &cldrv->head) {
            client_num++;
        }
        pthread_mutex_unlock(&cldrv->mtx);

        usbpack pack = {
            .domainid = BB_REQ_DBG,
            .subcmdid = BB_DBG_CLIENT_CHANGE,
            .datalen  = sizeof(client_num),
            .data_v   = &client_num,
            .msgid    = 0,
            .sta      = 0,
        };
        cldrv->client_plug_event = 0;
        return make_usbpack2buff(buff, len, &pack);
    }

    pthread_mutex_lock(&cldrv->mtx);
    if (cldrv->sendlen) {
        int     sndlen = cldrv->sendlen > 900 ? 900 : cldrv->sendlen;
        usbpack pack   = {
              .domainid = BB_REQ_DBG,
              .subcmdid = BB_DBG_DATA,
              .datalen  = sndlen,
              .data_v   = cldrv->sendbuff,
        };
        int retlen     = make_usbpack2buff(buff, len, &pack);
        cldrv->sendlen = 0;
        pthread_mutex_unlock(&cldrv->mtx);
        return retlen;
    }
    pthread_mutex_unlock(&cldrv->mtx);

    return -1;
}

static baseact cl_act = {
    .dev_cls        = cl_dev_cls,
    .dev_nm         = cl_dev_names,
    .dev_deinit     = cl_deinit,
    .dev_pack_chk   = cl_dev_pack_chk,
    .dev_pack_proc  = cl_dev_pack_proc,
    .dev_write_able = cl_need_write,
    .dev_pack_make  = cl_dev_pack_make,
};

/**
 * @brief 注册rpc调试通道
 *
 * @param plist
 * @param mac
 */
void rpc_debug_reg(void* plist, bb_mac_t mac)
{
    cl_drv* cldrv = (cl_drv*)malloc(sizeof(cl_drv));
    bn_init(&cldrv->base, plist, nod_debug, &cl_act);

    pthread_mutex_init(&cldrv->mtx, NULL);
    INIT_LIST_HEAD(&cldrv->head);

    char* names  = cldrv->names;
    int   offset = 0;

    offset += sprintf(names + offset, "%s/" , DAEMON_LOG_PATH);

    for (int i = 0; i < sizeof(mac); i++) {
        offset += sprintf(names + offset, "%02x", mac.addr[i]);
    }
    offset += sprintf(names + offset, ".log");

    cldrv->log_fp = fopen(names, "w");
    if (!cldrv->log_fp) {
        com_log(COM_CMDLINE, "open log file %s failed ! errno = %d", names, errno);
    } else {
        com_log(COM_CMDLINE, "open log file %s ok", names);
    }
    plist_insert_node(plist, &cldrv->base);

    cldrv->sendlen           = 0;
    cldrv->client_plug_event = 1; /// 注册后要刷新一次
    bn_tx_inotify(&cldrv->base);
}

static int cl_rpc_chk(unsigned char* buff, int len, struct threadinfo* info)
{
    usbpack pack;
    int     ret = unpack_usb_pack(buff, len, 0, &pack);

    if (ret) {
        return RPC_CHK_NXT;
    }

    if (pack.domainid == BB_REQ_DBG) {
        return RPC_CHK_SUC;
    }
    return RPC_CHK_NXT;
}

static int find_id(struct basenode* pnod, void* par)
{
    if (pnod->tp == nod_debug) {
        return 1;
    }
    return 0;
}

static cl_drv* get_cl_dev_node(struct dev_node_list* plist)
{
    pthread_mutex_lock(&plist->mtx);
    // 寻找debug node
    struct basenode* pbn = plist_find_work_node(plist, find_id, NULL);
    pthread_mutex_unlock(&plist->mtx);
    if (pbn) {
        return container_of(pbn, cl_drv, base);
    }
    return NULL;
}

static int cl_cb_read(struct rpc_node* rpc, unsigned char* buff, int len)
{
    cl_rpc* clrpc = container_of(rpc, cl_rpc, base);
    cl_drv* cldrv = (cl_drv*)clrpc->drv_ptr;

    usbpack pack;
    int     ret = unpack_usb_pack(buff, len, 0, &pack);

    if (ret) {
        com_log(COM_CALLBACK_COM, "cb get error msg len = %d", len);
        return -1;
    }

    if (pack.datalen == 0) {
        usbpack ret = {
            .domainid = BB_REQ_DBG,
            .data_v   = NULL,
            .datalen  = 0,
        };
        unsigned char sndbuf[128];
        int           sndlen = make_usbpack2buff(sndbuf, sizeof(sndbuf), &ret);
        send(rpc->tinfo->fd, (char*)sndbuf, sndlen, 0);
    } else if (cldrv) {
        pthread_mutex_lock(&cldrv->mtx);
        memcpy(cldrv->sendbuff + cldrv->sendlen, pack.data_v, pack.datalen);
        cldrv->sendlen += pack.datalen;
        bn_tx_inotify(&cldrv->base);
        pthread_mutex_unlock(&cldrv->mtx);
    }

    return 0;
}

static int cl_rpc_node_end(struct rpc_node* prpc)
{
    if (!prpc) {
        return -1;
    }

    cl_rpc* clrpc = container_of(prpc, cl_rpc, base);
    cl_drv* cldrv = (cl_drv*)clrpc->drv_ptr;

    if (cldrv) {
        pthread_mutex_lock(&cldrv->mtx);
        list_del(&clrpc->node);
        cldrv->client_plug_event = 1;
        bn_tx_inotify(&cldrv->base);
        pthread_mutex_unlock(&cldrv->mtx);
    }

    free(clrpc);
    return 0;
}

static rpc_act cl_rpc_act = {
    .rpc_rd_cb = cl_cb_read,
    .end       = cl_rpc_node_end,
};

static struct rpc_node* cl_rpc_nod_start(struct threadinfo* tinfo)
{
    cl_drv* cldrv = get_cl_dev_node(tinfo->plist);
    if (!cldrv) {
        com_log(COM_CALLBACK_COM, "debug drv not found");
        return NULL;
    }

    cl_rpc* clrpc = (cl_rpc*)malloc(sizeof(cl_rpc));
    if (!clrpc) {
        return NULL;
    }
    clrpc->drv_ptr = cldrv;
    INIT_LIST_HEAD(&clrpc->node);

    rpc_node* prpc = &clrpc->base;
    prpc->tinfo    = tinfo;
    prpc->pact     = &cl_rpc_act;
    prpc->nodetype = nod_debug;

    pthread_mutex_lock(&cldrv->mtx);
    list_add_tail(&clrpc->node, &cldrv->head);
    cldrv->client_plug_event = 1;
    bn_tx_inotify(&cldrv->base);
    pthread_mutex_unlock(&cldrv->mtx);

    com_log(COM_CALLBACK_COM, "reg new debug rpc @%p with drv @%p", clrpc, cldrv);

    return prpc;
}

NODE_INFO cl_info = {
    .nodetype = nod_debug,
    .rpc_chk  = cl_rpc_chk,
    .start    = cl_rpc_nod_start,
};
