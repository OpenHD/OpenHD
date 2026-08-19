#ifndef __RPC_NODE_H__
#define __RPC_NODE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "dev8030.h"
#include "list.h"
#include "rpc_dev_bind.h"
#include "socketfd_port.h"
#include <pthread.h>
struct rpc_node;
struct dev_node_list;
typedef struct rpc_info rpc_info;
typedef int (*rpc_read_cb)(struct rpc_node*, unsigned char* buff, int len);
typedef int (*rpc_end)(struct rpc_node*);

typedef struct {
    rpc_end     end;
    rpc_read_cb rpc_rd_cb;
} rpc_act;

typedef struct threadinfo {
    rpc_info*             prpc_info; ///< 指向设备控制
    struct dev_node_list* plist;     ///< 指向单个usb
    SOCKETFD              fd;
    pthread_t             rd_thread;
} threadinfo;

typedef struct rpc_node {
    threadinfo* tinfo;
    NODE_TYPE   nodetype;
    rpc_act*    pact;
} rpc_node;

typedef struct {
    SOCKETFD         listen_socket;
    pthread_t        listen_thread;
    int              id;
    struct rpc_info* pinfo;
    struct list_head node;
} rpc_listen_node;

typedef struct rpc_info {
    struct list_head rpc_listen_head; ///< 监听列表
    pthread_rwlock_t rpc_listen_lk;
    int              rpc_listen_nxt_id;

    struct list_head head; ///< 控制设备链表 下挂dev8030
    pthread_rwlock_t head_lk;

    int nxt_working_id;

    struct list_head head_hotplug_cb;    ///< 用于通知上下线链表
    pthread_rwlock_t head_hotplug_lk;
} rpc_info;

rpc_info* rpc_init(int port[], int portlen, const char* udsname);
void      rpc_close_listen(rpc_info* pinfo);

void  rpc_dev_add(rpc_info* prpc_info, dev8030* pdev);
int   rpc_dev_getsz(rpc_info* prpc_info);
int   rpc_dev_getid_all(rpc_info* prpc_info, uint32_t* ids, size_t sz);
int   rpc_dev_fill_serial(rpc_info* prpc_info, uint32_t workid, uint8_t* buff, size_t len);
void* rpc_dev_get_plist(rpc_info* prpc_info, uint32_t workid);
int   rpc_dev_get_nxt_working_id(rpc_info* prpc_info);

dev8030* rpc_get_plat_by_name(rpc_info* prpc_info, const char* name);
#ifdef __cplusplus
}
#endif

#endif
