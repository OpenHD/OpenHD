#include "bb_api.h"
#include "debug_rpc.h"
#include "list.h"
#include "session.h"
#include "usbpack.h"
#include <stdint.h>
#include <stdlib.h>

struct dbg_hdl {
    BASE_SESSION            base;
    dbg_recv                recb;
    struct bb_dev_handle_t* pdev;
    void*                   priv;
};

static int dbg_rpc_cb(struct BB_HANDLE* phd, struct usbpack* pack, struct BASE_SESSION* psess)
{
    struct dbg_hdl* dbg = container_of(psess, struct dbg_hdl, base);

    if (dbg->recb) {
        if (pack->datalen == 0) {
            bs_weakup(&dbg->base);
        }
        dbg->recb(dbg, dbg->priv, pack->datapack, pack->datalen);
    }

    return 0;
}

static void dbg_deinit(struct BASE_SESSION* psess)
{
    struct dbg_hdl* dbg = container_of(psess, struct dbg_hdl, base);
    bs_fun.de_init(psess);
    free(dbg);
}

static int dbg_type(void)
{
    return st_ioctl;
}

const BASE_FUN dbgfun = {
    .rdcb    = dbg_rpc_cb,
    .de_init = dbg_deinit,
    .tpid    = dbg_type,
};

static void debug_init(struct dbg_hdl* dbg, struct BB_HANDLE* phd, dbg_recv recv)
{
    bs_init(&dbg->base, phd, &dbgfun);
    dbg->recb = recv;
}

void dbg_write(struct dbg_hdl* hdl, unsigned char* buff, int len)
{
    if (!hdl || !hdl->base.phd || !buff || len <= 0) {
        return;
    }

    usbpack pack = {
        .data_v   = buff,
        .datalen  = len,
        .domainid = BB_REQ_DBG,
        .subcmdid = BB_DBG_DATA,
    };

    send_usbpack(hdl->base.phd, &pack);
}

struct dbg_hdl* dbg_setup(struct bb_dev_handle_t* pdev, dbg_recv recv, void* priv, int timeout)
{
    struct BB_HANDLE* phd = bb_gethandle(pdev);
    if (!phd) {
        return NULL;
    }

    struct dbg_hdl* dbg = (struct dbg_hdl*)malloc(sizeof(struct dbg_hdl));
    if (!dbg) {
        return NULL;
    }

    debug_init(dbg, phd, recv);

    usbpack testpack = {
        .domainid = BB_REQ_DBG,
        .data_v   = NULL,
        .datalen  = 0,
    };

    int ret = bs_send_usbpack_and_wait(&dbg->base, &testpack, timeout);

    do {
        if (!ret) {
            return dbg;
        }
    } while (0);

    bs_trig_close(&dbg->base);

    return NULL;
}
