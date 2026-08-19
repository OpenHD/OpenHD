#ifndef __BASE_NODE_H__
#define __BASE_NODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"
#include "rpc_dev_bind.h"
#include <pthread.h>
#include <stdint.h>

struct usbpack;
struct basenode;

typedef enum {
    proc_nxt = 0,
    proc_already,
} PROC_RET;

typedef PROC_RET (*bf_pack_chk)(struct basenode*, struct usbpack*);
typedef int (*bf_pack_proc)(struct basenode*, struct usbpack*);
typedef int (*bf_need_write)(struct basenode*);
typedef int (*bf_pack_make)(struct basenode*, unsigned char* buff, int len);
typedef int (*bf_deinit)(struct basenode*);
typedef int (*bf_close)(struct basenode*);
typedef char* (*bf_names)(struct basenode*);

typedef union {
    struct {
        uint32_t subcmdid : 24;
        uint32_t domainid : 8;
    };
    uint32_t reqid;
} REQID;

struct dev_node_list;

typedef struct {
    bf_pack_chk   dev_pack_chk;
    bf_pack_proc  dev_pack_proc;
    bf_need_write dev_write_able;
    bf_pack_make  dev_pack_make;
    bf_deinit     dev_deinit;
    bf_close      dev_cls;
    bf_names      dev_nm;
} baseact;

typedef void (*cls_cb)(struct basenode* pbn, void* par);

void bn_init(struct basenode* pbn, struct dev_node_list* plist, NODE_TYPE tp, const baseact* act);
void bn_set_free(struct basenode* pbn);
void bn_force_close_inotify(struct basenode* pbn);
void bn_set_close_cb(struct basenode* pbn, cls_cb clscb, void* clbpriv);
void bn_tx_inotify(struct basenode* pbn);

extern baseact bnact;

typedef struct basenode {
    struct list_head node;
    NODE_TYPE        tp;

    struct dev_node_list* plist;

    int            freeflag; ///< 给devlist表示需要调用dev_deinit
    baseact const* pact;     ///< 这个必须写

    void* exitpar;

    struct list_head cls_node;
    cls_cb           clscb;
    void*            clbpriv;
} basenode;

typedef enum {
    dev_not_init           = 0,
    dev_keep_alive         = 1,
    dev_get_remote_bufflen = 2,
    dev_normal_work        = 3,
} dev_init_seq;

#ifdef __cplusplus
}
#endif

#endif
