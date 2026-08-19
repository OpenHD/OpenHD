#include "session_ioctl.h"
#include "ar8030.h"
#include "bb_dev.h"
#include "ioctl_tab.h"
#include "session.h"
#include "usbpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct IOCTL_SESSION {
    BASE_SESSION     base;
    int              reqid;
    int              sta;
    void*            pbuf;
    bb_dev_handle_t* pdev;
} IOCTL_SESSION;

static int io_rpc_cb(struct BB_HANDLE* phd, struct usbpack* pack, struct BASE_SESSION* psess)
{
    IOCTL_SESSION* iose = container_of(psess, IOCTL_SESSION, base);
    iose->sta           = pack->sta;
    if (iose->pbuf && pack->datalen) {
        memcpy(iose->pbuf, pack->datapack, pack->datalen);
    }
    bs_weakup(psess);
    return 0;
}

static void io_deinit(struct BASE_SESSION* psess)
{
    IOCTL_SESSION* iose = container_of(psess, IOCTL_SESSION, base);
    printf("io: req %#x deinit\n", iose->reqid);
    iose->pdev->ioctl_sess = NULL;
    bs_fun.de_init(psess);
    free(psess);
}

static int io_type(void)
{
    return st_ioctl;
}

const BASE_FUN iofun = {
    .rdcb    = io_rpc_cb,
    .de_init = io_deinit,
    .tpid    = io_type,
};

static void io_init(IOCTL_SESSION* iosess, struct BB_HANDLE* phd)
{
    bs_init(&iosess->base, phd, &iofun);

    iosess->reqid = 0;
}

struct IOCTL_SESSION* get_new_ioctl(bb_dev_handle_t* pdev)
{
    if (pdev->ioctl_sess) {
        BASE_SESSION* base = (BASE_SESSION*)pdev->ioctl_sess;
        if (base && base->fun->tpid() == io_type()) {
            return (IOCTL_SESSION*)base;
        }
        printf("err no session in hdl\n");
    }

    struct BB_HANDLE* phd = bb_gethandle(pdev);

    if (!phd) {
        return NULL;
    }

    IOCTL_SESSION* bbio = (IOCTL_SESSION*)malloc(sizeof(IOCTL_SESSION));
    io_init(bbio, phd);

    pdev->ioctl_sess = bbio;
    bbio->pdev       = pdev;

    return bbio;
}

int ioctl_bb_ioctl(bb_dev_handle_t* pdev, uint32_t request, const void* input, void* output, int timeout)
{
    int iptlen = get_bb_ioctl_cmdiptlen(request);
    if (iptlen < 0) {
        printf("req %x not found \n", request);
        return -2;
    }

    IOCTL_SESSION* iose = get_new_ioctl(pdev);
    if (!iose) {
        printf("can't connect to remote\n");
        return -1;
    }

    iose->reqid = request;
    iose->pbuf  = output;

    usbpack pack = {
        .reqid   = request,
        .data_v  = (void*)input,
        .datalen = iptlen,
    };

    int ret = bs_send_usbpack_and_wait(&iose->base, &pack, timeout);

    if (ret) {
        printf("ioctl fail , remote exit\n");
        return -1;
    }

    return iose->sta;
}
