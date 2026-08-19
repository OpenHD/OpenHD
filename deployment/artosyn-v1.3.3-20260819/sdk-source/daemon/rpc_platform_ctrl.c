

#include "bb_api.h"
#include "com_log.h"
#include "rpc_dev_bind.h"
#include "rpc_node.h"
#include "usbpack.h"
#include <stdio.h>

static int uart_ctrl(struct threadinfo* tinfo, usbpack* pack)
{
    dev8030* puart = rpc_get_plat_by_name(tinfo->prpc_info, "uart");

    unsigned char buff[4096];
    unsigned char retdat[1024];
    int           outlen = 0;
    int           sta    = -1;
    if (puart && puart->pact->opt_ctrl) {
        sta = puart->pact->opt_ctrl(puart, pack->reqid, pack->data_v, pack->datalen, retdat, &outlen);
    } else {
        com_log(COM_BASE_COM, "can't get list");
    }

    usbpack retpack = {
        .sta     = sta,
        .reqid   = pack->reqid,
        .datalen = outlen,
        .data_v  = retdat,
    };
    int sdlen = make_usbpack2buff(buff, 4096, &retpack);

    send(tinfo->fd, (char*)buff, sdlen, 0);
    return 0;
}

void com_log_set_level(char* name, int level);

static int daemon_set_dbg(struct threadinfo* tinfo, usbpack* pack)
{
    daemon_dbg_ctrl* pctrl = pack->data_v;

    com_log_set_level(pctrl->names, pctrl->level);

    usbpack retpack = {
        .sta     = 0,
        .reqid   = pack->reqid,
        .datalen = 0,
        .data_v  = 0,
    };
    unsigned char buff[4096];

    int sdlen = make_usbpack2buff(buff, 4096, &retpack);
    send(tinfo->fd, (char*)buff, sdlen, 0);

    return 0;
}

int com_log_get_level(char* name);

static int daemon_get_dbg(struct threadinfo* tinfo, usbpack* pack)
{
    daemon_dbg_ctrl* pctrl = pack->data_v;

    pctrl->level = com_log_get_level(pctrl->names);

    usbpack retpack = {
        .sta     = 0,
        .reqid   = pack->reqid,
        .datalen = sizeof(daemon_dbg_ctrl),
        .data_v  = pctrl,
    };
    unsigned char buff[4096];

    int sdlen = make_usbpack2buff(buff, 4096, &retpack);
    send(tinfo->fd, (char*)buff, sdlen, 0);

    return 0;
}

static int daemon_exec(struct threadinfo* tinfo, usbpack* pack)
{
    pack->datapack[pack->datalen] = 0;

#if WIN32
    FILE* fp = _popen((char*)pack->datapack, "wr");
#else
    FILE* fp = popen((char*)pack->datapack, "wr");
#endif

#define MAX_LEN 2048
    char retcmd[MAX_LEN];
    int  cmdoffset = 0;

    do {
        int rdlen = read(fileno(fp), retcmd + cmdoffset, MAX_LEN - cmdoffset);

        if (rdlen <= 0) {
            break;
        }
        cmdoffset += rdlen;

        if (cmdoffset >= MAX_LEN) {
            break;
        }

    } while (1);

    fclose(fp);

    usbpack retpack = {
        .sta     = 0,
        .reqid   = pack->reqid,
        .datalen = cmdoffset,
        .data_v  = retcmd,
    };
    unsigned char buff[4096];

    int sdlen = make_usbpack2buff(buff, 4096, &retpack);
    send(tinfo->fd, (char*)buff, sdlen, 0);

    return 0;
}

int rpc_plat_proc(struct threadinfo* tinfo, usbpack* pack)
{
    switch (pack->reqid) {
    case BB_RPC_SERIAL_SETUP:
        return uart_ctrl(tinfo, pack);
    case BB_RPC_SERIAL_LIST:
        return uart_ctrl(tinfo, pack);
    case BB_RPC_SET_DEBUG_LV:
        return daemon_set_dbg(tinfo, pack);
    case BB_RPC_GET_DEBUG_LV:
        return daemon_get_dbg(tinfo, pack);
    case BB_RPC_HOST_EXEC:
        return daemon_exec(tinfo, pack);
    default: {
        unsigned char buff[4096];
        usbpack       retpack = {
                  .sta     = -1,
                  .reqid   = pack->reqid,
                  .datalen = 0,
        };
        int sdlen = make_usbpack2buff(buff, 4096, &retpack);
        com_log(COM_BASE_COM, "unknwon plat opt = %x\n", pack->reqid);
    } break;
    }

    return -1;
}
