#ifndef __NORMAL_NODE_H__
#define __NORMAL_NODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "base_node.h"
#include "rpc_node.h"
struct nor_dev;
typedef struct nor_rpc {
    struct rpc_node base;
    struct nor_dev* nodev;
} nor_rpc;

typedef struct nor_dev {
    basenode         base;
    struct rpc_node* prpc; ///< 下属节点
    REQID            reqid;
    int              need_write;
    int              init_flg;

    uint8_t  cmd_buff[2048];
    uint32_t cmd_len;
    uint8_t  ret_buff[2048];
    uint32_t ret_len;
    int      cur_msgid;
} nor_dev;

#ifdef __cplusplus
}
#endif

#endif
