#ifndef __BB_DEV_H__
#define __BB_DEV_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"
#include <pthread.h>
#include <stdint.h>

typedef struct cb_list cb_list;

typedef struct CB_SESSION CB_SESSION;

typedef int(cb_chk)(CB_SESSION* pcb, void* par);

typedef struct bb_host_t {
    int  port;
    char remote_addr[128];
#ifdef ENABLE_UDS
    char uds_addr[128];
#endif
    struct list_head open_handle_list; // 打开的列表

    struct list_head hotplug_change_cb;
    pthread_rwlock_t hotplug_change_lk;
} bb_host_t;

typedef struct bb_dev_t {
    uint32_t   id;
    bb_host_t* phost;
} bb_dev_t;

typedef struct bb_dev_tmp_t {
    void*    ptr;
    bb_dev_t dat;
} bb_dev_tmp_t;

struct bb_dev_list {
    int                 dev_num;
    struct bb_dev_tmp_t dev_list[0];
};

typedef struct bb_dev_handle_t {
    bb_host_t*       phost;
    struct list_head bb_dev_handle_list;

    void*            ioctl_sess;
    struct list_head cblshead;
    pthread_mutex_t  cbmtx;
    pthread_cond_t   cbcv;

    pthread_mutex_t ioctl_lk;

    uint32_t sel_id;
} bb_dev_handle_t;

CB_SESSION* dev_find_node(bb_dev_handle_t* pdev, cb_chk chk, int evtid);
void        dev_del_nod(bb_dev_handle_t* pdev, cb_chk chk, struct CB_SESSION* pcb);
void        dev_insert_session(bb_dev_handle_t* pdev, struct CB_SESSION* cbsess);
#ifdef __cplusplus
}
#endif
#endif
