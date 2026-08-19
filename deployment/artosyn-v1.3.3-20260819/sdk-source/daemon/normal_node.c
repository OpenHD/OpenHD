#include "normal_node.h"
#include "base_node.h"
#include "bb_api.h"
#include "com_log.h"
#include "rpc_debug.h"
#include "rpc_dev_bind.h"
#include "rpc_node.h"
#include "usb_event_list.h"
#include "usbpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int normal_dev_pack_proc(struct basenode* pnod, struct usbpack* pack)
{
    nor_dev* pnormal = container_of(pnod, nor_dev, base);

    pnormal->need_write = 0;
    // 调用远程rpc
    if (pnormal->prpc) {
        unsigned char buff[4096];
        int           ret = make_usbpack2buff(buff, 4096, pack);
        pnormal->init_flg = 0;
        send(pnormal->prpc->tinfo->fd, (char*)buff, ret, 0);
    } else {
        bn_set_free(&pnormal->base);
    }
    return 0;
}

static PROC_RET normal_dev_pack_chk(struct basenode* pnod, struct usbpack* pack)
{
    nor_dev* pnormal = container_of(pnod, nor_dev, base);
    if (pnormal->reqid.reqid != pack->reqid) {
        return proc_nxt;
    }

    if (pnormal->cur_msgid != pack->msgid) {
        return proc_nxt;
    }

    return proc_already;
}

static int normal_need_write(struct basenode* pnod)
{
    nor_dev* pnormal = container_of(pnod, nor_dev, base);
    return pnormal->need_write;
}

static int normal_dev_pack_make(struct basenode* pnod, unsigned char* buff, int len)
{
    nor_dev* pnormal = container_of(pnod, nor_dev, base);

    if (!pnormal->need_write) {
        return -1;
    }

    pnormal->cur_msgid = dev_node_get_tmp_msgid(pnormal->base.plist);

    usbpack pack = {
        .reqid    = pnormal->reqid.reqid,
        .datalen  = pnormal->cmd_len,
        .datapack = pnormal->cmd_buff,
        .msgid    = pnormal->cur_msgid,
    };

    pnormal->need_write = 0;
    pnormal->init_flg   = 0;

    return make_usbpack2buff(buff, len, &pack);
}

static int normal_dev_deinit(struct basenode* pbn)
{
    nor_dev* pnormal = container_of(pbn, nor_dev, base);
    com_log(COM_IOCTL_COM, "ioctl dev exit %p %p", pnormal, pnormal->prpc);
    if(pnormal->prpc) {
        nor_rpc* norrpc = (nor_rpc*)pnormal->prpc;
        norrpc->nodev = NULL;
    }

    bnact.dev_deinit(pbn);
    free(pbn);
    return 0;
}

static int normal_dev_cls(struct basenode* pnod)
{
    nor_dev* pnormal = container_of(pnod, nor_dev, base);

    if (pnormal->prpc && pnormal->prpc->tinfo) {
        socke_close(pnormal->prpc->tinfo->fd);
    }

    return 0;
}

static char* normal_dev_name(struct basenode* pnod)
{
    return "ioctl";
}

static baseact noract = {
    .dev_pack_chk   = normal_dev_pack_chk,
    .dev_pack_proc  = normal_dev_pack_proc,
    .dev_write_able = normal_need_write,
    .dev_pack_make  = normal_dev_pack_make,
    .dev_deinit     = normal_dev_deinit,
    .dev_cls        = normal_dev_cls,
    .dev_nm         = normal_dev_name,
};

static int normal_node_start(nor_dev* pnormal, dev_node_list* plist)
{
    bn_init(&pnormal->base, plist, nod_normal, &noract);

    pnormal->cmd_len = 0;
    pnormal->ret_len = 0;

    pnormal->need_write = 0;
    pnormal->prpc       = NULL;
    pnormal->init_flg   = 0;

    return 0;
}

static int normal_rpc_chk(unsigned char* buff, int len, struct threadinfo* tinfo)
{
    usbpack pack;
    int     ret = unpack_usb_pack(buff, len, 0, &pack);

    if (ret) {
        return RPC_CHK_NXT;
    }

    if (pack.domainid >= 0 && pack.domainid <= 2) {
        return RPC_CHK_SUC;
    }

    if (pack.domainid == BB_REQ_RPC) {
        return RPC_CHK_SUC;
    }

    if (pack.domainid == BB_REQ_RPC_IOCTL) {
        return RPC_CHK_SUC;
    }

    if (pack.domainid == BB_REQ_REMOTE) {
        return RPC_CHK_SUC;
    }

    return RPC_CHK_NXT;
}

static int rpc_normal_read(struct rpc_node* rpc, unsigned char* buff, int len)
{
    if (len < 4) {
        return 0;
    }

    usbpack pack;
    int     ret = unpack_usb_pack(buff, len, 0, &pack);

    if (ret) {
        com_log(COM_IOCTL_COM, "recv err!");
        return 0;
    }

    if (pack.domainid == BB_REQ_RPC) {
        return rpc_debug_proc(rpc, &pack);
    }

    uint32_t reqid = pack.reqid;
    nor_rpc* norpc = (nor_rpc*)rpc;

    nor_dev* pnormal = norpc->nodev;
    if (pnormal->init_flg) {
        if (pnormal->reqid.reqid == reqid) {
            com_log(COM_IOCTL_COM, "recv reqid error = 0x%x,curid = 0x%x", reqid, pnormal->reqid.reqid);
            return 0;
        }
    } else {
        pnormal->reqid.reqid = reqid;
        pnormal->init_flg    = 1;
    }

    memcpy(pnormal->cmd_buff, pack.datapack, pack.datalen);
    pnormal->cmd_len    = pack.datalen;
    pnormal->need_write = 1;

    dev_node_tx_inotify(rpc->tinfo->plist);
    return 0;
}

static int normal_rpc_node_end(struct rpc_node* prpc);

static rpc_act pact = {
    .rpc_rd_cb = rpc_normal_read,
    .end       = normal_rpc_node_end,
};

static rpc_node* normal_rpc_nod_start(struct threadinfo* tinfo)
{
    rpc_node* prpc = (rpc_node*)malloc(sizeof(nor_rpc));
    if (!prpc) {
        return NULL;
    }
    nor_rpc* norpc = (nor_rpc*)prpc;
    prpc->tinfo    = tinfo;

    prpc->nodetype = nod_normal;
    norpc->nodev   = malloc(sizeof(nor_dev));
    if (!norpc->nodev) {
        com_log(COM_IOCTL_COM, "malloc normalnode err");
        free(prpc);
        return NULL;
    }

    prpc->pact = &pact;

    pthread_mutex_lock(&prpc->tinfo->plist->mtx);

    nor_dev* pnorm = norpc->nodev;
    normal_node_start(pnorm, prpc->tinfo->plist);
    pnorm->prpc = prpc;
    // 加入链表
    plist_insert_node(prpc->tinfo->plist, &norpc->nodev->base);

    pthread_mutex_unlock(&prpc->tinfo->plist->mtx);

    return prpc;
}

static int normal_rpc_node_end(struct rpc_node* prpc)
{
    if (!prpc) {
        return -1;
    }

    nor_rpc* norpc = (nor_rpc*)prpc;
    if (norpc->nodev) {
        nor_dev* pnod = norpc->nodev;
        pnod->prpc    = NULL;
        norpc->nodev  = NULL;
        bn_set_free(&pnod->base);
    }

    free(prpc);
    return 0;
}

NODE_INFO normal_info = {
    .nodetype = nod_normal,
    .start    = normal_rpc_nod_start,
    .rpc_chk  = normal_rpc_chk,
};
