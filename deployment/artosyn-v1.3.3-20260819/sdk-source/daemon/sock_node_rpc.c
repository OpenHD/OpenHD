#include "bb_api.h"
#include "com_log.h"
#include "rpc_dev_bind.h"
#include "rpc_node.h"
#include "sock_node.h"
#include "unused.h"
#include "usb_event_list.h"
#include "usbpack.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define TX_DATA_CHECK 0
#define SOCK_OPEN_BASE_LEN 12
#define SOCK_OPEN_ENCRYPT_LEN 45

int find_socket(struct basenode* pnod, void* par)
{
    if (pnod->tp != nod_socket) {
        return 0;
    }

    sock_dev* pusb = container_of(pnod, sock_dev, base);

    sock_dev* psock = (sock_dev*)pnod;
    usbpack*  pack  = (usbpack*)par;

    int slot = (pack->reqid >> 8) & 0xff;
    int port = (pack->reqid >> 0) & 0xff;

    if (psock->port == port && psock->slot == slot) {
        return 1;
    }
    return 0;
}

static int sock_rpc_chk(unsigned char* buff, int len, struct threadinfo* tinfo)
{
    usbpack pack;
    int     ret = unpack_usb_pack(buff, len, 0, &pack);

    if (ret || pack.domainid != BB_REQ_SOCKET) {
        return RPC_CHK_NXT;
    }

    unsigned char opt  = (pack.reqid >> 16) & 0xff;
    unsigned char slot = (pack.reqid >> 8) & 0xff;
    unsigned char port = (pack.reqid >> 0) & 0xff;

    UNUSED(slot);
    UNUSED(port);

    if (opt != so_open) {
        return -2;
    }

    return RPC_CHK_SUC;
}

/**
 * @brief 返回给 app 状态数据数据
 *
 * @param rpc
 * @param pack
 */
void sock_rpc_send_msg(struct sock_rpc* rpc, usbpack* pack)
{
    int packlen = (pack->data_v && pack->datalen) ? pack->datalen : 0;
    packlen += get_fix_usblen();

    rpc_send_group* psg       = malloc(sizeof(rpc_send_group) + packlen);
    uint8_t*        datastart = ((uint8_t*)psg) + sizeof(rpc_send_group);

    psg->datalen = make_usbpack2buff(datastart, packlen, pack);

    pthread_mutex_lock(&rpc->mtx);
    list_add_tail(&psg->node, &rpc->send_cmd_list_head);
    pthread_cond_broadcast(&rpc->cv);
    pthread_mutex_unlock(&rpc->mtx);
}

static int sp_opt_proc(sock_rpc* priv, usbpack* pack, sock_dev* psoc, int opt, uint32_t reqid)
{
    switch (opt) {
    case so_query_len:
        if (psoc) {
            int32_t avail = ringbuffer_avail(&psoc->buf_dev);
            int32_t len   = ringbuffer_len(&psoc->buf_dev);

            QUERY_TX_OUT tmp = {
                .buff_index = psoc->buf_head_index,
                .buff_len   = len,
                .buff_avail = avail,
            };

            usbpack retpack = {
                .data_v  = &tmp,
                .datalen = sizeof(tmp),
                .reqid   = reqid,
                .sta     = 0,
            };

            sock_rpc_send_msg(priv, &retpack);
        } else {
            usbpack retpack = {
                .data_v  = NULL,
                .datalen = 0,
                .reqid   = reqid,
                .sta     = -1,
            };
            sock_rpc_send_msg(priv, &retpack);
        }
        return 0;
        break;
    case so_set_tx_limit:
        if (psoc) {
            pthread_mutex_lock(&psoc->mtx_tx_buf);
            BUFF_LIMIT* lim     = pack->data_v;
            psoc->buf_dev_limit = lim->buff_len;
            pthread_mutex_unlock(&psoc->mtx_tx_buf);

            BUFF_LIMIT outtmp = {
                .buff_len = psoc->buf_dev_limit,
            };

            usbpack retpack = {
                .data_v  = &outtmp,
                .datalen = sizeof(outtmp),
                .reqid   = reqid,
                .sta     = 0,
            };

            sock_rpc_send_msg(priv, &retpack);
        } else {
            usbpack retpack = {
                .data_v  = NULL,
                .datalen = 0,
                .reqid   = reqid,
                .sta     = -1,
            };
            sock_rpc_send_msg(priv, &retpack);
        }
        return 0;
        break;
    case so_get_tx_limit:
        if (psoc) {
            BUFF_LIMIT outtmp = {
                .buff_len = psoc->buf_dev_limit,
            };

            usbpack retpack = {
                .data_v  = &outtmp,
                .datalen = sizeof(outtmp),
                .reqid   = reqid,
                .sta     = 0,
            };

            sock_rpc_send_msg(priv, &retpack);
        } else {
            usbpack retpack = {
                .data_v  = NULL,
                .datalen = 0,
                .reqid   = reqid,
                .sta     = -1,
            };
            sock_rpc_send_msg(priv, &retpack);
        }
        return 0;
        break;
    default:
        break;
    }

    if (opt >= so_user_base_start && opt <= so_user_base_end) {
        if (!psoc || pack->datalen < 0 || (size_t)pack->datalen > sizeof(((socket_ioctl_ctl*)0)->tx_buff)) {
            usbpack retpack = {
                .data_v  = NULL,
                .datalen = 0,
                .reqid   = reqid,
                .sta     = -EINVAL,
            };
            sock_rpc_send_msg(priv, &retpack);
            return 0;
        }

        if (opt == BB_SOCK_ENC_SET) {
            bb_sock_upd_enc_t* enc = (bb_sock_upd_enc_t*)pack->datapack;
            if (pack->datalen != (int)sizeof(*enc) ||
                (enc->encrypt_en && enc->encrypt_mode >= BB_SOCK_ENCRYPT_MODE_MAX)) {
                usbpack retpack = {
                    .data_v  = NULL,
                    .datalen = 0,
                    .reqid   = reqid,
                    .sta     = -EINVAL,
                };
                sock_rpc_send_msg(priv, &retpack);
                return 0;
            }
        } else if (opt == BB_SOCK_ENC_GET && pack->datalen != 0) {
            usbpack retpack = {
                .data_v  = NULL,
                .datalen = 0,
                .reqid   = reqid,
                .sta     = -EINVAL,
            };
            sock_rpc_send_msg(priv, &retpack);
            return 0;
        }

        socket_ioctl_ctl* sic = malloc(sizeof(socket_ioctl_ctl));
        if (!sic) {
            usbpack retpack = {
                .data_v  = NULL,
                .datalen = 0,
                .reqid   = reqid,
                .sta     = -ENOMEM,
            };
            sock_rpc_send_msg(priv, &retpack);
            return 0;
        }

        sic->opt   = opt;
        sic->txflg = 0;
        memcpy(sic->tx_buff, pack->datapack, pack->datalen);
        sic->tx_len = pack->datalen;

        pthread_mutex_lock(&psoc->mtx_tx_buf);
        list_add_tail(&sic->node, &psoc->sock_ioctl_list);
        psoc->ioctl_act_flg = 1;
        dev_node_tx_inotify(priv->base.tinfo->plist);
        pthread_mutex_unlock(&psoc->mtx_tx_buf);
        return 0;
    }

    return -1;
}

static int rpc_socket_read_proc(struct rpc_node* rpc, usbpack* pack)
{
    sock_rpc* priv = (sock_rpc*)rpc;
    sock_dev* psoc = priv->sockdev;

    uint32_t reqid = pack->reqid;

    int opt  = (pack->reqid >> 16) & 0xff;
    int slot = (pack->reqid >> 8) & 0xff;
    int port = (pack->reqid >> 0) & 0xff;

    if (!sp_opt_proc(priv, pack, psoc, opt, reqid)) {
        return 0;
    }

    if (!psoc) {
        com_log(COM_NET, "no psoc!!");
        return -1;
    }

    switch (psoc->sock_sta) {
    case sock_not_init: {
        uint32_t sock_cmd_flg;
        uint32_t rx_buff_len;
        uint32_t tx_buff_len;
        uint8_t  encrypt_mode = BB_SOCK_ENCRYPT_MODE_DEFAULT;
        uint8_t  key[32]      = {0};

        if (pack->datalen != SOCK_OPEN_BASE_LEN && pack->datalen != SOCK_OPEN_ENCRYPT_LEN) {
            com_log(COM_SOCKET_COM, "reject socket open: invalid payload length %d", pack->datalen);
            usbpack retpack = {
                .reqid   = reqid,
                .data_v  = NULL,
                .datalen = 0,
                .sta     = -EINVAL,
            };
            sock_rpc_send_msg(priv, &retpack);
            break;
        }

        memcpy(&sock_cmd_flg, pack->datapack, sizeof(sock_cmd_flg));
        if ((sock_cmd_flg & BB_SOCK_FLAG_ENCRYPT) && pack->datalen != SOCK_OPEN_ENCRYPT_LEN) {
            com_log(COM_SOCKET_COM, "reject encrypted socket open: missing encryption parameters");
            usbpack retpack = {
                .reqid   = reqid,
                .data_v  = NULL,
                .datalen = 0,
                .sta     = -EINVAL,
            };
            sock_rpc_send_msg(priv, &retpack);
            break;
        }
        if (!(sock_cmd_flg & BB_SOCK_FLAG_ENCRYPT) && pack->datalen != SOCK_OPEN_BASE_LEN) {
            com_log(COM_SOCKET_COM, "reject unencrypted socket open: unexpected encryption parameters");
            usbpack retpack = {
                .reqid   = reqid,
                .data_v  = NULL,
                .datalen = 0,
                .sta     = -EINVAL,
            };
            sock_rpc_send_msg(priv, &retpack);
            break;
        }

        memcpy(&rx_buff_len, pack->datapack + 4, sizeof(rx_buff_len));
        memcpy(&tx_buff_len, pack->datapack + 8, sizeof(tx_buff_len));
        if (sock_cmd_flg & BB_SOCK_FLAG_ENCRYPT) {
            memcpy(&encrypt_mode, pack->datapack + 12, sizeof(encrypt_mode));
            if (encrypt_mode >= BB_SOCK_ENCRYPT_MODE_MAX) {
                com_log(COM_SOCKET_COM, "reject encrypted socket open: invalid mode %u", encrypt_mode);
                usbpack retpack = {
                    .reqid   = reqid,
                    .data_v  = NULL,
                    .datalen = 0,
                    .sta     = -EINVAL,
                };
                sock_rpc_send_msg(priv, &retpack);
                break;
            }
            memcpy(key, pack->datapack + 13, sizeof(key));
        }

        psoc->slot         = slot;
        psoc->port         = port;
        psoc->reqid.reqid  = reqid;
        psoc->sock_cmd_flg = sock_cmd_flg;
        psoc->rx_buff_len  = rx_buff_len;
        psoc->tx_buff_len  = tx_buff_len;
        psoc->encrypt_mode = encrypt_mode;
        memcpy(psoc->key, key, sizeof(psoc->key));

        psoc->sock_sta = sock_need_send_cmd;
        com_log(COM_SOCKET_COM, "socket try init slot %d port %d", psoc->slot, psoc->port);
        dev_node_tx_inotify(rpc->tinfo->plist);
        break;
    }
    case sock_need_send_cmd:
        com_log(COM_SOCKET_COM, "socket waiting remote cmd!");
        break;
    case sock_can_send_data:
    case sock_wait_usb_data: {
        switch (opt) {
        case so_write: {
            // write data;
            int len = sock_dev_push_data(psoc, priv, pack->datapack, pack->datalen);

            usbpack retpack = {
                .reqid   = BB_REQ_SOCKET << 24 | so_write_ret << 16 | slot << 8 | port,
                .data_v  = NULL,
                .datalen = 0,
                .sta     = len,
                .msgid   = pack->sta,
            };

            sock_rpc_send_msg(priv, &retpack);
        } break;
        case so_close:
            psoc->sock_sta = sock_need_send_close;
            bn_tx_inotify(&psoc->base);
            // close socket;
            break;
        case so_open: {
            usbpack retpack = {
                .reqid   = reqid,
                .data_v  = NULL,
                .datalen = 0,
                .sta     = 0,
            };

            sock_rpc_send_msg(priv, &retpack);
        } break;
        default:
            com_log(COM_SOCKET_COM, "recv error opt = %d", opt);
            break;
        }
    } break;
    default:
        com_log(COM_SOCKET_COM, "socket recv in error sta = %d!!", psoc->sock_sta);
        break;
    }
    return 0;
}

static int rpc_socket_read(struct rpc_node* rpc, unsigned char* buff, int len)
{
    sock_rpc* priv = (sock_rpc*)rpc;

    if (priv->rx_savelen + len > priv->rx_savemax) {
        int offset  = priv->rx_savelen + len - priv->rx_savemax;
        int leftlen = priv->rx_savelen - offset;
        memmove(priv->rx_savebuffer, priv->rx_savebuffer + offset, leftlen);
        priv->rx_savelen = leftlen;
    }

    memmove(priv->rx_savebuffer + priv->rx_savelen, buff, len);
    priv->rx_savelen += len;

    int offset = 0;
    do {
        usbpack pack;

        int retoffset = 0;

        int ret = unpack_usb_pack(priv->rx_savebuffer + offset, priv->rx_savelen - offset, &retoffset, &pack);

        if (!ret) {
            rpc_socket_read_proc(rpc, &pack);
            offset += retoffset;
        } else {
            break;
        }
    } while (true);

    if (offset >= priv->rx_savelen) {
        priv->rx_savelen = 0;
    } else if (offset) {
        int leftlen = priv->rx_savelen - offset;
        memmove(priv->rx_savebuffer, priv->rx_savebuffer + offset, leftlen);
        priv->rx_savelen = leftlen;
    }

    return 0;
}

int sock_rpc_push_read_data(sock_rpc* prpc, unsigned char* buff, int len)
{
    if (!prpc || !buff || len <= 0) {
        return -1;
    }

    sock_dev* psoc = prpc->sockdev;

    usbpack retpack = {
        .data_v  = buff,
        .datalen = len,
        .reqid   = BB_REQ_SOCKET << 24 | so_read << 16 | psoc->slot << 8 | psoc->port << 0,
        .msgid   = (uint32_t)prpc->rd_pos_init,
        .sta     = len,
    };

    com_log(COM_SOCKET_COM,
            "%d-%d , read @%#" PRIx64 "len = %#" PRIx64,
            psoc->slot,
            psoc->port,
            prpc->rd_pos_init,
            len);

    sock_rpc_send_msg(prpc, &retpack);
    prpc->rd_pos_init += len;

    pthread_mutex_lock(&prpc->mtx);
    pthread_cond_broadcast(&prpc->cv);
    pthread_mutex_unlock(&prpc->mtx);
    return len;
}

static void* socket_rpc_wr_thread(void* p)
{
    sock_rpc* priv = (sock_rpc*)p;
    rpc_node* pnod = &priv->base;

    pthread_mutex_lock(&priv->mtx);
    while (priv->workflg) {
        int txflg = 0;
        if (!list_empty(&priv->send_cmd_list_head)) {
            txflg++;
            rpc_send_group* psg = list_first_entry(&priv->send_cmd_list_head, rpc_send_group, node);
            list_del(&psg->node);
            pthread_mutex_unlock(&priv->mtx);
            char* txbuff = ((char*)psg) + sizeof(rpc_send_group);
            send(pnod->tinfo->fd, txbuff, psg->datalen, 0);
            free(psg);
            pthread_mutex_lock(&priv->mtx);
        }

        if (txflg == 0) {
            pthread_cond_wait(&priv->cv, &priv->mtx);
        }
    }
    pthread_mutex_unlock(&priv->mtx);

    com_log(COM_SOCKET_COM, "rpc_wr_thread exit");
    return NULL;
}

static int sock_rpc_node_end(struct rpc_node* prpc);

static rpc_act pact = {
    .rpc_rd_cb = rpc_socket_read,
    .end       = sock_rpc_node_end,
};
struct rpc_node* sock_rpc_nod_start(threadinfo* tinfo)
{
    sock_rpc* priv = (sock_rpc*)malloc(sizeof(sock_rpc));
    if (!priv) {
        return NULL;
    }

    rpc_node* prpc = &priv->base;
    prpc->tinfo    = tinfo;

    prpc->nodetype = nod_socket;
    priv->sockdev  = sock_node_alloc();
    if (!priv->sockdev) {
        com_log(COM_SOCKET_COM, "malloc socknode err");
        free(priv);
        return NULL;
    }
    prpc->pact    = &pact;
    priv->workflg = 1;

    priv->wr_cur_ptr = 0;
    INIT_LIST_HEAD(&priv->sock_wr_list);
    
    priv->rd_pos_init = 0;
    priv->rx_savelen  = 0;
    priv->rx_savemax  = RX_SAVEBUFFER_LEN;

    INIT_LIST_HEAD(&priv->send_cmd_list_head);

    pthread_mutex_init(&priv->mtx, NULL);
    pthread_cond_init(&priv->cv, NULL);
    pthread_create(&priv->wr_thread, NULL, socket_rpc_wr_thread, prpc);

    sock_node_start(priv->sockdev, priv);

    return prpc;
}

static int sock_rpc_node_end(struct rpc_node* prpc)
{
    if (!prpc) {
        return -1;
    }

    sock_rpc* sockrpc = container_of(prpc, sock_rpc, base);

    sock_dev* sockdev = NULL;
    pthread_mutex_lock(&sockrpc->mtx);
    if (sockrpc->sockdev) {
        sockdev          = sockrpc->sockdev;
        sockrpc->sockdev = NULL;
    }
    pthread_mutex_unlock(&sockrpc->mtx);

    if (sockdev) {
        pthread_mutex_lock(&sockdev->mtx_tx_buf);
        sockdev->sock_sta = sock_need_send_close;
        sockdev->sockrpc  = NULL;
        dev_node_tx_inotify(prpc->tinfo->plist);
        pthread_mutex_unlock(&sockdev->mtx_tx_buf);
    }

    pthread_mutex_lock(&sockrpc->mtx);
    while (!list_empty(&sockrpc->send_cmd_list_head)) {
        rpc_send_group* psg = list_first_entry(&sockrpc->send_cmd_list_head, rpc_send_group, node);
        list_del(&psg->node);
        free(psg);
    }

    pthread_mutex_unlock(&sockrpc->mtx);

    sock_wr_node *psock_wr_node, *sock_wr_node_tmp;
    list_for_each_entry_safe (psock_wr_node, sock_wr_node_tmp, &sockrpc->sock_wr_list, node, sock_wr_node) {
        list_del(&psock_wr_node->node);
        free(psock_wr_node);
    }
    // 回收wr thread
    sockrpc->workflg = 0;
    pthread_cond_broadcast(&sockrpc->cv);
    pthread_join(sockrpc->wr_thread, NULL);
    pthread_mutex_destroy(&sockrpc->mtx);
    pthread_cond_destroy(&sockrpc->cv);

    free(sockrpc);

    return 0;
}

NODE_INFO sock_info = {
    .nodetype = nod_socket,
    .start    = sock_rpc_nod_start,
    .rpc_chk  = sock_rpc_chk,
};
