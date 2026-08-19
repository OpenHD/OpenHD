#include "ar8030.h"
#include "bb_api.h"
#include "bb_dev.h"
#include "session.h"
#include "usbpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _hotplug_session {
    struct list_head  node;
    BASE_SESSION      base;
    void*             priv;
    bb_event_callback cb;
    int               initflg;
} hotplug_session;

static int hotplug_rpc_cb(struct BB_HANDLE* phd, struct usbpack* pack, struct BASE_SESSION* psess)
{
    hotplug_session* phot = container_of(psess, hotplug_session, base);
    if (!pack->datalen) {
        bs_weakup(psess);
        return 0;
    }

    bb_event_hotplug_t plugdat;
    memcpy(&plugdat, pack->data_v, sizeof(plugdat));

    if (phot->cb) {
        phot->cb(&plugdat, phot->priv);
    }

    return 0;
}

static void hotplug_deinit(struct BASE_SESSION* psess)
{
    hotplug_session* hotsess = container_of(psess, hotplug_session, base);
    printf("hotplug deinit\n");
    list_del(&hotsess->node);
    bs_fun.de_init(psess);
    free(hotsess);
}

static int hotplug_type(void)
{
    return st_hotplug;
}

static const BASE_FUN hotplug_fun = {
    .rdcb    = hotplug_rpc_cb,
    .de_init = hotplug_deinit,
    .tpid    = hotplug_type,
};

AR8030_API int bb_dev_reg_hotplug_cb(bb_host_t* phost, bb_event_callback cbfun, void* priv)
{
    struct BB_HANDLE* phd  = bb_gethandle_from_host(phost);
    hotplug_session*  phot = (hotplug_session*)malloc(sizeof(hotplug_session));

    if (!phd || !phot) {
        return -1;
    }

    bs_init(&phot->base, phd, &hotplug_fun);
    phot->initflg = 0;
    phot->cb      = cbfun;
    phot->priv    = priv;

    pthread_rwlock_wrlock(&phost->hotplug_change_lk);
    list_add_tail(&phot->node, &phost->hotplug_change_cb);
    pthread_rwlock_unlock(&phost->hotplug_change_lk);

    usbpack pack = {
        .reqid = BB_RPC_GET_HOTPLUG_EVENT,
    };

    int ret = bs_send_usbpack_and_wait(&phot->base, &pack, -1);
    if(ret) {
        printf("hotplug fail , remote exit!\n");
    }

    return ret;
}

void bb_dev_del_all_hotplug_cb(bb_host_t* phost)
{
    pthread_rwlock_wrlock(&phost->hotplug_change_lk);
    while (!list_empty(&phost->hotplug_change_cb)) {
        hotplug_session* phot = list_first_entry(&phost->hotplug_change_cb, hotplug_session, node);
        bs_trig_close(&phot->base);
        list_del(&phot->node);
    }
    pthread_rwlock_unlock(&phost->hotplug_change_lk);
}
