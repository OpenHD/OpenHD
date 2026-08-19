#ifndef __CALLBACK_NODE_H__
#define __CALLBACK_NODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "base_node.h"
#include "list.h"
#include "rpc_node.h"

struct rpc_node;

typedef enum {
    cb_not_init = 0,
    cb_need_send_cmd,
    cb_wait_usb_cmd,
    cb_wait_usb_data,

    cb_need_send_close,
    cb_wait_usb_close,
} CB_STATUS;

struct cb_dev;
typedef struct cb_rpc {
    struct rpc_node base;
    struct cb_dev*  cbdev;
    int             initflag;

    struct list_head lists;
} cb_rpc;

typedef struct cb_dev {
    basenode  base;
    uint16_t  cbevt;
    CB_STATUS cb_status;

    struct list_head head; // 下属链表
    pthread_mutex_t  mtx;

    int exitflg;

    char names[128];
} cb_dev;

struct dev_node_list;
void add_base_callback(struct dev_node_list* plist, int maxnum);

#ifdef __cplusplus
}
#endif

#endif
