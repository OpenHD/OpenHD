
#include "ar8030.h"
#include "bb_api.h"
#include "bb_dev.h"
#include "session.h"
#include "usbpack.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    BASE_SESSION base;
    void*        opt;
    int          opt_len;
    int          sta;

    int             exitflg;
    pthread_mutex_t mtx;
    pthread_cond_t  cv;
} platform_session;

static int platform_cb(struct BB_HANDLE* phd, struct usbpack* pack, struct BASE_SESSION* psess)
{
    platform_session* platsession = container_of(psess, platform_session, base);
    platsession->sta = pack->sta;
    if (platsession->opt && platsession->opt_len > 0) {
        memcpy(platsession->opt, pack->data_v, pack->datalen);
    }
    bs_weakup(psess);
    return 0;
}

static void platform_deinit(struct BASE_SESSION* psess)
{
    platform_session* platsession = container_of(psess, platform_session, base);
    bs_fun.de_init(psess);
    platsession->exitflg = 1;
    pthread_cond_broadcast(&platsession->cv);
}

static int platform_type(void)
{
    return st_platform;
}

static const BASE_FUN platform_fun = {
    .rdcb    = platform_cb,
    .de_init = platform_deinit,
    .tpid    = platform_type,
};

int platform_com_ctrl(bb_host_t* phost, uint32_t id, void* ipt, int ipt_len, void* opt, int opt_len)
{
    struct BB_HANDLE* phd  = bb_gethandle_from_host(phost);
    platform_session  phot = {
         .opt     = opt,
         .opt_len = opt_len,
    };

    if (!phd) {
        return -1;
    }

    bs_init(&phot.base, phd, &platform_fun);
    pthread_mutex_init(&phot.mtx, NULL);
    pthread_cond_init(&phot.cv, NULL);
    phot.exitflg = 0;

    usbpack pack = {
        .subcmdid = id,
        .data_v   = ipt,
        .datalen  = ipt_len,
    };
    pack.domainid = BB_REQ_PLAT_CTL;

    int ret = bs_send_usbpack_and_wait(&phot.base, &pack, -1);
    if (ret) {
        printf("hotplug fail , remote exit!\n");
    }

    bs_trig_close(&phot.base);

    pthread_mutex_lock(&phot.mtx);
    while (phot.exitflg == 0) {
        pthread_cond_wait(&phot.cv, &phot.mtx);
    }
    pthread_mutex_unlock(&phot.mtx);

    pthread_mutex_destroy(&phot.mtx);
    pthread_cond_destroy(&phot.cv);
    return ret;
}