#include "sock_node.h"
#include "bb_api.h"
#include "com_log.h"
#include "rpc_dev_bind.h"
#include "rpc_node.h"
#include "unused.h"
#include "usb_event_list.h"
#include "usbpack.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static sock_dev* psockdev = NULL;

struct socket_msg_ret {
    uint64_t pos;
    uint32_t len;
};

static void check_send_cpl_ptr(sock_dev* psock, sock_rpc* priv, uint32_t reqid)
{
    sock_wr_node *psock_wr_node, *sock_wr_node_tmp;

    list_for_each_entry_safe (psock_wr_node, sock_wr_node_tmp, &priv->sock_wr_list, node, sock_wr_node) {
        if (psock->buf_head_index >= psock_wr_node->wr_max) {
            // 发送完成
            usbpack retpack = {
                .data_v  = NULL,
                .datalen = 0,
                .reqid   = reqid,
                .sta     = (int32_t)(psock_wr_node->wr_max - psock_wr_node->wr_init),
            };
            list_del(&psock_wr_node->node);
            free(psock_wr_node);
            sock_rpc_send_msg(priv, &retpack);
        } else {
            return;
        }
    }
}

static int dev_dat_so_write_proc(sock_dev* psock, struct usbpack* pack)
{
    psockdev = psock;

    unsigned char opt  = (pack->reqid >> 16) & 0xff;
    unsigned char slot = (pack->reqid >> 8) & 0xff;
    unsigned char port = (pack->reqid >> 0) & 0xff;

    UNUSED(opt);
    UNUSED(slot);
    UNUSED(port);

    // rpc完成提醒
    sock_rpc* priv = psock->sockrpc;
    if (!psock->sockrpc) {
        com_log(COM_SOCKET_COM, "%d-%d refuse write opt while remote disconnect", slot, port);
        return -1;
    }
    if (pack->sta >= 0) {
        uint64_t rcvpos;
        memcpy(&rcvpos, pack->datapack, sizeof(uint64_t));

        uint64_t endpos = rcvpos + (uint64_t)pack->sta;

        com_log(COM_SOCKET_DATA,
                "%d-%d send cpl pos %#" PRIx64 " ,len %#" PRIx32 " ,endpos = %#" PRIx64,
                slot,
                port,
                endpos - pack->sta,
                pack->sta,
                endpos);

        if (endpos < psock->buf_head_index || endpos > psock->buf_wr_index) {
            com_log(COM_SOCKET_DATA,
                    "%d-%d error pos msg!! %#" PRIx64 " , head = %#" PRIx64 ", wr = %#" PRIx64,
                    slot,
                    port,
                    endpos,
                    psock->buf_head_index,
                    psock->buf_wr_index);
        } else {
            uint64_t popcnt = endpos - psock->buf_head_index;
            ringbuffer_out(&psock->buf_dev, 0, (uint32_t)popcnt);
            psock->buf_head_index += popcnt;
            com_log(COM_SOCKET_DATA,
                    "%d-%d head buff = %#" PRIx64 " ,tx buffer left = %#" PRIx32,
                    slot,
                    port,
                    psock->buf_head_index,
                    ringbuffer_len(&psock->buf_dev));
        }

        check_send_cpl_ptr(psock, priv, pack->reqid);

        if (psock->sock_sta == sock_wait_usb_data) {
            // 8030发送
            psock->sock_sta = sock_can_send_data;
            bn_tx_inotify(&psock->base);
        }

    } else {
        struct socket_msg_ret retmsg;
        memcpy(&retmsg, pack->datapack, sizeof(retmsg));
        switch (pack->sta) {
        case -0x106: {
            com_log(COM_SOCKET_DATA,
                    "%d-%d send ok pos %#" PRIx64 " , len %#" PRIx32,
                    slot,
                    port,
                    retmsg.pos,
                    retmsg.len);
        } break;
        case -0x107: {
            // 强制更新写入地址
            psock->buf_wr_index = retmsg.pos;
            com_log(COM_SOCKET_DATA,
                    "%d-%d send pending pos %#" PRIx64 ", len %" PRIx32 " , buf head pos = %#" PRIx64
                    " wr head pos = %#" PRIx64,
                    slot,
                    port,
                    retmsg.pos,
                    retmsg.len,
                    psock->buf_head_index,
                    psock->buf_wr_index);
            int dev_dataflg = ringbuffer_get_data(&psock->buf_dev,
                                                  NULL,
                                                  (uint32_t)(psock->buf_wr_index - psock->buf_head_index),
                                                  1);

            if (psock->buf_wr_index == psock->buf_head_index && dev_dataflg) {
                psock->sock_sta = sock_can_send_data;
            } else if (psock->sock_sta == sock_can_send_data) {
                // 等待usb响应
                psock->sock_sta = sock_wait_usb_data;
            }
            bn_tx_inotify(&psock->base);
        } break;
        case -0x108: {
            com_log(COM_SOCKET_DATA,
                    "%d-%d written pos = %#" PRIx64 ", wanted pos = %# " PRIx32,
                    slot,
                    port,
                    retmsg.pos,
                    retmsg.len);
#if 0
            uint64_t wantedpos = retmsg.len;
            // 空闲状态
            if (psock->sock_sta == sock_wait_usb_data) {
                // 发送位置错误
                if (wantedpos >= psock->buf_head_index) {
                    uint32_t testpos = (uint32_t)(wantedpos - psock->buf_head_index);
                    int      testdat = ringbuffer_get_data(&psock->buf_dev, NULL, testpos, 1);
                    if (testdat) {
                        psock->sock_sta     = sock_can_send_data;
                        psock->buf_wr_index = wantedpos;
                        com_log(COM_SOCKET_COM, "%d-%d force resend pos = %#" PRIx64, slot, port, wantedpos);
                        bn_tx_inotify(&psock->base);
                    }
                }
            }
#endif
        } break;

        default:
            com_log(COM_SOCKET_COM, "%d-%d unknown err = %x", slot, port, -pack->sta);
            break;
        }
    }

    return 0;
}

static int dev_dat_so_read_proc(sock_dev* psock, unsigned char* buff, int len)
{
    sock_rpc* prpc = psock->sockrpc;

    if (!prpc) {
        com_log(COM_SOCKET_COM, "recive dat but remote rpc closed!");
        return 0;
    }
    sock_rpc_push_read_data(prpc, buff, len);
    return 0;
}

static int sock_dev_bb_ioctl(struct basenode* pnod, struct usbpack* pack, sock_dev* psock, sock_rpc* priv)
{
    unsigned char opt  = (pack->reqid >> 16) & 0xff;
    unsigned char slot = (pack->reqid >> 8) & 0xff;
    unsigned char port = (pack->reqid >> 0) & 0xff;

    if (opt < so_user_base_start) {
        return -1;
    }

    int findflg = 0;
    pthread_mutex_lock(&psock->mtx_tx_buf);
    socket_ioctl_ctl* sic;
    list_for_each_entry (sic, &psock->sock_ioctl_list, node, socket_ioctl_ctl) {
        if (opt == sic->opt) {
            findflg = 1;
            break;
        }
    }
    if (findflg) {
        list_del(&sic->node);
    }
    pthread_mutex_unlock(&psock->mtx_tx_buf);

    if (findflg) {
        sock_rpc_send_msg(priv, pack);
        free(sic);
    } else {
        com_log(COM_SOCKET_COM, "unknown user msg opt = %d", opt);
    }

    return 0;
}

static int sock_dev_pack_proc(struct basenode* pnod, struct usbpack* pack)
{
    sock_dev* psock = container_of(pnod, sock_dev, base);
    sock_rpc* priv  = psock->sockrpc;

    unsigned char opt  = (pack->reqid >> 16) & 0xff;
    unsigned char slot = (pack->reqid >> 8) & 0xff;
    unsigned char port = (pack->reqid >> 0) & 0xff;

    UNUSED(slot);
    UNUSED(port);

    if (opt == so_close) {
        // 接收到close 指令
        bn_set_free(&psock->base);
        return 0;
    }

    if (0 == sock_dev_bb_ioctl(pnod, pack, psock, priv)) {
        return 0;
    }

    switch (psock->sock_sta) {
    case sock_wait_usb_cmd:
        // sock 打开成功
        // 调用远程rpc
        if (!psock->sockrpc) {
            com_log(COM_SOCKET_COM, "socket rpc has closed!!");
            psock->sock_sta = sock_need_send_close;
            return 0;
        }

        int err_code = pack->sta;
        if (!pack->sta) {
            // 打开成功
            err_code          = 0;
            psock->sock_sta   = sock_can_send_data;
            psock->has_opened = 1;
            ringbuffer_reset(&psock->buf_dev);
        } else {
            // 打开失败
            psock->sock_sta = sock_not_init;
            if (pack->sta == 0x101) {
                com_log(COM_SOCKET_COM, "slot %d port %d socket has opened!", psock->slot, psock->port);
            }
        }
        usbpack retpack = {
            .data_v  = NULL,
            .datalen = 0,
            .reqid   = BB_REQ_SOCKET << 24 | so_open << 16 | slot << 8 | port,
            .sta     = err_code,
        };

        sock_rpc_send_msg(priv, &retpack);
        break;
    case sock_need_send_close:
    case sock_wait_usb_close:
        com_log(COM_SOCKET_COM, "refuse opt = %d in close phase", opt);
        break;
    case sock_wait_usb_data:
    case sock_can_send_data:
        if (!psock->sockrpc) {
            com_log(COM_SOCKET_COM, "socket rpc has closed!!");
            psock->sock_sta = sock_need_send_close;
            return 0;
        }

        if (opt == so_write) {
            pthread_mutex_lock(&psock->mtx_tx_buf);
            int ret = dev_dat_so_write_proc(psock, pack);
            pthread_mutex_unlock(&psock->mtx_tx_buf);

            return ret;
        } else if (opt == so_read) {
            return dev_dat_so_read_proc(psock, (unsigned char*)pack->datapack, pack->datalen);
        } else {
            com_log(COM_SOCKET_COM, "unknown opt = %d", opt);
        }
        break;
    default:
        com_log(COM_SOCKET_COM, "usb recv in error sta = %d", psock->sock_sta);
        break;
    }
    return 0;
}

static PROC_RET sock_dev_pack_chk(struct basenode* pnod, struct usbpack* pack)
{
    sock_dev* psoc = container_of(pnod, sock_dev, base);

    unsigned char doid = (pack->reqid >> 24) & 0xff;
    unsigned char opt  = (pack->reqid >> 16) & 0xff;
    unsigned char slot = (pack->reqid >> 8) & 0xff;
    unsigned char port = (pack->reqid >> 0) & 0xff;

    UNUSED(opt);

    if (doid != BB_REQ_SOCKET) {
        return proc_nxt;
    }

    if (slot == psoc->slot && port == psoc->port) {
        if (psoc->has_opened && opt == so_open) {
            return proc_nxt;
        }
        return proc_already;
    }
    return proc_nxt;
}

static int sock_dev_need_write(struct basenode* pnod)
{
    sock_dev* psock = container_of(pnod, sock_dev, base);

    if (psock->ioctl_act_flg) {
        return proc_already;
    }

    switch (psock->sock_sta) {
    case sock_need_send_cmd:
        return 1;
    case sock_need_send_close:
        // 没有打开的情况下直接可以关闭
        if (!psock->has_opened) {
            bn_set_free(pnod);
            return 0;
        }
        return 1;

    case sock_can_send_data:
        if (psock->sockrpc) {

            int ret = ringbuffer_get_data(&psock->buf_dev,
                                          NULL,
                                          (uint32_t)(psock->buf_wr_index - psock->buf_head_index),
                                          1);
            if (ret > 0) {
                return 1;
            }
        }
        break;
    default:
        break;
    }
    return 0;
}

static void add_new_wr_node(sock_rpc* priv, int slot, int port, int lenret)
{
    sock_wr_node* pnew = malloc(sizeof(sock_wr_node));

    pnew->wr_init = priv->wr_cur_ptr;
    pnew->wr_max  = priv->wr_cur_ptr + lenret;

    com_log(COM_SOCKET_DATA,
            "%d-%d rpc recv len = %#" PRIx32 ",cur = %#" PRIx64 " , max = %#" PRIx64,
            slot,
            port,
            lenret,
            priv->wr_cur_ptr,
            priv->wr_cur_ptr + lenret);

    priv->wr_cur_ptr += lenret;

    list_add_tail(&pnew->node, &priv->sock_wr_list);
}

/**
 * @brief 写入socket write数据
 *
 * @param psoc
 * @param datapack
 * @param datalen
 * @return int
 */
int sock_dev_push_data(sock_dev* psoc, sock_rpc* priv, uint8_t* datapack, int datalen)
{
    pthread_mutex_lock(&psoc->mtx_tx_buf);
    int lenret = ringbuffer_in_with_limit(&psoc->buf_dev, datapack, datalen, psoc->buf_dev_limit);

    if (lenret != datalen) {
        com_log(COM_SOCKET_DATA,
                "%d-%d warning loss rpc data len = %d,push = %d",
                psoc->slot,
                psoc->port,
                datalen,
                lenret);
    }

#if 1
    if (psoc->buf_wr_index != psoc->buf_head_index) {
        com_log(COM_SOCKET_DATA,
                "%d-%d warning , buf_wr_index = %#" PRIx64 ",buf_head_index = %#" PRIx64,
                psoc->slot,
                psoc->port,
                psoc->buf_wr_index,
                psoc->buf_head_index);
    }
#endif
    psoc->sock_sta = sock_can_send_data;

    add_new_wr_node(priv, psoc->slot, psoc->port, lenret);

    pthread_mutex_unlock(&psoc->mtx_tx_buf);

    bn_tx_inotify(&psoc->base);

    return lenret;
}

static int _sock_dev_pack_make(struct basenode* pnod, unsigned char* buff, int len)
{
    sock_dev* psock = container_of(pnod, sock_dev, base);

    socket_ioctl_ctl* sic;
    list_for_each_entry (sic, &psock->sock_ioctl_list, node, socket_ioctl_ctl) {

        if (sic->txflg) {
            continue;
        }

        sic->txflg   = 1;
        usbpack pack = {
            .reqid    = BB_REQ_SOCKET << 24 | sic->opt << 16 | psock->slot << 8 | psock->port,
            .datalen  = sic->tx_len,
            .datapack = sic->tx_buff,
        };

        return make_usbpack2buff(buff, len, &pack);
    }

    psock->ioctl_act_flg = 0;

    switch (psock->sock_sta) {
    case sock_need_send_cmd: {
        uint8_t bufftmp[45];
        int     cmd_len = 0;

        memcpy(bufftmp + cmd_len, &psock->sock_cmd_flg, 4);
        cmd_len += 4;
        memcpy(bufftmp + cmd_len, &psock->rx_buff_len, 4);
        cmd_len += 4;
        memcpy(bufftmp + cmd_len, &psock->tx_buff_len, 4);
        cmd_len += 4;

        if (psock->sock_cmd_flg & BB_SOCK_FLAG_ENCRYPT) {
            memcpy(bufftmp + cmd_len, &psock->encrypt_mode, 1);
            cmd_len += 1;
            memcpy(bufftmp + cmd_len, psock->key, sizeof(psock->key));
            cmd_len += sizeof(psock->key);
        }

        usbpack pack = {
            .reqid    = BB_REQ_SOCKET << 24 | so_open << 16 | psock->slot << 8 | psock->port,
            .datalen  = cmd_len,
            .datapack = bufftmp,
        };

        psock->sock_sta = sock_wait_usb_cmd;

        return make_usbpack2buff(buff, len, &pack);
    }
    case sock_need_send_close: {
        // 没有打开的情况下直接可以关闭
        if (!psock->has_opened) {
            bn_set_free(&psock->base);
            break;
        }
        usbpack pack = {
            .reqid    = BB_REQ_SOCKET << 24 | so_close << 16 | psock->slot << 8 | psock->port,
            .datalen  = 0,
            .datapack = 0,
        };

        psock->sock_sta = sock_wait_usb_close;

        return make_usbpack2buff(buff, len, &pack);
    }
    case sock_can_send_data: {

        usbpack pack = {
            .reqid = BB_REQ_SOCKET << 24 | so_write << 16 | psock->slot << 8 | psock->port,
        };
        int datalen = ringbuffer_get_data(&psock->buf_dev,
                                          buff + get_usb_data_offset() + sizeof(uint64_t),
                                          (uint32_t)(psock->buf_wr_index - psock->buf_head_index),
                                          psock->max_payload_len - sizeof(uint64_t));
        memcpy(buff + get_usb_data_offset(), &psock->buf_wr_index, sizeof(uint64_t));

        com_log(COM_SOCKET_DATA, "tx pos = %#" PRIx64 " , len = %#" PRIx32, psock->buf_wr_index, datalen);

        int ret = -1;
        if (datalen <= 0) {
            psock->sock_sta = sock_wait_usb_data;
            ret             = -1;
        } else {
            // 更新wr位置
            psock->buf_wr_index += datalen;
            pack.datalen = datalen + sizeof(uint64_t);
            ret          = make_usbpack2buff_head_only(buff, len, &pack);
        }
        return ret;
    }

    default:
        break;
    }
    return -1;
}

static int sock_dev_pack_make(struct basenode* pnod, unsigned char* buff, int len)
{
    sock_dev* psock = container_of(pnod, sock_dev, base);
    pthread_mutex_lock(&psock->mtx_tx_buf);
    int ret = _sock_dev_pack_make(pnod, buff, len);
    pthread_mutex_unlock(&psock->mtx_tx_buf);
    return ret;
}

static int sock_dev_deinit(struct basenode* pnod)
{
    sock_dev* psock = container_of(pnod, sock_dev, base);

    com_log(COM_SOCKET_COM, "remove socket node , slot = %d ,port = %d", psock->slot, psock->port);

    sock_rpc* sockrpc = NULL;
    pthread_mutex_lock(&psock->mtx_tx_buf);
    if (psock->sockrpc) {
        sockrpc        = psock->sockrpc;
        psock->sockrpc = NULL;
    }

    while (!list_empty(&psock->sock_ioctl_list)) {
        socket_ioctl_ctl* sic = list_first_entry(&psock->sock_ioctl_list, socket_ioctl_ctl, node);
        com_log(COM_SOCKET_COM, "remove socket ioctl code = %d", sic->opt);
        list_del(&sic->node);
        free(sic);
    }

    memset(psock->key, 0, sizeof(psock->key));

    pthread_mutex_unlock(&psock->mtx_tx_buf);

    if (sockrpc) {
        pthread_mutex_lock(&sockrpc->mtx);
        sockrpc->sockdev = NULL;
        socke_close(sockrpc->base.tinfo->fd);
        pthread_mutex_unlock(&sockrpc->mtx);
    }

    pthread_mutex_destroy(&psock->mtx_tx_buf);
    bnact.dev_deinit(pnod);
    free(psock);
    return 0;
}

static int sock_dev_cls(struct basenode* pnod)
{
    sock_dev* psock = container_of(pnod, sock_dev, base);
    if (psock->sockrpc && psock->sockrpc->base.tinfo) {
        bn_set_free(&psock->base);
        socke_close(psock->sockrpc->base.tinfo->fd);
    }
    return 0;
}

static char* sock_dev_name(struct basenode* pnod)
{
    sock_dev* psock = container_of(pnod, sock_dev, base);
    if (!psock->nameflg) {
        sprintf(psock->names, "sock slot %d , port %d", psock->slot, psock->port);
        psock->nameflg = 1;
    }
    return psock->names;
}

static baseact socact = {
    .dev_pack_chk   = sock_dev_pack_chk,
    .dev_pack_proc  = sock_dev_pack_proc,
    .dev_write_able = sock_dev_need_write,
    .dev_pack_make  = sock_dev_pack_make,
    .dev_deinit     = sock_dev_deinit,
    .dev_cls        = sock_dev_cls,
    .dev_nm         = sock_dev_name,
};

int sock_node_start(sock_dev* psock, struct sock_rpc* prpc)
{
    bn_init(&psock->base, prpc->base.tinfo->plist, nod_socket, &socact);

    psock->sock_sta    = sock_not_init;
    psock->sockrpc     = NULL;
    psock->reqid.reqid = 0;

    psock->buf_dev.buffer = psock->_buf_dev;
    psock->buf_dev.size   = sizeof(psock->_buf_dev);
    psock->buf_wr_index   = 0;
    psock->buf_dev_limit  = 0;
    psock->buf_head_index = 0;
    psock->has_opened     = 0;
    psock->nameflg        = 0;
    psock->ioctl_act_flg  = 0;
    psock->encrypt_mode   = BB_SOCK_ENCRYPT_MODE_DEFAULT;
    memset(psock->key, 0, sizeof(psock->key));

    INIT_LIST_HEAD(&psock->sock_ioctl_list);

    ringbuffer_reset(&psock->buf_dev);
    pthread_mutex_init(&psock->mtx_tx_buf, NULL);

    psock->sockrpc         = prpc;
    psock->max_payload_len = prpc->base.tinfo->plist->remote_buff_max_payload;

    pthread_mutex_lock(&prpc->base.tinfo->plist->mtx);
    // 加入链表
    plist_insert_node(prpc->base.tinfo->plist, &psock->base);

    pthread_mutex_unlock(&prpc->base.tinfo->plist->mtx);

    return 0;
}

sock_dev* sock_node_alloc(void)
{
    return malloc(sizeof(sock_dev));
}
