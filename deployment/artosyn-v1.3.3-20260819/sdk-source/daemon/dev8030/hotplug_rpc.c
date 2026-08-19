#include "bb_api.h"
#include "com_log.h"
#include "rpc_node.h"
#include "usbpack.h"
#include <stdio.h>
#include <stdlib.h>
typedef struct {
    struct rpc_node  base;
    struct list_head node;
} hotplug_cb;

static int rpc_hotplug_read(struct rpc_node* rpc, unsigned char* buff, int len)
{
    return 0;
}

static int rpc_hotplug_end(struct rpc_node* prpc)
{
    if (!prpc) {
        return -1;
    }
    hotplug_cb* hotcb = container_of(prpc, hotplug_cb, base);

    rpc_info* rinfo = hotcb->base.tinfo->prpc_info;
    pthread_rwlock_wrlock(&rinfo->head_hotplug_lk);
    list_del(&hotcb->node);
    pthread_rwlock_unlock(&rinfo->head_hotplug_lk);

    com_log(COM_HOT_PLUG_COM, "exit on offline callback");
    free(hotcb);
    return 0;
}

static rpc_act hotplug_rpcact = {
    .rpc_rd_cb = rpc_hotplug_read,
    .end       = rpc_hotplug_end,
};

/**
 * @brief 生成热插拔节点
 *
 * @param tinfo
 * @return struct rpc_node*
 */
struct rpc_node* hotplug_cb_alloc(struct threadinfo* tinfo)
{
    hotplug_cb* pcb = (hotplug_cb*)malloc(sizeof(hotplug_cb));
    if (!pcb) {
        return NULL;
    }
    rpc_node* prpc = &pcb->base;
    prpc->tinfo    = tinfo;
    prpc->pact     = &hotplug_rpcact;

    rpc_info* rinfo = tinfo->prpc_info;
    pthread_rwlock_wrlock(&rinfo->head_hotplug_lk);
    list_add_tail(&pcb->node, &rinfo->head_hotplug_cb);
    pthread_rwlock_unlock(&rinfo->head_hotplug_lk);

    char buff[128];

    usbpack pack = {
        .datalen = 0,
        .reqid   = BB_RPC_GET_HOTPLUG_EVENT,
        .msgid   = 0,
    };

    int len = make_usbpack2buff((unsigned char*)buff, sizeof(buff), &pack);
    send(tinfo->fd, buff, len, 0);

    return prpc;
}

/**
 * @brief 通知设备上下线
 *
 * @param prpc
 * @param bb_event_hotplug_t 热插拔数据
 * @return int
 */
void dev8030_hotplug_event(rpc_info* prpc, bb_event_hotplug_t* plug_evt)
{
    if (list_empty(&prpc->head_hotplug_cb)) {
        return;
    }

    char buff[128];

    usbpack pack = {
        .datalen = sizeof(bb_event_hotplug_t),
        .data_v  = plug_evt,
        .reqid   = BB_RPC_GET_HOTPLUG_EVENT,
        .msgid   = 0,
        .sta     = 0,
    };

    int len = make_usbpack2buff((unsigned char*)buff, sizeof(buff), &pack);

    pthread_rwlock_rdlock(&prpc->head_hotplug_lk);
    hotplug_cb* pos;
    list_for_each_entry (pos, &prpc->head_hotplug_cb, node, hotplug_cb) {
        send(pos->base.tinfo->fd, buff, len, 0);
    }
    pthread_rwlock_unlock(&prpc->head_hotplug_lk);
}
