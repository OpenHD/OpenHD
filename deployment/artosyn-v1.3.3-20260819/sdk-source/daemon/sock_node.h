#ifndef __SOCK_NODE_H__
#define __SOCK_NODE_H__

#include <stdint.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "base_node.h"
#include "com_cfg.h"
#include "list.h"
#include "ringbuffer.h"
#include "rpc_node.h"
#include "usbpack.h"
#include <pthread.h>
struct rpc_node;

typedef enum {
    sock_not_init = 0,
    sock_need_send_cmd,
    sock_wait_usb_cmd,

    sock_can_send_data,
    sock_wait_usb_data,

    sock_need_send_close,
    sock_wait_usb_close,
} SOCK_STATUS;

typedef enum {
    TX_FLG = 1 << 0,
    RX_FLG = 1 << 1,
} SOCKET_CONF;

typedef struct {
    int              tx_len;
    uint8_t          tx_buff[2048];
    int              opt;
    int              txflg;
    struct list_head node;
} socket_ioctl_ctl;

struct sock_rpc;
typedef struct sock_dev {
    basenode         base;
    struct sock_rpc* sockrpc; // 下属节点
    REQID            reqid;
    SOCK_STATUS      sock_sta;
    int              has_opened;

    uint32_t sock_cmd_flg; ///< @ref SOCKET_CONF
    uint32_t rx_buff_len;
    uint32_t tx_buff_len;
    uint8_t  encrypt_mode;
    uint8_t  key[32];
    int      slot;
    int      port;

    ringbuffer_t buf_dev;        ///< 这个发送给8030
    uint32_t     buf_dev_limit;  ///< ringbuff写入限制
    uint64_t     buf_wr_index;   ///< 写入位置
    uint64_t     buf_head_index; ///< rb 位置
    uint32_t     max_payload_len;

    pthread_mutex_t mtx_tx_buf;

    // rpc-8030 cmd list
    struct list_head sock_ioctl_list;
    int              ioctl_act_flg;

    uint8_t _buf_dev[500 * 1024];

    char nameflg;
    char names[128];
} sock_dev;

struct rpc_node;
#define RX_SAVEBUFFER_LEN (SOCK_LEN_APP_TO_DAEMON * 2)

typedef struct {
    struct list_head node;
    uint64_t         wr_init; ///< socket_write发送目标
    uint64_t         wr_max;  ///< 用户层调用bb_socket_write时 wr_cpl_max起点
} sock_wr_node;

typedef struct {
    int              datalen;
    struct list_head node;
} rpc_send_group;

typedef struct sock_rpc {
    struct rpc_node base;
    sock_dev*       sockdev;

    pthread_t       wr_thread;
    pthread_mutex_t mtx;
    pthread_cond_t  cv;
    int             workflg;

    // rpc返回命令
    struct list_head send_cmd_list_head;

    int rx_savemax;
    int rx_savelen; ///< rx线程内rx_savebuffer位置

    struct list_head sock_wr_list; /// 用于保存发送数据的位置
    uint64_t         wr_cur_ptr;   ///< socket_write 当前位置

    uint64_t rd_pos_init; ///< socket_read ringbuffer起始位置

    uint8_t rx_savebuffer[RX_SAVEBUFFER_LEN];
} sock_rpc;

sock_dev* sock_node_alloc(void);
int       sock_node_start(sock_dev* psock, sock_rpc* prpc);
int       sock_dev_push_data(sock_dev* psock, sock_rpc* priv, uint8_t* buff, int len);
int       sock_rpc_push_read_data(sock_rpc* prpc, unsigned char* buff, int len);
void      sock_rpc_send_msg(struct sock_rpc* rpc, usbpack* pack);

#ifdef __cplusplus
}
#endif

#endif
