#include "session_callback.h"
#include "bb_api.h"
#include "bb_dev.h"
#include "com_cfg.h"
#include "usbpack.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct CB_SESSION {
    BASE_SESSION            base;
    bb_set_event_callback_t pcb;
    struct bb_dev_handle_t* pdev;
    int                     staflg;
    struct list_head        node;
} CB_SESSION;

CB_SESSION* get_cb_from_node(struct list_head* phead)
{
    return container_of(phead, CB_SESSION, node);
}

BASE_SESSION* get_bs_from_node(struct list_head* phead)
{
    return &container_of(phead, CB_SESSION, node)->base;
}

struct list_head* get_node_from_cb(CB_SESSION* pcb)
{
    return &pcb->node;
}

static int cb_type(void)
{
    return st_callback;
}

static int cb_rpc_cb(struct BB_HANDLE* phd, struct usbpack* pack, struct BASE_SESSION* psess)
{
    CB_SESSION* cbsession = container_of(psess, CB_SESSION, base);

    if (cbsession->staflg != 0) {
        cbsession->staflg = pack->sta;
        bs_weakup(&cbsession->base);
    } else {
        if (cbsession->pcb.callback) {
            int retcode = (pack->reqid >> 16) & 0xff;
            if (retcode == SUBSCRIBE_DAT_RET) {
                cbsession->pcb.callback(pack->datapack, cbsession->pcb.user);
            } else {
                printf("call back code error = %d\n", retcode);
            }
        }
    }
    return 0;
}

static int del_session(CB_SESSION* testcb, void* par)
{
    if (testcb && testcb->base.fun->tpid() == cb_type()) {
        if (testcb == par) {
            return 1;
        }
    }
    return 0;
}

static void cb_deinit(struct BASE_SESSION* psess)
{
    CB_SESSION* cbsession = container_of(psess, CB_SESSION, base);
    printf("io: evt %d deinit\n", cbsession->pcb.event);
    dev_del_nod(cbsession->pdev, del_session, cbsession);
    bs_fun.de_init(psess);
    free(psess);
}

static void offline_cb_deinit(struct BASE_SESSION* psess)
{
    CB_SESSION* cbsession = container_of(psess, CB_SESSION, base);
    printf("io: dev offline\n");

    if (cbsession->pcb.callback) {
        cbsession->pcb.callback(0, cbsession->pcb.user);
    }
    dev_del_nod(cbsession->pdev, del_session, cbsession);
    bs_fun.de_init(psess);
    free(psess);
}

const BASE_FUN cbfun = {
    .rdcb    = cb_rpc_cb,
    .de_init = cb_deinit,
    .tpid    = cb_type,
};

const BASE_FUN offline_cbfun = {
    .rdcb    = cb_rpc_cb,
    .de_init = offline_cb_deinit,
    .tpid    = cb_type,
};

static CB_SESSION* get_new_cb(bb_dev_handle_t* pdev, bb_set_event_callback_t* pcb)
{
    struct BB_HANDLE* phd = bb_gethandle(pdev);

    if (!phd) {
        return NULL;
    }
    CB_SESSION* cbsession = (CB_SESSION*)malloc(sizeof(struct CB_SESSION));
    if (pcb->event == BB_EVENT_OFFLINE) {
        bs_init(&cbsession->base, phd, &offline_cbfun);
    } else {
        bs_init(&cbsession->base, phd, &cbfun);
    }

    cbsession->pcb.callback = pcb->callback;
    cbsession->pcb.event    = pcb->event;
    cbsession->pcb.user     = pcb->user;
    cbsession->staflg       = -1;
    cbsession->pdev         = pdev;

    return cbsession;
}

static int create_new_cb(bb_dev_handle_t* pdev, bb_set_event_callback_t* pcb, int timeout)
{
    CB_SESSION* cbsession = get_new_cb(pdev, pcb);

    if (!cbsession) {
        return -1;
    }

    uint32_t setrequest = BB_REQ_CB << 24 | SUBSCRIBE_REQ << 16 | pcb->event;

    usbpack pack = {
        .reqid = setrequest,
    };

    int ret = bs_send_usbpack_and_wait(&cbsession->base, &pack, timeout);
    if (ret) {
        printf("callback init err , remote exit\n");
        return -1;
    }

    if (cbsession->staflg != 0) {
        //
        printf("callback init err = %d\n", cbsession->staflg);
        cbfun.de_init(&cbsession->base);
        return -1;
    }

    dev_insert_session(pdev, cbsession);

    return 0;
}

static int fd_session(CB_SESSION* pcb, void* par)
{
    int* evtid = par;

    if (pcb->pcb.event == *evtid) {
        return 1;
    }
    return 0;
}

int cb_bb_ioctl(bb_dev_handle_t* pdev, uint32_t request, const void* input, int timeout)
{
    bb_set_event_callback_t* inptcb = (bb_set_event_callback_t*)input;

    CB_SESSION* pcb = dev_find_node(pdev, fd_session, inptcb->event);
    if (request == BB_SET_EVENT_SUBSCRIBE) {
        if (pcb) {
            // 更新 callback
            pcb->pcb.callback = inptcb->callback;
            pcb->pcb.user     = inptcb->user;
            return 0;
        } else {
            return create_new_cb(pdev, inptcb, timeout);
        }
    }

    if (request == BB_SET_EVENT_UNSUBSCRIBE) {
        if (pcb) {
            pcb->pcb.callback = NULL;
        }
        return 0;
    }
    return -1;
}
