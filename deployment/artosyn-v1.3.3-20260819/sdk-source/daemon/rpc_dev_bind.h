#ifndef __RPC_DEV_BIND_H__
#define __RPC_DEV_BIND_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rpc_node;
struct threadinfo;

enum {
    RPC_CHK_SUC = 0,
    RPC_CHK_NXT = -1,
};

typedef struct rpc_node* (*NODE_START)(struct threadinfo* tinfo);
typedef int (*RPC_CHK)(unsigned char* buff, int len, struct threadinfo* tinfo);

typedef int (*RPC_RECV)(struct rpc_node*, char* buff, int len);

typedef enum NODE_TYPE {
    nod_not_init = -1,
    nod_normal   = 0,
    nod_callback = 1,
    nod_socket   = 2,
    nod_debug    = 3,
    nod_hardware = 4,
    nod_max      = 5,
} NODE_TYPE;

typedef struct {
    NODE_TYPE  nodetype;
    NODE_START start; ///< 0成功 其他失败
    RPC_CHK    rpc_chk;
} NODE_INFO;

int rpc_recv_chk_init(unsigned char* buff, int len, struct threadinfo* tinfo, struct rpc_node**);
#ifdef __cplusplus
}
#endif

#endif
