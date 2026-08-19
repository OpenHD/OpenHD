#ifndef __RPC_CMDLINE_H__
#define __RPC_CMDLINE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bb_api.h"
#include <pthread.h>

void rpc_debug_reg(void* plist, bb_mac_t mac);

#ifdef __cplusplus
}
#endif

#endif