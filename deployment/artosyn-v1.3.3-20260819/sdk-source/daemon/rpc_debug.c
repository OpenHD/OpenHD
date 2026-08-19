#include "base_node.h"
#include "bb_api.h"
#include "com_log.h"
#include "rpc_dev_bind.h"
#include "rpc_node.h"
#include "sock_node.h"
#include "usb_event_list.h"
#include "usbpack.h"
#include <string.h>

static int find_socket(struct basenode* pnod, void* par)
{
    if (pnod->tp != nod_socket) {
        return 0;
    }

    sock_dev* psock = container_of(pnod, sock_dev, base);

    QUERY_TX_IN* qur = (QUERY_TX_IN*)par;

    return psock->slot == qur->slot && psock->port == qur->port;
}

static int buf_query(struct rpc_node* rpc, usbpack* pack)
{
    unsigned char buff[4096];

    usbpack retpack = {
        .reqid   = BB_RPC_SOCK_BUF_STA,
        .datalen = 0,
    };

    if (pack->datalen != sizeof(QUERY_TX_IN)) {
        int ret = make_usbpack2buff(buff, 4096, &retpack);
        send(rpc->tinfo->fd, (char*)buff, ret, 0);

        return -1;
    }

    QUERY_TX_IN qur;
    memcpy(&qur, pack->datapack, sizeof(QUERY_TX_IN));

    QUERY_TX_OUT quret;

    struct dev_node_list* plist = rpc->tinfo->plist;
    pthread_mutex_lock(&plist->mtx);
    struct basenode* pbn = plist_find_work_node(plist, find_socket, &qur);
    if (pbn) {
        sock_dev* psock  = container_of(pbn, sock_dev, base);
        quret.buff_index = psock->buf_head_index;
        quret.buff_avail = ringbuffer_avail(&psock->buf_dev);
        quret.buff_len   = ringbuffer_len(&psock->buf_dev);
        retpack.data_v   = &quret;
        retpack.datalen  = sizeof(quret);
        retpack.sta      = 0;
    } else {
        retpack.sta = -1;
        com_log(COM_NET, "errsta = %d", -1);
    }
    pthread_mutex_unlock(&plist->mtx);

    int ret = make_usbpack2buff(buff, 4096, &retpack);
    send(rpc->tinfo->fd, (char*)buff, ret, 0);

    return retpack.sta;
}

int rpc_debug_proc(struct rpc_node* rpc, usbpack* pack)
{
    switch (pack->reqid) {
    case BB_RPC_SOCK_BUF_STA:
        return buf_query(rpc, pack);

    default:
        break;
    }

    return -1;
}
