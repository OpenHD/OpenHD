#include "callback_node.h"
#include "base_node.h"
#include "bb_api.h"
#include "com_cfg.h"
#include "com_log.h"
#include "rpc_dev_bind.h"
#include "rpc_node.h"
#include "usb_event_list.h"
#include "usbpack.h"
#include <stdio.h>
#include <stdlib.h>

static void set_callback_domain(uint32_t* reqid, uint8_t cb_domain)
{
    if (reqid) {
        *reqid = (*reqid & ~(0xff << 16)) | (cb_domain << 16);
    }
}

static void send_data_to_list(cb_dev* pcb, struct usbpack* pack, int staflg)
{
    char buff[2048];

    int len = make_usbpack2buff((unsigned char*)buff, sizeof(buff), pack);

    pthread_mutex_lock(&pcb->mtx);
    cb_rpc* pos;
    list_for_each_entry (pos, &pcb->head, lists, cb_rpc) {
        if (pos->initflag != staflg) {
            continue;
        }
        send(pos->base.tinfo->fd, buff, len, 0);
    }
    pthread_mutex_unlock(&pcb->mtx);
}

static void cb_set_sta(cb_dev* pcb, int sta)
{
    pthread_mutex_lock(&pcb->mtx);
    cb_rpc* pos;
    list_for_each_entry (pos, &pcb->head, lists, cb_rpc) {
        pos->initflag = sta;
    }
    pthread_mutex_unlock(&pcb->mtx);
}

static int cb_dev_pack_proc(struct basenode* pnod, struct usbpack* pack)
{
    cb_dev* pcb = container_of(pnod, cb_dev, base);

    switch (pcb->cb_status) {
    case cb_wait_usb_cmd:
        com_log(COM_CALLBACK_COM, "cb: evt %d get usb cmd sta = %d", pcb->cbevt, pack->sta);
        if (pack->sta == 0) {
            com_log(COM_CALLBACK_COM, "cb: evt %d subscribe ok", pcb->cbevt);
            sprintf(pcb->names, "cb id = %d", pcb->cbevt);
            pcb->cb_status = cb_wait_usb_data;
            set_callback_domain(&pack->reqid, SUBSCRIBE_REQ_RET);
            if (!list_empty(&pcb->head)) {
                send_data_to_list(pcb, pack, 0);
            }

            cb_set_sta(pcb, 1);
        } else {
            // 发送请求失败
            pcb->cb_status = cb_not_init;
            set_callback_domain(&pack->reqid, SUBSCRIBE_REQ_FAL);
            if (!list_empty(&pcb->head)) {
                send_data_to_list(pcb, pack, 0);
            } else {
                // 删除节点
                bn_set_free(&pcb->base);
                return -1;
            }
        }
        break;
    case cb_wait_usb_data:
        com_log(COM_CALLBACK_DAT, "cb: evt %d get usb dat", pcb->cbevt);
        if (!list_empty(&pcb->head)) {
            set_callback_domain(&pack->reqid, SUBSCRIBE_DAT_RET);
            send_data_to_list(pcb, pack, 1);
        }
        break;
    default:
        com_log(COM_CALLBACK_COM, "cb : evt %d error sta = %d", pcb->cbevt, pcb->cb_status);
        break;
    }
    return 0;
}

static PROC_RET cb_dev_pack_chk(struct basenode* pnod, struct usbpack* pack)
{
    cb_dev* pcb = container_of(pnod, cb_dev, base);
    if (pack->reqid >> 24 != BB_REQ_CB) {
        return proc_nxt;
    }

    if ((pack->reqid & 0xffff) != pcb->cbevt) {
        return proc_nxt;
    }

    return proc_already;
}

static int cb_need_write(struct basenode* pnod)
{
    cb_dev* pcb = container_of(pnod, cb_dev, base);
    return pcb->cb_status == cb_need_send_cmd || pcb->cb_status == cb_need_send_close;
}

static int cb_dev_pack_make(struct basenode* pnod, unsigned char* buff, int len)
{
    cb_dev* pcb = container_of(pnod, cb_dev, base);

    if (!cb_need_write(pnod)) {
        return -1;
    }

    usbpack pack = {
        .reqid = BB_REQ_CB << 24 | 1 << 16 | pcb->cbevt,
    };

    pcb->cb_status = cb_wait_usb_cmd;

    return make_usbpack2buff(buff, len, &pack);
}

static int cb_dev_deinit(struct basenode* pnod)
{
    cb_dev* pcb = container_of(pnod, cb_dev, base);
    com_log(COM_CALLBACK_COM, "cbdrv(%p) %s evtid(%d) exit", pcb, pcb->names, pcb->cbevt);
    pthread_mutex_destroy(&pcb->mtx);
    bnact.dev_deinit(pnod);
    free(pcb);
    return 0;
}

static void check_all_exit(cb_dev* cbdev)
{
    if (!cbdev->exitflg) {
        return;
    }

    if (list_empty(&cbdev->head)) {
        bn_set_free(&cbdev->base);
    }
}

static int cb_dev_cls(struct basenode* pnod)
{
    cb_dev* pcb = container_of(pnod, cb_dev, base);

    // 设置退出
    pcb->exitflg = 1;
    com_log(COM_CALLBACK_COM, "evt %d , trig close", pcb->cbevt);

    pthread_mutex_lock(&pcb->mtx);
    if (!list_empty(&pcb->head)) {
        cb_rpc* pos;
        list_for_each_entry (pos, &pcb->head, lists, cb_rpc) {
            socke_close(pos->base.tinfo->fd);
        }
    } else {
        check_all_exit(pcb);
    }
    pthread_mutex_unlock(&pcb->mtx);

    return 0;
}

static char* cb_dev_names(struct basenode* pnod)
{
    cb_dev* pcb = container_of(pnod, cb_dev, base);

    return pcb->names;
}

static baseact cbact = {
    .dev_pack_chk   = cb_dev_pack_chk,
    .dev_pack_proc  = cb_dev_pack_proc,
    .dev_write_able = cb_need_write,
    .dev_pack_make  = cb_dev_pack_make,
    .dev_deinit     = cb_dev_deinit,
    .dev_cls        = cb_dev_cls,
    .dev_nm         = cb_dev_names,
};

static int offline_cb_dev_pack_make(struct basenode* pnod, unsigned char* buff, int len)
{
    cb_dev* pcb = container_of(pnod, cb_dev, base);

    com_log(COM_CALLBACK_COM, "offline evt subscribe ok");
    sprintf(pcb->names, "cb offline");
    pcb->cb_status = cb_wait_usb_data;

    usbpack pack = {
        .datalen  = 0,
        .datapack = NULL,
        .domainid = BB_REQ_CB << 24 | pcb->cbevt,
        .sta      = 0,
        .msgid    = 0,
    };

    if (!list_empty(&pcb->head)) {
        send_data_to_list(pcb, &pack, 0);
    }

    cb_set_sta(pcb, 1);
    return -1;
}

static baseact offline_cbact = {
    .dev_write_able = cb_need_write,
    .dev_pack_make  = offline_cb_dev_pack_make,
    .dev_deinit     = cb_dev_deinit,
    .dev_cls        = cb_dev_cls,
    .dev_nm         = cb_dev_names,
};

static int cb_node_start(cb_dev* cbdev, dev_node_list* plist, int evtid)
{
    bn_init(&cbdev->base, plist, nod_callback, &cbact);
    if (evtid == BB_EVENT_OFFLINE) {
        cbdev->base.pact = &offline_cbact;
    }

    cbdev->cbevt     = evtid;
    cbdev->cb_status = cb_not_init;

    cbdev->exitflg = 0;
    sprintf(cbdev->names, "cb not start");

    INIT_LIST_HEAD(&cbdev->head);

    pthread_mutex_init(&cbdev->mtx, NULL);

    return 0;
}

static int cb_rpc_chk(unsigned char* buff, int len, struct threadinfo* tinfo)
{
    usbpack pack;
    int     ret = unpack_usb_pack(buff, len, 0, &pack);

    if (ret) {
        return RPC_CHK_NXT;
    }

    if (pack.domainid == BB_REQ_CB) {
        return RPC_CHK_SUC;
    }
    return RPC_CHK_NXT;
}

static int find_id(struct basenode* pnod, void* par)
{
    if (pnod->tp != nod_callback) {
        return 0;
    }

    cb_dev* pcb    = container_of(pnod, cb_dev, base);
    int*    pevtid = (int*)par;
    return pcb->cbevt == *pevtid;
}

static cb_dev* get_cb_dev_node(struct rpc_node* rpc, struct dev_node_list* plist, int evtid)
{
    pthread_mutex_lock(&plist->mtx);
    // 寻找cb_dev
    struct basenode* pbn   = plist_find_work_node(plist, find_id, &evtid);
    cb_dev*          cbdev = container_of(pbn, cb_dev, base);

    cb_rpc* this_rpc = container_of(rpc, cb_rpc, base);

    if (!cbdev) {
        // 创建 cb_dev
        cbdev = (cb_dev*)malloc(sizeof(cb_dev));
        if (!cbdev) {
            com_log(COM_CALLBACK_COM, "malloc dev_node err");
            pthread_mutex_unlock(&plist->mtx);
            return NULL;
        }
        cb_node_start(cbdev, plist, evtid);
        plist_insert_node(plist, &cbdev->base);
    }
    pthread_mutex_unlock(&plist->mtx);

    // 加入callback list
    pthread_mutex_lock(&cbdev->mtx);
    list_add(&this_rpc->lists, &cbdev->head);
    pthread_mutex_unlock(&cbdev->mtx);

    return cbdev;
}

static int rpc_cb_read(struct rpc_node* rpc, unsigned char* buff, int len)
{
    usbpack pack;
    int     ret = unpack_usb_pack(buff, len, 0, &pack);

    if (ret) {
        com_log(COM_CALLBACK_COM, "cb get error msg len = %d", len);
        return -1;
    }

    if ((pack.reqid & 0xffff) >= BB_EVENT_MAX) {
        // 返回失败
        pack.sta   = -2;
        pack.msgid = 0;

        len = make_usbpack2buff(buff, len, &pack);
        send(rpc->tinfo->fd, (char*)buff, len, 0);
        return 0;
    }

    cb_rpc* cbrpc = container_of(rpc, cb_rpc, base);

    if (cbrpc->cbdev == NULL) {
        // 第一次接收
        cb_dev* pcb = get_cb_dev_node(rpc, rpc->tinfo->plist, pack.reqid & 0xffff);

        cbrpc->cbdev = pcb;
        if (pcb->cb_status == cb_wait_usb_data) {
            // 直接返回ok
            pack.sta   = 0;
            pack.msgid = 0;

            cbrpc->initflag = 1;
            set_callback_domain(&pack.reqid, SUBSCRIBE_REQ_RET);
            com_log(COM_CALLBACK_COM, "cb: evt %d had subscribed", pack.reqid & 0xffff);
            len = make_usbpack2buff((unsigned char*)buff, len, &pack);
            if (len > 0) {
                send(rpc->tinfo->fd, (char*)buff, len, 0);
            }
            return 0;
        }
    }

    // check status
    cb_dev* pcb = cbrpc->cbdev;
    if (pcb->cb_status != cb_not_init) {
        if (pcb->cbevt == (pack.reqid & 0xffff)) {
            com_log(COM_CALLBACK_COM, "recv reqid error = 0x%x,evt = 0x%x", pack.reqid, pcb->cbevt);
            return 0;
        }
    } else {
        pcb->cbevt     = pack.reqid & 0xffff;
        pcb->cb_status = cb_need_send_cmd;
    }

    dev_node_tx_inotify(rpc->tinfo->plist);
    return 0;
}

static int cb_rpc_node_end(struct rpc_node* prpc);

static rpc_act cb_rcpact = {
    .rpc_rd_cb = rpc_cb_read,
    .end       = cb_rpc_node_end,
};

static struct rpc_node* cb_rpc_nod_start(struct threadinfo* tinfo)
{
    cb_rpc* pcb = (cb_rpc*)malloc(sizeof(cb_rpc));
    if (!pcb) {
        return NULL;
    }
    pcb->initflag  = 0;
    rpc_node* prpc = &pcb->base;
    prpc->tinfo    = tinfo;
    prpc->pact     = &cb_rcpact;
    prpc->nodetype = nod_callback;

    pcb->cbdev = NULL;

    return prpc;
}

static int cb_rpc_node_end(struct rpc_node* prpc)
{
    if (!prpc) {
        return -1;
    }

    cb_rpc* cbrpc = container_of(prpc, cb_rpc, base);

    cb_dev* cbdev = cbrpc->cbdev;
    com_log(COM_CALLBACK_COM, "rpc:%p dev:%p", cbdev, prpc);
    if (cbdev) {
        pthread_mutex_lock(&cbdev->mtx);
        list_del(&cbrpc->lists);
        pthread_mutex_unlock(&cbdev->mtx);
        cbrpc->cbdev = NULL;
        // 检查退出
        check_all_exit(cbdev);
    }

    free(prpc);
    return 0;
}

void add_base_callback(dev_node_list* plist, int maxnum)
{
    for (int i = 0; i < maxnum; i++) {
        cb_dev* cbdev = (cb_dev*)malloc(sizeof(cb_dev));
        if (!cbdev) {
            com_log(COM_CALLBACK_COM, "malloc dev_node err");
            return;
        }
        cb_node_start(cbdev, plist, i);
        cbdev->cb_status = cb_need_send_cmd;
        plist_insert_node(plist, &cbdev->base);
    }
}

NODE_INFO cb_info = {
    .nodetype = nod_callback,
    .start    = cb_rpc_nod_start,
    .rpc_chk  = cb_rpc_chk,
};
