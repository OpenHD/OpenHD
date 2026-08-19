#ifndef __DEV8030_H__
#define __DEV8030_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bb_api.h"
#include "list.h"
#include <stdint.h>

typedef struct rpc_info rpc_info;

typedef int (*exitflg)(void* upar);

typedef struct dev8030_act dev8030_act;
typedef struct dev8030     dev8030;

typedef int   (*devrpc_poll)(dev8030* pdev);
typedef void* (*devrpc_get_plist)(dev8030* pdev, uint32_t workid);
typedef int   (*devrpc_getsz)(dev8030* pdev);
typedef int   (*devrpc_getid_all)(dev8030* pdev, uint32_t* ids, size_t sz);
typedef int   (*devrpc_fill_serial)(dev8030* pdev, uint32_t workid, uint8_t* buff, size_t len);
typedef int   (*devrpc_ctrl)(dev8030* pdev, uint32_t reqid, void* input, int iptlen, void* output, int* optlen);

typedef struct dev8030_act {
    const char*        name;
    devrpc_poll        polldev;
    devrpc_get_plist   getplist;
    devrpc_getsz       getsz;
    devrpc_getid_all   getid_all;
    devrpc_fill_serial fill_serial;
    devrpc_ctrl        opt_ctrl;
} dev8030_act;

typedef struct dev8030 {
    struct dev8030_act const* pact;
    rpc_info*                 pinfo;
    struct list_head          node;
} dev8030;

int reg_usbrpc_platfrom(rpc_info* prpc);
int reg_sdiorpc_platform(rpc_info* prpc, const char** devs, int dev_cnt);
int reg_uartrpc_platform(rpc_info* prpc, char* devs, uart_par* par);
int reg_drvrpc_platform(rpc_info* prpc, const char** devs, int dev_cnt);
int reg_usbrpc_hostjnimode_platform(rpc_info* prpc);
int dev8030_poll(rpc_info* prpc);

#ifdef __cplusplus
}
#endif

#endif