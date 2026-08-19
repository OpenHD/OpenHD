#ifndef __DEV_NODE_LIST_H__
#define __DEV_NODE_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

// #define HAVE_STRUCT_TIMESPEC
#include "bb_api.h"
#include "list.h"
#include <pthread.h>

typedef int (*dev_wr)(void* handle, unsigned char* buff, int len, int timeout);

typedef int (*dev_rd_cb)(void* priv, unsigned char* buff, int len);
typedef int (*dev_evloop)(void* devhandle);
typedef int (*dev_name)(void* devhandle, char* buff, int len);

typedef struct dev_node_list dev_node_list;
typedef void (*dev_alive)(dev_node_list* plist, void* priv);

struct basenode;
typedef struct rpc_info rpc_info;

enum running_sta {
    run_rpc = 1 << 0,
    run_dev = 1 << 1,

    run_all = ~0,
};

enum dev_close_sta {
    sta_close_idle = 0,
    sta_need_close = 1 << 0, ///< 需要关闭
    sta_close_ok   = 1 << 1, ///< 关闭完成
};

typedef struct dev_node_list {
    pthread_mutex_t mtx;
    pthread_cond_t  cv;

    pthread_mutexattr_t mtx_attr;

    int readers;

    pthread_t t_send;
    pthread_t t_recv;

    void*      devhandle;
    dev_wr     dev_send;
    dev_rd_cb  dev_rd_cb;
    void*      recv_priv;
    dev_evloop dev_loop;
    dev_name   devname;

    dev_alive devalive;
    void*     devalive_priv;

    int remote_buff_len;
    int remote_buff_max_payload;

    int remote_max_event_id;

    bb_mac_t mac; 

    int              running;
    struct list_head work_list;
    struct list_head hard_list;

    int scanflg;

    // 产生临时msgid
    pthread_spinlock_t msgid_lk;
    int                msgid_tmp;
} dev_node_list;

/**
 * @brief 返回1表示找到 0表示没有找到
 *
 */
typedef int (*finder)(struct basenode* pnod, void* par);

void             plist_insert_node(dev_node_list* plist, struct basenode* nod);
struct basenode* plist_find_work_node(dev_node_list* plist, finder fd, void* fdpar);
void             plist_del_nod(dev_node_list* plist, struct basenode* nod);

void dev_node_list_init(dev_node_list* plist);
int  dev_node_list_get_buff(dev_node_list* plist);
void dev_node_tx_inotify(dev_node_list* plist);
int  dev_node_list_check_running(dev_node_list* plist);
void dev_node_force_exit(dev_node_list* plist);
int  dev_node_get_tmp_msgid(dev_node_list* plist);

#ifdef __cplusplus
}
#endif

#endif
