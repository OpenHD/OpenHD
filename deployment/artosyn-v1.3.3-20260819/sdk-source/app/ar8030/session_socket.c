#include "ar8030.h"
#include "bb_api.h"
#include "bb_dev.h"
#include "com_cfg.h"
// #include "com_log.h"
#include "ioctl_tab.h"
#include "list.h"
#include "ringbuffer.h"
#include "session.h"
#include "session_socket_datagram_ext.h"
#include "socket_fd.h"
#include "timspec_helper.h"
#include "unused.h"
#include "usbpack.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFLEN (SOCK_LEN_APP_TO_DAEMON)

typedef struct SOCK_SESSION SOCK_SESSION;

typedef void (*wkfun)(SOCK_SESSION* sock, struct usbpack* pack, void* prv);

typedef struct wake_up wake_up;
typedef struct wake_up {
    struct list_head node;

    wkfun wkf;
    void* prv;
} wake_up;

typedef struct SOCK_SESSION {
    BASE_SESSION base;
    ringbuffer_t rx_rb;
    uint8_t      rx_buffer[BUFLEN];

    int sock_slot;
    int sock_port;
    int sock_flg;
    int is_opened;

    pthread_mutex_t somtx;
    pthread_cond_t  socv;

    int      datagram_mode;
    uint8_t* datagram_buffer_rx;
    uint8_t* datagram_buffer_tx;

    struct list_head head;

    pthread_cond_t exitcv;

    uint8_t exit_flg : 1;

    int sockfd;

    int send_index;

    int send_seq;
} SOCK_SESSION;

typedef struct {
    int wr_len;
    int wr_pkt_id;
    int wr_daemon_cpl_len;
} so_write_ret_dat;

static int so_rpc_cb(struct BB_HANDLE* phd, struct usbpack* pack, struct BASE_SESSION* psess)
{
    SOCK_SESSION* sock = container_of(psess, SOCK_SESSION, base);
    unsigned char opt  = (pack->reqid >> 16) & 0xff;
    unsigned char slot = (pack->reqid >> 8) & 0xff;
    unsigned char port = (pack->reqid >> 0) & 0xff;

    UNUSED(port);
    UNUSED(slot);

    pthread_mutex_lock(&sock->somtx);
    int packflg = 0;
    // 接收到数据
    if (sock->is_opened && opt == so_read) {
        // push data
        int pushlen = ringbuffer_in(&sock->rx_rb, pack->datapack, pack->datalen);
        if (pushlen != pack->datalen) {
            printf("recv data leak!! get = %d , push= %d\n", pack->datalen, pushlen);
        }
        packflg++;
    }
    // 需要唤醒
    wake_up* test;
    list_for_each_entry (test, &sock->head, node, wake_up) {
        packflg++;
        if (test->wkf) {
            test->wkf(sock, pack, test->prv);
        }
    }

    if (sock->is_opened && opt == so_write_ret) {
        packflg++;
        // printf("ret msgid = %d , ret = %d", pack->msgid, pack->sta);
    }

    pthread_mutex_unlock(&sock->somtx);
    // 包异常
    if (!packflg) {
        printf("recv bad socket pack , opt = %d datalen = %d packsta = %d\n", opt, pack->datalen, pack->sta);
    }

    return 0;
}

static void so_deinit(struct BASE_SESSION* psess)
{
    SOCK_SESSION* sock = container_of(psess, SOCK_SESSION, base);

    socket_del_fd(sock->sockfd);

    printf("sock: slot %d port %d deinit\n", sock->sock_slot, sock->sock_port);
    sock->exit_flg = 1;

    pthread_mutex_lock(&sock->somtx);
    while (!list_empty(&sock->head)) {
        pthread_cond_broadcast(&sock->socv);
        printf("socket wait subprocess exit!\n");
        pthread_cond_wait(&sock->exitcv, &sock->somtx);
    }
    pthread_mutex_unlock(&sock->somtx);

    pthread_cond_destroy(&sock->exitcv);
    if (sock->datagram_buffer_tx) {
        free(sock->datagram_buffer_tx);
    }

    pthread_mutex_destroy(&sock->somtx);
    pthread_cond_destroy(&sock->socv);

    if (sock->datagram_buffer_rx) {
        free(sock->datagram_buffer_rx);
    }
    bs_fun.de_init(psess);
    free(psess);
}

static int so_type(void)
{
    return st_socket;
}

const BASE_FUN sofun = {
    .rdcb    = so_rpc_cb,
    .de_init = so_deinit,
    .tpid    = so_type,
};

static void sock_init(SOCK_SESSION* sock, struct BB_HANDLE* phd, int slot, int port, int flg)
{
    bs_init(&sock->base, phd, &sofun);

    sock->rx_rb.buffer = sock->rx_buffer;
    sock->rx_rb.size   = BUFLEN;

    sock->sock_slot = slot;
    sock->sock_port = port;
    sock->sock_flg  = flg;

    sock->is_opened = 0;

    INIT_LIST_HEAD(&sock->head);

    pthread_cond_init(&sock->exitcv, NULL);

    sock->exit_flg = 0;
    sock->sockfd   = -1;

    sock->send_index = 0;

    pthread_mutex_init(&sock->somtx, NULL);
    pthread_cond_init(&sock->socv, NULL);
}

static SOCK_SESSION* create_socket_sess(bb_dev_handle_t* pdev, int slot, int port, int flg)
{
    struct BB_HANDLE* phd = bb_gethandle(pdev);
    if (!phd) {
        return NULL;
    }
    SOCK_SESSION* sock = (SOCK_SESSION*)malloc(sizeof(SOCK_SESSION));
    sock_init(sock, phd, slot, port, flg);
    return sock;
}

static void wk_op(SOCK_SESSION* sock, usbpack* pack, void* prv)
{
    unsigned char opt = (pack->reqid >> 16) & 0xff;

    if (opt == so_open) {
        int* sta = prv;
        *sta     = pack->sta;
        pthread_cond_broadcast(&sock->socv);
    }
}

AR8030_API int bb_socket_open(bb_dev_handle_t* pdev, bb_slot_e slot, uint32_t port, uint32_t flg, bb_sock_opt_t* opt)
{
    if (!pdev) {
        return -1;
    }

    if ((flg & (BB_SOCK_FLAG_TX | BB_SOCK_FLAG_RX)) == 0) {
        printf("bb socket open using err flg\n");
        return -1;
    }
    if ((flg & BB_SOCK_FLAG_ENCRYPT) &&
        (!opt || opt->encrypt_mode >= BB_SOCK_ENCRYPT_MODE_MAX)) {
        printf("bb encrypted socket open using invalid options\n");
        return -EINVAL;
    }

    SOCK_SESSION* sock = create_socket_sess(pdev, slot, port, flg);
    if (!sock) {
        printf("can't connect to remote\n");
        return -2;
    }

    if (flg & BB_SOCK_FLAG_DATAGRAM) {
        sock->datagram_buffer_rx = malloc(BUFLEN);
        sock->datagram_buffer_tx = malloc(BUFLEN);
        sock->datagram_mode      = 1;
    } else {
        sock->datagram_buffer_rx = NULL;
        sock->datagram_buffer_tx = NULL;
        sock->datagram_mode      = 0;
    }

    // 启动创建socket
    uint8_t       size = 0;
    uint8_t       buff[45] = {0};
    uint32_t      req = 4 << 24 | so_open << 16 | slot << 8 | port << 0;
    bb_sock_opt_t tmpopt = {0};
    tmpopt.rx_buf_size = BB_CONFIG_MAC_RX_BUF_SIZE;
    tmpopt.tx_buf_size = BB_CONFIG_MAC_TX_BUF_SIZE;
    if (!opt) {
        opt = &tmpopt;
    }

    flg &= (BB_SOCK_FLAG_TX | BB_SOCK_FLAG_RX | BB_SOCK_FLAG_ENCRYPT);
    /**
     * Only send flag and tx/rx buffer size
     * |--flag--|--tx_buf--|--rx_buf--|--encrypt_mode--|--encrypt_key--|
     * |-4bytes-|--4bytes--|--4bytes--|------1 byte----|----32 bytes---|
    */
    memcpy(buff + size, &flg, 4);
    size += 4;
    memcpy(buff + size, &opt->tx_buf_size, 4);
    size += 4;
    memcpy(buff + size, &opt->rx_buf_size, 4);
    size += 4;

    if (flg & BB_SOCK_FLAG_ENCRYPT) {
        memcpy(buff + size, &opt->encrypt_mode, 1);
        size += 1;
        memcpy(buff + size, opt->key, sizeof(opt->key));
        size += sizeof(opt->key);
    }

    usbpack pack = {
        .reqid   = req,
        .data_v  = buff,
        .datalen = size,
    };

    int     sta = 0;
    wake_up wu  = {
         .wkf = wk_op,
         .prv = &sta,
    };
    pthread_mutex_lock(&sock->somtx);
    list_add(&wu.node, &sock->head);

    send_usbpack(sock->base.phd, &pack);

    pthread_cond_wait(&sock->socv, &sock->somtx);

    list_del(&wu.node);
    int exitflg = sock->exit_flg;
    if (exitflg) {
        pthread_cond_broadcast(&sock->exitcv);
        sta = -1;
    }
    pthread_mutex_unlock(&sock->somtx);

    if (sta) {
        // 创建失败
        if (!exitflg) {
            bs_trig_close(&sock->base);
        }
        return -1;
    }
    // 创建成功
    int fd = socket_add_fd(pdev, sock);

    ringbuffer_reset(&sock->rx_rb);
    sock->sockfd = fd;

    sock->is_opened = 1;

    return fd;
}

static void wk_wr(SOCK_SESSION* sock, usbpack* pack, void* prv)
{
    unsigned char opt = (pack->reqid >> 16) & 0xff;

    if (!sock->is_opened) {
        return;
    }

    so_write_ret_dat* pdat = prv;

    switch (opt) {
    case so_write_ret:
        if (pdat->wr_pkt_id == pack->msgid) {
            pdat->wr_daemon_cpl_len = pack->sta;
        }
        break;
    case so_write:
        pdat->wr_len = pack->sta;
        pthread_cond_broadcast(&sock->socv);
        break;
    default:
        break;
    }
}

static int _bb_socket_write(SOCK_SESSION* sock, void* inbuff, uint32_t len, struct timespec* timeout)
{
    uint32_t req = 4 << 24 | so_write << 16 | sock->sock_slot << 8 | sock->sock_port << 0;

    usbpack pack = {
        .reqid   = req,
        .data_v  = (void*)inbuff,
        .datalen = len,
        .sta     = sock->send_seq++,
    };

    pthread_mutex_lock(&sock->somtx);

    so_write_ret_dat wr_dat = {
        .wr_len            = 0,
        .wr_pkt_id         = pack.sta,
        .wr_daemon_cpl_len = 0,
    };

    wake_up wu = {
        .wkf = wk_wr,
        .prv = &wr_dat,
    };
    list_add(&wu.node, &sock->head);
    // com_log(COM_SOCKET_DATA, "send start = %d", sock->send_index);
    send_usbpack(sock->base.phd, &pack);
    // com_log(COM_SOCKET_DATA, "send end = %d , len = %d", sock->send_index, pack.datalen);
    sock->send_index += pack.datalen;
    int sta = 0;
    while (!sock->exit_flg && !wr_dat.wr_len) {
        if (timeout) {
            sta = pthread_cond_timedwait(&sock->socv, &sock->somtx, timeout);
            if (sta == ETIMEDOUT) {
                break;
            } else if (sta != 0) {
                printf("wr pthread_cond_timedwait err ret = %d\n", sta);
            }
        } else {
            pthread_cond_wait(&sock->socv, &sock->somtx);
        }
    }

    list_del(&wu.node);
    if (sock->exit_flg) {
        pthread_cond_broadcast(&sock->exitcv);
        wr_dat.wr_len = -1;
    }
    pthread_mutex_unlock(&sock->somtx);

    if (sta == ETIMEDOUT) {
        wr_dat.wr_len = wr_dat.wr_daemon_cpl_len;
    }

    return wr_dat.wr_len;
}

extern int max_payload(void);

AR8030_API int bb_socket_write(int sockfd, void* inbuff, uint32_t len, int timeout)
{
    SOCK_SESSION* sock = socket_get_session(sockfd);
    uint32_t      len0 = len;
    if (!sock || sock->exit_flg) {
        return -1;
    }

    if ((sock->sock_flg & BB_SOCK_FLAG_TX) == 0) {
        return -1;
    }

    const uint32_t maxsnd = max_payload();

    uint32_t        curlen = 0;
    struct timespec tp;
    if (timeout > 0) {
        get_timespec_from_now(&tp, timeout);
    }

    if (sock->datagram_mode) {
        usbpack pack = {
            .datalen = len,
            .data_v  = (void*)inbuff,
        };
        int retlen = make_datagram_pack2buff(sock->datagram_buffer_tx, BUFLEN, &pack);

        if (retlen > 0) {
            len    = retlen;
            inbuff = sock->datagram_buffer_tx;
        } else {
            printf("datagram pack make error!!\n");
            return -1;
        }
    }

    while (curlen < len) {
        char* pos   = (char*)inbuff;
        int   sdlen = len - curlen > maxsnd ? maxsnd : len - curlen;
        int   ret   = _bb_socket_write(sock, pos + curlen, sdlen, timeout > 0 ? &tp : NULL);

        if (ret < 0) {
            if (sock->datagram_mode) {
                return -1;
            }
            break;
        }
        curlen += ret;
        if (ret < sdlen) {
            break;
        }
    }
    if (sock->datagram_mode) {
        return len0;
    }

    return curlen;
}

static void wk_rd(SOCK_SESSION* sock, usbpack* pack, void* prv)
{
    if (!ringbuffer_len(&sock->rx_rb)) {
        return;
    }
    pthread_cond_broadcast(&sock->socv);
}

static int datagram_get_data(SOCK_SESSION* sock, void* outbuff, uint32_t len)
{
    int tmplen = ringbuffer_get_data(&sock->rx_rb, sock->datagram_buffer_rx, 0, BUFLEN);
    if (tmplen <= 0) {
        return -1;
    }

    int     usedlen = 0;
    usbpack packret;
    int     packflg = unpack_datagram_pack(sock->datagram_buffer_rx, tmplen, &usedlen, &packret);

    if (packflg) {
        // no pack
        return -1;
    }

    if (packret.datapack != sock->datagram_buffer_rx + ss_head_cnt()) {
        printf("datagram: data leak len = %d\n",
               (int32_t)(packret.datapack - sock->datagram_buffer_rx - ss_head_cnt()));
    }

    if (packret.datalen > len) {
        printf("datagram: recv buff is too short!! buffer len = %d , get data len = %d\n", len, packret.datalen);
        packret.datalen = len;
    }
    memcpy(outbuff, packret.datapack, packret.datalen);
    tmplen = ringbuffer_out(&sock->rx_rb, NULL, usedlen);
    if (tmplen != usedlen) {
        printf("error ringbuffer_out , usedlen = %d,get = %d\n", usedlen, tmplen);
    }
    return packret.datalen;
}

AR8030_API int bb_socket_read(int sockfd, void* outbuff, uint32_t len, int timeout)
{
    SOCK_SESSION* sock = socket_get_session(sockfd);
    if (!sock || sock->exit_flg) {
        return -1;
    }

    if ((sock->sock_flg & BB_SOCK_FLAG_RX) == 0) {
        return -1;
    }

    struct timespec tp;
    if (timeout > 0) {
        get_timespec_from_now(&tp, timeout);
    }

    int retlen = 0;
    int sta    = 0;
    pthread_mutex_lock(&sock->somtx);
    wake_up wu = {
        .wkf = wk_rd,
        .prv = NULL,
    };

    list_add(&wu.node, &sock->head);
    while (!sock->exit_flg) {
        if (!sock->datagram_mode) {
            // stream mode
            retlen = ringbuffer_out(&sock->rx_rb, outbuff, len);
            if (retlen > 0) {
                break;
            }
        } else {
            // datagram mode
            retlen = datagram_get_data(sock, outbuff, len);
            if (retlen > 0) {
                break;
            }
        }

        if (timeout > 0) {
            sta = pthread_cond_timedwait(&sock->socv, &sock->somtx, &tp);
            if (sta == ETIMEDOUT) {
                break;
            } else if (sta != 0) {
                printf("rd pthread_cond_timedwait err ret = %d\n", sta);
            }
        } else {
            pthread_cond_wait(&sock->socv, &sock->somtx);
        }
    }

    list_del(&wu.node);
    if (sock->exit_flg) {
        pthread_cond_broadcast(&sock->exitcv);
        retlen = -1;
    }
    pthread_mutex_unlock(&sock->somtx);

    if (sta == ETIMEDOUT) {
        return -1;
    }

    return retlen;
}

AR8030_API int bb_socket_close(int sockfd)
{
    SOCK_SESSION* sock = socket_get_session(sockfd);
    if (!sock) {
        return -1;
    }
    socket_del_fd(sockfd);

    pthread_mutex_lock(&sock->somtx);
    bs_trig_close(&sock->base);
    pthread_mutex_unlock(&sock->somtx);

    return 0;
}

struct querytmp {
    void* buff;
    int   sta;
    int   opt;
    int   rx_len;
};

static void wk_with_sp_fix_rxlen(SOCK_SESSION* sock, usbpack* pack, void* prv)
{
    unsigned char    opt  = (pack->reqid >> 16) & 0xff;
    struct querytmp* ptmp = (struct querytmp*)prv;
    if (opt != ptmp->opt) {
        return;
    }

    if (pack->sta != 0) {
        printf("query buff len err!!\n");
        ptmp->sta = pack->sta;
    } else if (pack->datalen != ptmp->rx_len) {
        printf("query buff len err!!\n");
        ptmp->sta = -EMSGSIZE;
    } else {
        if (pack->datalen) {
            memcpy(ptmp->buff, pack->data_v, pack->datalen);
        }
        ptmp->sta = 0;
    }
    pthread_cond_broadcast(&sock->socv);
}

static int bb_socket_com_opt(SOCK_SESSION* sock, void* value, int txlen, int rxlen, uint8_t opt, wkfun wkf)
{
    uint32_t req = 4 << 24 | opt << 16 | sock->sock_slot << 8 | sock->sock_port << 0;

    usbpack pack = {
        .reqid   = req,
        .datalen = txlen,
        .data_v  = value,
    };

    pthread_mutex_lock(&sock->somtx);
#define MAG_ERR (-1234)
    struct querytmp tmp = {
        .buff   = value,
        .sta    = MAG_ERR,
        .opt    = opt,
        .rx_len = rxlen,
    };

    wake_up wu = {
        .wkf = wkf,
        .prv = &tmp,
    };

    list_add(&wu.node, &sock->head);
    send_usbpack(sock->base.phd, &pack);
    while (!sock->exit_flg && tmp.sta == MAG_ERR) {
        pthread_cond_wait(&sock->socv, &sock->somtx);
    }

    list_del(&wu.node);
    if (sock->exit_flg) {
        pthread_cond_broadcast(&sock->exitcv);
        tmp.sta = -ECONNRESET;
    }
    pthread_mutex_unlock(&sock->somtx);
    return tmp.sta;
}

AR8030_API int bb_socket_ioctl(int sockfd, bb_sock_cmd_e cmd, void* value)
{
    SOCK_SESSION* sock = socket_get_session(sockfd);
    if (!sock || sock->exit_flg) {
        printf("bb_socket_ioctl sta err\n");
        return -1;
    }

    switch (cmd) {
    case BB_SOCK_QUERY_TX_BUFF_LEN:
        return bb_socket_com_opt(sock, value, 0, sizeof(QUERY_TX_OUT), so_query_len, wk_with_sp_fix_rxlen);
        break;
    case BB_SOCK_QUERY_RX_BUFF_LEN: {
        QUERY_RX_OUT* buf = (QUERY_RX_OUT*)value;
        buf->buff_len     = ringbuffer_len(&sock->rx_rb);
        return 0;
        break;
    } break;
    case BB_SOCK_READ_INV_DATA: {
        pthread_mutex_lock(&sock->somtx);
        ringbuffer_reset(&sock->rx_rb);
        pthread_mutex_unlock(&sock->somtx);
        return 0;
    } break;
    case BB_SOCK_SET_TX_LIMIT: {
        return bb_socket_com_opt(sock, value, 0, sizeof(BUFF_LIMIT), so_set_tx_limit, wk_with_sp_fix_rxlen);
    } break;
    case BB_SOCK_GET_TX_LIMIT: {
        return bb_socket_com_opt(sock, value, 0, sizeof(BUFF_LIMIT), so_get_tx_limit, wk_with_sp_fix_rxlen);
    } break;
    case BB_SOCK_TX_LEN_GET: {
        return bb_socket_com_opt(sock, value, 0, sizeof(uint64_t), BB_SOCK_TX_LEN_GET, wk_with_sp_fix_rxlen);
    } break;
    case BB_SOCK_TX_LEN_RESET: {
        return bb_socket_com_opt(sock, value, 0, 0, BB_SOCK_TX_LEN_RESET, wk_with_sp_fix_rxlen);
    } break;
    case BB_SOCK_IOCTL_ECHO: {
        return bb_socket_com_opt(sock, value, 4, 4, BB_SOCK_IOCTL_ECHO, wk_with_sp_fix_rxlen);
    } break;
    case BB_SOCK_ENC_SET: {
        bb_sock_upd_enc_t* enc = (bb_sock_upd_enc_t*)value;
        if (!enc || (enc->encrypt_en && enc->encrypt_mode >= BB_SOCK_ENCRYPT_MODE_MAX)) {
            return -EINVAL;
        }
        return bb_socket_com_opt(sock, value, sizeof(*enc), 0, BB_SOCK_ENC_SET, wk_with_sp_fix_rxlen);
    } break;
    case BB_SOCK_ENC_GET: {
        if (!value) {
            return -EINVAL;
        }
        return bb_socket_com_opt(sock, value, 0, sizeof(bb_sock_upd_enc_t), BB_SOCK_ENC_GET, wk_with_sp_fix_rxlen);
    } break;
    default:
        printf("bb_socket_ioctl unknown cmd = 0x%02x\n", cmd);
        return -2;
    }
}

AR8030_API int bb_socket_ioctl_enc_set(int sockfd, void* ipt, int ipt_size, void* opt, int opt_size)
{
    if (!ipt || ipt_size != (int)sizeof(bb_sock_upd_enc_t) || opt || opt_size != 0) {
        return -EINVAL;
    }
    return bb_socket_ioctl(sockfd, BB_SOCK_ENC_SET, ipt);
}

AR8030_API int bb_socket_ioctl_enc_get(int sockfd, void* ipt, int ipt_size, void* opt, int opt_size)
{
    if (ipt || ipt_size != 0 || !opt || opt_size != (int)sizeof(bb_sock_upd_enc_t)) {
        return -EINVAL;
    }
    return bb_socket_ioctl(sockfd, BB_SOCK_ENC_GET, opt);
}
