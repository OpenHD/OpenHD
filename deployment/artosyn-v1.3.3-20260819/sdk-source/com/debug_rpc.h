#ifndef __DEBUG_RPC_H__
#define __DEBUG_RPC_H__
#include <stdint.h>
#include "bb_api.h"

#ifdef __cplusplus
extern "C" {
#endif

struct dbg_hdl;
struct bb_dev_handle_t;

typedef void (*dbg_recv)(struct dbg_hdl* hdl, void* priv, unsigned char* buff, int len);
AR8030_API void dbg_write(struct dbg_hdl* hdl, unsigned char* buff, int len);
AR8030_API struct dbg_hdl* dbg_setup(struct bb_dev_handle_t* pdev, dbg_recv recv, void* priv, int timeout);

#ifdef __cplusplus
}
#endif

#endif
