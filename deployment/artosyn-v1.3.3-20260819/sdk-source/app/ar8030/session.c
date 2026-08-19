#include "session.h"
#include <pthread.h>
#include <stdio.h>

void bs_weakup(BASE_SESSION* psess)
{
    pthread_mutex_lock(&psess->mtx);
    psess->wakeup_set = 1;
    pthread_cond_broadcast(&psess->cv);
    pthread_mutex_unlock(&psess->mtx);
}

int bs_send_usbpack_and_wait(BASE_SESSION* psess, struct usbpack* pack, int timeout)
{
    int ret = 0;

    if (!psess->phd) {
        return -1;
    }
    pthread_mutex_lock(&psess->mtx);
    psess->wakeup_need = 1;
    psess->wakeup_set  = 0;
    send_usbpack(psess->phd, pack);
    while (!psess->exit_flg && !psess->wakeup_set) {
        if (timeout >= 0) {
            struct timespec ts;
#ifdef WIN32
            timespec_get(&ts, TIME_UTC);
#else
            clock_gettime(CLOCK_REALTIME, &ts);
#endif
            ts.tv_nsec += ((timeout % 1000) * 1000 * 1000);
            ts.tv_sec += (timeout / 1000) + (ts.tv_nsec/1000000000ULL);
            ts.tv_nsec = ts.tv_nsec % 1000000000ULL;

            ret = pthread_cond_timedwait(&psess->cv, &psess->mtx, &ts);
            if (ret) {
                printf("wait ret err = %d\n", ret);
                break;
            }
        }
        else {
            ret = pthread_cond_wait(&psess->cv, &psess->mtx);
            if (ret) {
                printf("wait ret err = %d\n", ret);
            }
        }
    }
    psess->wakeup_need = 0;
    psess->wakeup_set  = 0;
    if (psess->exit_flg) {
        pthread_cond_broadcast(&psess->exitcv);
        ret = psess->exit_flg;
    }
    pthread_mutex_unlock(&psess->mtx);

    return ret;
}

void bs_init(BASE_SESSION* psess, struct BB_HANDLE* phd, BASE_FUN const* fun)
{
    pthread_mutex_init(&psess->mtx, NULL);
    pthread_cond_init(&psess->cv, NULL);
    bb_set_new_session(phd, psess);

    psess->wakeup_need = 0;
    psess->wakeup_set  = 0;
    psess->exit_flg    = 0;
    pthread_cond_init(&psess->exitcv, NULL);

    psess->fun = fun;
    psess->phd = phd;
}

static void bs_deinit(BASE_SESSION* psess)
{
    psess->exit_flg = 1;

    if (psess->wakeup_need) {
        pthread_cond_signal(&psess->cv);
        printf("session wait subprocess exit!\n");
        pthread_cond_wait(&psess->exitcv, &psess->mtx);
    }

    pthread_cond_destroy(&psess->exitcv);
    pthread_mutex_destroy(&psess->mtx);
    pthread_cond_destroy(&psess->cv);
    if (psess->phd) {
        bb_set_new_session(psess->phd, NULL);
        psess->phd = NULL;
    }
}

static int bs_rpc_cb(struct BB_HANDLE* phd, struct usbpack* pack, struct BASE_SESSION* psess)
{
    return 0;
}

static int bs_type(void)
{
    return st_notinit;
}

const BASE_FUN bs_fun = {
    .rdcb    = bs_rpc_cb,
    .de_init = bs_deinit,
    .tpid    = bs_type,
};
