#include "bg_proc.h"
#include "pthread.h"
#include "ringbuffer.h"
#include <stdlib.h>
#include <string.h>

typedef struct bg_proc {
    uint8_t* rb_buffer;
    uint8_t* batch_buffer;
    size_t   batchsz;

    int             stop_flg;
    ringbuffer_t    rb;
    pthread_mutex_t mtx;
    pthread_cond_t  cv;
    pthread_t       thread;
    write_proc      wr_proc;
    void*           wr_priv;
} bg_proc;

static void* bg_write_thread(void* ptr)
{
    bg_proc* bgdat = (bg_proc*)ptr;
    pthread_mutex_lock(&bgdat->mtx);
    while (!bgdat->stop_flg) {
        if (ringbuffer_is_empty(&bgdat->rb)) {
            pthread_cond_wait(&bgdat->cv, &bgdat->mtx);
        }

        int len = ringbuffer_out(&bgdat->rb, bgdat->batch_buffer, bgdat->batchsz);

        pthread_mutex_unlock(&bgdat->mtx);
        bgdat->wr_proc(bgdat->wr_priv, bgdat->batch_buffer, len);
        pthread_mutex_lock(&bgdat->mtx);
    }
    pthread_mutex_unlock(&bgdat->mtx);
    return NULL;
}

bg_proc* bg_init(write_proc proc, void* priv, size_t buffsz, size_t batch_sz)
{
    bg_proc* ret = (bg_proc*)malloc(sizeof(bg_proc));

    memset(ret, 0, sizeof(bg_proc));
    ret->batch_buffer = malloc(batch_sz);
    ret->batchsz      = batch_sz;
    ret->rb_buffer    = malloc(buffsz);

    ret->rb.buffer = ret->rb_buffer;
    ret->rb.size   = buffsz;
    ret->wr_priv   = priv;
    ret->wr_proc   = proc;

    ret->stop_flg = 0;

    pthread_mutex_init(&ret->mtx, NULL);
    pthread_cond_init(&ret->cv, NULL);

    pthread_create(&ret->thread, NULL, bg_write_thread, ret);

    return ret;
}

void bg_write(bg_proc* bgdata, void* data, size_t len)
{
    if (!bgdata || !data || !len) {
        return;
    }

    pthread_mutex_lock(&bgdata->mtx);
    ringbuffer_in(&bgdata->rb, data, len);
    pthread_cond_signal(&bgdata->cv);
    pthread_mutex_unlock(&bgdata->mtx);
}

void bg_close(bg_proc* bgdata)
{
    bgdata->stop_flg = 1;
    pthread_cond_signal(&bgdata->cv);
    pthread_join(bgdata->thread, NULL);

    pthread_mutex_destroy(&bgdata->mtx);
    pthread_cond_destroy(&bgdata->cv);

    free(bgdata->batch_buffer);
    free(bgdata->rb_buffer);

    free(bgdata);
}
