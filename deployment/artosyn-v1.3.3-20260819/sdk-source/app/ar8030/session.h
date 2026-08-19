#ifndef _SESSION_H__
#define _SESSION_H__

#include <pthread.h>
#include <stdint.h>

struct BB_HANDLE;
struct BASE_SESSION;
struct usbpack;
struct bb_dev_handle_t;
struct bb_host_t;
struct usbpack;

enum session_type {
    st_notinit  = -1,
    st_ioctl    = 1,
    st_callback = 2,
    st_socket   = 3,
    st_hotplug  = 4,
    st_platform = 5,
};

typedef int (*rpc_cb)(struct BB_HANDLE*, struct usbpack*, struct BASE_SESSION* psess);
typedef void (*deinit)(struct BASE_SESSION*);
typedef int (*get_type)(void);
typedef struct {
    rpc_cb   rdcb;
    deinit   de_init;
    get_type tpid;
} BASE_FUN;

typedef struct BASE_SESSION {
    const BASE_FUN*   fun;
    struct BB_HANDLE* phd;
    pthread_mutex_t   mtx;
    pthread_cond_t    cv;
    uint8_t           wakeup_need : 1;
    uint8_t           wakeup_set  : 1;
    uint8_t           exit_flg    : 1;
    pthread_cond_t    exitcv;
} BASE_SESSION;

extern const BASE_FUN bs_fun;

int  bs_send_usbpack_and_wait(BASE_SESSION* psess, struct usbpack* pack, int timeout);
void bs_init(BASE_SESSION* psess, struct BB_HANDLE* phd, BASE_FUN const* fun);
void bs_weakup(BASE_SESSION* psess);
void bs_trig_close(BASE_SESSION* psess);

struct BB_HANDLE* bb_gethandle(struct bb_dev_handle_t* pdev);
struct BB_HANDLE* bb_gethandle_from_host(struct bb_host_t* phost);

BASE_SESSION* bb_get_session(struct BB_HANDLE* pdev);

void bb_set_new_session(struct BB_HANDLE* phd, BASE_SESSION* psess);

int send_usbpack(struct BB_HANDLE* phd, struct usbpack* pack);

struct usbpack make_usb_pack(uint32_t reqid, uint8_t* data, uint32_t len);
#endif