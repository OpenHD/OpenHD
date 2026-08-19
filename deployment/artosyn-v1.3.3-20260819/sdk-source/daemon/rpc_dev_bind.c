
#include "rpc_dev_bind.h"
#include "bb_api.h"
#include "com_log.h"
#include "hotplug_rpc.h"
#include "rpc_node.h"
#include "rpc_platform_ctrl.h"
#include "usbpack.h"
#include <stdlib.h>

extern NODE_INFO normal_info;
extern NODE_INFO cb_info;
extern NODE_INFO sock_info;
extern NODE_INFO cl_info;

NODE_INFO* info_arr[] = {
    &normal_info,
    &cb_info,
    &sock_info,
    &cl_info,
};

static void ret_test(struct threadinfo* tinfo)
{
    uint8_t tmpbuff[1024];
    usbpack tstpack = {
        .data_v  = NULL,
        .datalen = 0,
        .sta     = 0,
        .reqid   = BB_RPC_TEST,
        .msgid   = 0,
    };

    int len = make_usbpack2buff(tmpbuff, sizeof(tmpbuff), &tstpack);
    send(tinfo->fd, (char*)tmpbuff, len, 0);
}

static void get_list_proc(struct threadinfo* tinfo)
{
    uint8_t buff[1024];
    int     sz = rpc_dev_getsz(tinfo->prpc_info);

    uint32_t* plist = NULL;

    if (sz) {
        plist = malloc(sizeof(uint32_t) * sz);
        rpc_dev_getid_all(tinfo->prpc_info, plist, sz);
    }

    usbpack pack = {
        .datalen  = sz * sizeof(uint32_t),
        .sta      = sz,
        .reqid    = BB_RPC_GET_LIST,
        .datapack = (uint8_t*)plist,
    };

    int len = make_usbpack2buff(buff, sizeof(buff), &pack);

    send(tinfo->fd, (char*)buff, len, 0);

    if (plist) {
        free(plist);
    }
}

static void get_mac(struct threadinfo* tinfo, uint32_t workid)
{
    uint8_t buff[1024];
    uint8_t mac[128];
    int     sz = rpc_dev_fill_serial(tinfo->prpc_info, workid, mac, sizeof(mac));

    usbpack pack = {
        .datalen  = sz,
        .sta      = sz,
        .reqid    = BB_RPC_GET_MAC,
        .datapack = mac,
    };

    if (sz < 0) {
        pack.datalen = 0;
    }

    int len = make_usbpack2buff(buff, sizeof(buff), &pack);

    send(tinfo->fd, (char*)buff, len, 0);
}

/**
 * @brief 初始状态下 尝试接收数据
 *
 * @param prpc
 * @param buff
 * @param len
 * @return NODE_TYPE
 */
int rpc_recv_chk_init(unsigned char* buff, int len, struct threadinfo* tinfo, struct rpc_node** retnode)
{
    usbpack pack;
    int     ret = unpack_usb_pack(buff, len, 0, &pack);

    if (ret < 0) {
        return -1;
    }

    if (pack.reqid == BB_RPC_TEST) {
        ret_test(tinfo);
        return -2;
    }

    if (pack.reqid == BB_RPC_GET_HOTPLUG_EVENT) {
        *retnode = hotplug_cb_alloc(tinfo);
        return -2;
    }

    // 获取work id
    if (pack.reqid == BB_RPC_GET_LIST) {
        get_list_proc(tinfo);
        return -2;
    }

    // 获取mac
    if (pack.reqid == BB_RPC_GET_MAC) {
        get_mac(tinfo, pack.msgid);
        return -2;
    }

    if(pack.domainid == BB_REQ_PLAT_CTL) {
        rpc_plat_proc(tinfo, &pack);
        return -2;
    }


    // 必须先选择一个work id
    void* plist = rpc_dev_get_plist(tinfo->prpc_info, pack.msgid);
    if (!plist) {
        // 返回错误 没有指定的设备
        usbpack retpack = {
            .datalen = 0,
            .msgid   = pack.msgid,
            .sta     = -1,
            .reqid   = BB_RPC_SEL_ID,
        };
        uint8_t txbuff[1024];
        int     len = make_usbpack2buff(txbuff, sizeof(txbuff), &retpack);

        send(tinfo->fd, (char*)txbuff, len, 0);

        return -3;
    }

    // id check ok
    tinfo->plist = plist;

    if (pack.reqid == BB_RPC_SEL_ID) {
        usbpack retpack = {
            .datalen = 0,
            .msgid   = pack.msgid,
            .sta     = 0,
            .reqid   = BB_RPC_SEL_ID,
        };
        uint8_t txbuff[1024];
        int     len = make_usbpack2buff(txbuff, sizeof(txbuff), &retpack);

        send(tinfo->fd, (char*)txbuff, len, 0);

        return -4;
    }

    if (!tinfo->plist) {
        // no work id
        return -5;
    }

    int sz = sizeof(info_arr) / sizeof(NODE_INFO*);

    for (int i = 0; i < sz; i++) {
        NODE_INFO* pinfo = info_arr[i];

        int ret = pinfo->rpc_chk(buff, len, tinfo);

        if (ret == RPC_CHK_NXT) {
            continue;
        }

        if (ret) {
            return ret;
        }

        rpc_node* nod = pinfo->start(tinfo);
        if (!nod) {
            continue;
        }

        *retnode = nod;

        com_log(COM_NET, "tf@%p change to %d", tinfo, nod->nodetype);

        return 0;
    }
    return -6;
}
