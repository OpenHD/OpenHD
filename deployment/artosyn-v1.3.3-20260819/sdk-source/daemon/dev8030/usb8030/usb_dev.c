#include "usb_dev.h"
#include "bb_api.h"
#include "com_log.h"
#include "hotplug_rpc.h"
#include "list.h"
#include "pthread.h"
#include "rpc_node.h"
#include "timspec_helper.h"
#include "usb_event_list.h"
#include <libusb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#define usleep(n) Sleep(n / 1000)
#endif

typedef struct {
    int                     index;
    int                     txlen;
    struct libusb_transfer* transfer;
    usb8030_info*           p8030;
    int                     txflg;
    unsigned char           buff[20 * 1024];
    struct list_head        node;
} usb_tr_info;

#define TX_USB_SZ (8)
#define RX_USB_SZ (2)

typedef struct libusb_8030 {
    uint8_t               path[32];
    int                   path_len;
    int                   openable;
    libusb_context*       pusb_ctx;
    libusb_device_handle* pusb_hd;
    libusb_device*        pusb_dev;

    uint8_t  hid_endpoint_in;
    uint8_t  hid_endpoint_out;
    uint32_t hid_endpoint_in_max_size;
    uint32_t hid_endpoint_out_max_size;

    int     serialindex;
    int     seriallen;
    uint8_t serialbuff[32];

    int android_mode;
    int android_fd;

    int if_id;
} libusb_8030;

typedef struct usb8030_info {
    libusb_8030 pusb_info;

    void* plist;

    int working_id;

    int             exitflg;
    pthread_mutex_t tx_mtx;
    pthread_cond_t  tx_cv;

    struct list_head tx_wait_queue;
    usb_tr_info      tx_info[TX_USB_SZ];
    usb_tr_info      rx_info[RX_USB_SZ];

    int rcv_release_cnt; // 用于release 时

    struct list_head node;
} usb8030_info;

static void _usb_tr_info_free_transmit(usb_tr_info* tr_info)
{
    pthread_mutex_lock(&tr_info->p8030->tx_mtx);
    if (tr_info->transfer) {
        libusb_free_transfer(tr_info->transfer);
        tr_info->transfer = NULL;
        com_log(COM_USB_COM, "usb %s transmit id = %d free", tr_info->txflg ? "tx" : "rx", tr_info->index);
    }
    pthread_mutex_unlock(&tr_info->p8030->tx_mtx);
}
static int usb_ev_loop(void* p);

typedef struct {
    pthread_t     id;
    int           exit_flg;
    usb8030_info* p8030;
} exit_helper;

static void* usb_exit_loop_helper(void* p)
{
    exit_helper* helper = (exit_helper*)p;
    while (helper->exit_flg == 0) {
        usb_ev_loop(helper->p8030);
    }
    return NULL;
}

static void usb_8030_free_all_transfer(usb8030_info* p8030)
{
    for (int i = 0; i < TX_USB_SZ; i++) {
        if (p8030->tx_info[i].transfer) {
            int ret = libusb_cancel_transfer(p8030->tx_info[i].transfer);
            if (ret == LIBUSB_ERROR_NOT_FOUND) {
                // _usb_tr_info_free_transmit(&p8030->tx_info[i]);
            }
        }
    }

    for (int i = 0; i < RX_USB_SZ; i++) {
        if (p8030->rx_info[i].transfer) {
            int ret = libusb_cancel_transfer(p8030->rx_info[i].transfer);
            if (ret == LIBUSB_ERROR_NOT_FOUND) {
                p8030->rcv_release_cnt++;
                _usb_tr_info_free_transmit(&p8030->rx_info[i]);
            }
        }
    }

    exit_helper helper = {
        .exit_flg = 0,
        .p8030    = p8030,
    };
    pthread_create(&helper.id, NULL, usb_exit_loop_helper, &helper);

    int txcpl_cnt = 0;
    while (txcpl_cnt < TX_USB_SZ) {
        pthread_mutex_lock(&p8030->tx_mtx);
        while (list_empty(&p8030->tx_wait_queue)) {
            pthread_cond_wait(&p8030->tx_cv, &p8030->tx_mtx);
        }
        usb_tr_info* tr_transfer = list_first_entry(&p8030->tx_wait_queue, usb_tr_info, node);
        list_del(&tr_transfer->node);
        txcpl_cnt++;
        pthread_mutex_unlock(&p8030->tx_mtx);

        _usb_tr_info_free_transmit(tr_transfer);
    }

    while (1) {
        int exitflg = 0;

        pthread_mutex_lock(&p8030->tx_mtx);
        if (p8030->rcv_release_cnt == RX_USB_SZ) {
            exitflg = 1;
            pthread_mutex_unlock(&p8030->tx_mtx);
            break;
        }
        while (list_empty(&p8030->tx_wait_queue)) {
            pthread_cond_wait(&p8030->tx_cv, &p8030->tx_mtx);
        }
        pthread_mutex_unlock(&p8030->tx_mtx);
    }
    helper.exit_flg = 1;
    pthread_join(helper.id, NULL);
    com_log(COM_USB_COM, "trx free ok");
}

static void usb_8030_free_libusb(usb8030_info* p8030)
{
    libusb_8030* usbinfo = &p8030->pusb_info;
    libusb_release_interface(usbinfo->pusb_hd, p8030->pusb_info.if_id);
    libusb_close(usbinfo->pusb_hd);
}

void usb_8030_unplug(usb8030_info* p8030)
{
    p8030->exitflg = 1;
    pthread_cond_broadcast(&p8030->tx_cv);

    dev_node_force_exit(p8030->plist);
    p8030->plist = NULL;

    usb_8030_free_all_transfer(p8030);

    usb_8030_free_libusb(p8030);

    pthread_mutex_destroy(&p8030->tx_mtx);
    pthread_cond_destroy(&p8030->tx_cv);

    free(p8030);
}

static void wrusb_release(usb8030_info* p8030, usb_tr_info* tr_transfer)
{
    pthread_mutex_lock(&p8030->tx_mtx);
    list_add_tail(&tr_transfer->node, &p8030->tx_wait_queue);
    pthread_cond_signal(&p8030->tx_cv);
    pthread_mutex_unlock(&p8030->tx_mtx);
}

static void LIBUSB_CALL wrusb(struct libusb_transfer* transfer)
{
    usb_tr_info* tr_transfer = (usb_tr_info*)transfer->user_data;

    usb8030_info* p8030 = tr_transfer->p8030;

    if (!p8030->plist) {
        com_log(COM_USB_COM, "1 tx status = %d", transfer->status);
        wrusb_release(p8030, tr_transfer);
        return;
    }

    if (!dev_node_list_check_running(tr_transfer->p8030->plist)) {
        com_log(COM_USB_COM, "2 tx status = %d", transfer->status);
        wrusb_release(p8030, tr_transfer);
        return;
    }

    if (tr_transfer->txlen != transfer->actual_length || transfer->status != LIBUSB_TRANSFER_COMPLETED) {
        com_log(COM_USB_COM, "error tx status = %d", transfer->status);
    }

    pthread_mutex_lock(&tr_transfer->p8030->tx_mtx);
    list_add_tail(&tr_transfer->node, &tr_transfer->p8030->tx_wait_queue);
    pthread_cond_signal(&tr_transfer->p8030->tx_cv);
    pthread_mutex_unlock(&tr_transfer->p8030->tx_mtx);
}

static int usb_write(void* handle, unsigned char* buf, int len, int timeout)
{
    usb8030_info* p8030 = (usb8030_info*)handle;

    pthread_mutex_lock(&p8030->tx_mtx);
    while (list_empty(&p8030->tx_wait_queue) && p8030->exitflg == 0) {
        pthread_cond_wait(&p8030->tx_cv, &p8030->tx_mtx);
    }

    if (p8030->exitflg) {
        pthread_mutex_unlock(&p8030->tx_mtx);
        return 0;
    }

    usb_tr_info* tr_transfer = list_first_entry(&p8030->tx_wait_queue, usb_tr_info, node);
    list_del(&tr_transfer->node);
    pthread_mutex_unlock(&p8030->tx_mtx);

    memcpy(tr_transfer->buff, buf, len);
    tr_transfer->txlen = len;
    libusb_fill_bulk_transfer(tr_transfer->transfer,
                              p8030->pusb_info.pusb_hd,
                              p8030->pusb_info.hid_endpoint_out,
                              tr_transfer->buff,
                              len,
                              wrusb,
                              tr_transfer,
                              timeout);
    int ret = libusb_submit_transfer(tr_transfer->transfer);
    if (ret) {
        com_log(COM_USB_COM, "usb tx error = %d", ret);
        pthread_mutex_lock(&p8030->tx_mtx);
        list_add(&tr_transfer->node, &tr_transfer->p8030->tx_wait_queue);
        pthread_mutex_unlock(&p8030->tx_mtx);
    }
    return ret;
}

static int set_up_rd(usb_tr_info* puti);

static void usb_rd_release(usb8030_info* p8030, usb_tr_info* puti)
{
    pthread_mutex_lock(&p8030->tx_mtx);
    p8030->rcv_release_cnt++;
    pthread_cond_broadcast(&p8030->tx_cv);
    pthread_mutex_unlock(&p8030->tx_mtx);
    _usb_tr_info_free_transmit(puti);
}

static void LIBUSB_CALL usb_rd_cb(struct libusb_transfer* transfer)
{
    usb_tr_info* puti = (usb_tr_info*)transfer->user_data;

    usb8030_info* p8030 = puti->p8030;

    if (!p8030->plist) {
        com_log(COM_USB_COM, "stop recv");
        usb_rd_release(p8030, puti);
        return;
    }

    if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
        dev_node_list* plist = (dev_node_list*)p8030->plist;
        if (plist->dev_rd_cb) {
            plist->dev_rd_cb(plist->recv_priv, transfer->buffer, transfer->actual_length);
        }
    } else {
        com_log(COM_USB_COM, "rx statu = %d", transfer->status);
        usb_rd_release(p8030, puti);
        return;
    }
    if (!dev_node_list_check_running(p8030->plist)) {
        com_log(COM_USB_COM, "stop recv running");
        usb_rd_release(p8030, puti);
        return;
    }
    set_up_rd(puti);
}

static int set_up_rd(usb_tr_info* puti)
{
    int getbuflen = dev_node_list_get_buff(puti->p8030->plist);
    libusb_fill_bulk_transfer(puti->transfer,
                              puti->p8030->pusb_info.pusb_hd,
                              puti->p8030->pusb_info.hid_endpoint_in,
                              puti->buff,
                              getbuflen > 0 ? getbuflen : 4096,
                              usb_rd_cb,
                              puti,
                              0);
    int ret = libusb_submit_transfer(puti->transfer);
    if (ret) {
        com_log(COM_USB_COM, "usb rx error = %d", ret);
    }
    return ret;
}

static int usb_ev_loop(void* p)
{
    usb8030_info* p8030 = (usb8030_info*)p;

    struct timeval tv = {
        .tv_usec = 100 * 1000,
        .tv_sec  = 0,
    };

    int rc = libusb_handle_events_timeout(p8030->pusb_info.pusb_ctx, &tv);
    if (rc != LIBUSB_SUCCESS) {
        fprintf(stderr, "Transfer Error: %s\n", libusb_error_name(rc));
    }
    return 0;
}

static int usb_dev_name(void* devhandle, char* buff, int len)
{
    if (!devhandle || !buff || len <= 0) {
        return 0;
    }

    if (len >= 4) {
        strcpy(buff, "usb");
        return 4;
    }
    return 0;
}

static void set_usb_info(usb8030_info* p8030, dev_alive alive_cb, void* cb_priv)
{
    dev_node_list* plist = (dev_node_list*)malloc(sizeof(dev_node_list));
    memset(plist, 0, sizeof(dev_node_list));
    plist->devhandle = p8030;
    plist->dev_send  = usb_write;
    plist->dev_rd_cb = NULL;
    plist->recv_priv = NULL;
    plist->dev_loop  = usb_ev_loop;
    plist->devname   = usb_dev_name;
    plist->running   = run_all;
    p8030->plist     = plist;

    plist->devalive      = alive_cb;
    plist->devalive_priv = cb_priv;

    plist->remote_buff_len         = 0;
    plist->remote_buff_max_payload = 0;
    pthread_mutex_init(&p8030->tx_mtx, NULL);
    pthread_cond_init(&p8030->tx_cv, NULL);
    p8030->exitflg         = 0;
    p8030->rcv_release_cnt = 0;
    INIT_LIST_HEAD(&p8030->tx_wait_queue);
    // set index
    for (int i = 0; i < TX_USB_SZ; i++) {
        usb_tr_info* ptr = p8030->tx_info + i;

        ptr->transfer        = libusb_alloc_transfer(0);
        ptr->transfer->flags = LIBUSB_TRANSFER_ADD_ZERO_PACKET;
        ptr->p8030           = p8030;
        ptr->index           = i;
        ptr->txflg           = 1;
        list_add_tail(&ptr->node, &p8030->tx_wait_queue);
    }

    for (int i = 0; i < RX_USB_SZ; i++) {
        p8030->rx_info[i].transfer = libusb_alloc_transfer(0);
        p8030->rx_info[i].p8030    = p8030;
        p8030->rx_info[i].index    = i;
        p8030->rx_info[i].txflg    = 0;
        set_up_rd(&p8030->rx_info[i]);
    }

    dev_node_list_init(plist);
}

static int usb_find_class_interface(struct libusb_config_descriptor* conf_desc,
                                    int                              nb_ifaces,
                                    enum libusb_class_code           class_id,
                                    uint8_t*                         endpoint_in,
                                    uint8_t*                         endpoint_out,
                                    uint32_t*                        endpoint_in_size,
                                    uint32_t*                        endpoint_out_size)
{
    struct libusb_endpoint_descriptor* endpoint;
    int                                i, j, k;

    for (i = 0; i < nb_ifaces; i++) {

        if (class_id != conf_desc->interface[i].altsetting[0].bInterfaceClass) {
            continue;
        }

        for (j = 0; j < conf_desc->interface[i].num_altsetting; j++) {

            for (k = 0; k < conf_desc->interface[i].altsetting[j].bNumEndpoints; k++) {
                struct libusb_ss_endpoint_companion_descriptor* ep_comp = NULL;
                endpoint = (struct libusb_endpoint_descriptor*)&conf_desc->interface[i].altsetting[j].endpoint[k];

                libusb_get_ss_endpoint_companion_descriptor(NULL, endpoint, &ep_comp);
                if (ep_comp) {
                    libusb_free_ss_endpoint_companion_descriptor(ep_comp);
                }

                if ((endpoint->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK)
                    & (LIBUSB_TRANSFER_TYPE_BULK | LIBUSB_TRANSFER_TYPE_INTERRUPT)) {
                    if (endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN) {
                        *endpoint_in      = endpoint->bEndpointAddress;
                        *endpoint_in_size = endpoint->wMaxPacketSize;
                    } else {
                        *endpoint_out      = endpoint->bEndpointAddress;
                        *endpoint_out_size = endpoint->wMaxPacketSize;
                    }
                }
            }

            if (*endpoint_in && *endpoint_out)
                return i;
        }
    }
    return -1;
}

static int usb_8030_probe(libusb_device* dev, libusb_8030* curinfo)
{
    libusb_device_handle* handle = NULL;

    int err = libusb_open(dev, &handle);

    if (err != LIBUSB_SUCCESS) {
        return 0;
    }

    curinfo->pusb_dev = dev;
    curinfo->pusb_hd  = handle;

    struct libusb_config_descriptor* conf_desc = NULL;

    do {
        err = libusb_get_config_descriptor(dev, 0, &conf_desc);
        if (err < 0) {
            break;
        }
        int nb_ifaces;
        nb_ifaces = conf_desc->bNumInterfaces;
        int iface = usb_find_class_interface(conf_desc,
                                             nb_ifaces,
                                             LIBUSB_CLASS_HID,
                                             &curinfo->hid_endpoint_in,
                                             &curinfo->hid_endpoint_out,
                                             &curinfo->hid_endpoint_in_max_size,
                                             &curinfo->hid_endpoint_out_max_size);

        if (iface < 0) {
            err = 1;
            break;
        }
#ifndef WIN32
        // win 沒有
        err = libusb_set_auto_detach_kernel_driver(handle, 1);
        if (err != 0) {
            break;
        }
#endif
        curinfo->seriallen = libusb_get_string_descriptor_ascii(handle,
                                                                curinfo->serialindex,
                                                                curinfo->serialbuff,
                                                                sizeof(curinfo->serialbuff));

        err = libusb_claim_interface(handle, iface);
        if (err != 0) {
            break;
        }
        curinfo->if_id = iface;
        err            = 0;
    } while (0);

    if (conf_desc) {
        libusb_free_config_descriptor(conf_desc);
    }

    if (err) {
        libusb_close(handle);
        return 0;
    }

    return 1;
}

static int usb_8030_get_all_usb(libusb_context* pusbctx, libusb_8030* sinfo, int maxnum)
{
    libusb_device**                 devs;
    int                             retcnt  = 0;
    ssize_t                         dev_cnt = libusb_get_device_list(pusbctx, &devs);
    struct libusb_device_descriptor desc;

    for (int i = 0; i < dev_cnt && retcnt < maxnum; i++) {
        libusb_8030* curinfo = &sinfo[retcnt];

        int ret = libusb_get_device_descriptor(devs[i], &desc);

        if (ret < 0) {
            continue;
        }
        if (ARTO_RTOS_VID != desc.idVendor || ARTO_RTOS_PID != desc.idProduct) {
            continue;
        }
        usleep(100 * 1000);
        curinfo->serialindex = desc.iSerialNumber;

        curinfo->path_len = libusb_get_port_numbers(devs[i], curinfo->path, sizeof(curinfo->path));
        curinfo->openable = usb_8030_probe(devs[i], curinfo);
        if (curinfo->openable) {
            curinfo->pusb_ctx = pusbctx;
            char buff[256];
            int  len = 0;
            len += sprintf(buff + len, "probe new 8030 dev @");
            if (curinfo->path_len > 0) {
                len += sprintf(buff + len, "%d", curinfo->path[0]);
                for (int j = 1; j < curinfo->path_len; j++) {
                    len += sprintf(buff + len, ".%d", curinfo->path[j]);
                }
            }
            com_log(COM_USB_COM, buff);
        }
        retcnt++;
    }
    libusb_free_device_list(devs, 1);
    return retcnt;
}

static usb8030_info* usb8030_check_close(struct list_head* phead, libusb_8030* sinfo, int scnt)
{
    usb8030_info* p8030 = NULL;
    list_for_each_entry (p8030, phead, node, usb8030_info) {
        int findflg = 0;
        for (int sindex = 0; sindex < scnt; sindex++) {
            libusb_8030* pinfo = &sinfo[sindex];
            if (pinfo->path_len == p8030->pusb_info.path_len
                && !memcmp(pinfo->path, p8030->pusb_info.path, p8030->pusb_info.path_len)) {
                findflg = 1;
                break;
            }
        }

        if (!findflg) {
            return p8030;
        }
    }
    return NULL;
}
typedef struct {
    pthread_mutex_t alive_mtx;
    pthread_cond_t  alive_cv;
    int             alive_flg;
    int             drop_step;
} alive_st;

static void usb_alive_cb(dev_node_list* plist, void* priv)
{
    alive_st* pst = (alive_st*)priv;
    if (pst->drop_step) {
        return;
    }
    pthread_mutex_lock(&pst->alive_mtx);
    pthread_cond_broadcast(&pst->alive_cv);
    pst->alive_flg = 1;
    pthread_mutex_unlock(&pst->alive_mtx);
}

static usb8030_info* start_8030_proc(libusb_8030* infos)
{
    if (!infos || infos->openable == 0) {
        return NULL;
    }

    usb8030_info* p8030 = malloc(sizeof(usb8030_info));
    memset(p8030, 0, sizeof(usb8030_info));
    memcpy(&p8030->pusb_info, infos, sizeof(libusb_8030));

    alive_st alive = {
        .alive_flg = 0,
        .drop_step = 0,
        .alive_mtx = PTHREAD_MUTEX_INITIALIZER,
        .alive_cv  = PTHREAD_COND_INITIALIZER,
    };

    set_usb_info(p8030, usb_alive_cb, &alive);
    // 等待远程响应
    com_log(COM_USB_COM, "wait alive");

    struct timespec tp;
    get_timespec_from_now(&tp, 1000);

    pthread_mutex_lock(&alive.alive_mtx);
    while (!alive.alive_flg) {
        int sta = pthread_cond_timedwait(&alive.alive_cv, &alive.alive_mtx, &tp);
        if (sta == ETIMEDOUT) {
            break;
        }
    }

    alive.drop_step = 1;
#if 0
    alive.alive_flg = 0;
#endif
    if (alive.alive_flg) {
        com_log(COM_USB_COM, "remote found");
    } else {
        com_log(COM_USB_COM, "bad remote dev");
        usb_8030_unplug(p8030);
    }

    pthread_mutex_unlock(&alive.alive_mtx);

    pthread_mutex_destroy(&alive.alive_mtx);
    pthread_cond_destroy(&alive.alive_cv);

    if (!alive.alive_flg) {
        return NULL;
    }

    return p8030;
}
typedef struct rpc_info rpc_info;
typedef struct usbrpc_ctrl {
    dev8030 basedev;

    struct list_head info8030_list; ///< 下挂设备
    int              use_cnt;

    pthread_mutex_t mtx;
    libusb_context* pusbctx;
} usbrpc_ctrl;

void usbrpc_lock(usbrpc_ctrl* pctrl)
{
    pthread_mutex_lock(&pctrl->mtx);
}

void usbrpc_unlock(usbrpc_ctrl* pctrl)
{
    pthread_mutex_unlock(&pctrl->mtx);
}

static int usbrpc_getsz(dev8030* pdev8030)
{
    usbrpc_ctrl* pctrl = (usbrpc_ctrl*)pdev8030;

    return pctrl->use_cnt;
}

static int usbrpc_getid_all(dev8030* pdev8030, uint32_t* ids, size_t sz)
{
    usbrpc_ctrl* pctrl = (usbrpc_ctrl*)pdev8030;

    if (!pctrl || !ids || sz <= 0) {
        return 0;
    }

    usbrpc_lock(pctrl);
    sz = sz > pctrl->use_cnt ? pctrl->use_cnt : sz;

    usb8030_info* pinfo = NULL;
    int           cnt   = 0;
    list_for_each_entry (pinfo, &pctrl->info8030_list, node, usb8030_info) {
        ids[cnt] = pinfo->working_id;
        if (++cnt >= sz) {
            break;
        }
    }
    usbrpc_unlock(pctrl);
    return cnt;
}

static usb8030_info* usbrpc_get_info(dev8030* pdev8030, uint32_t workid)
{
    usbrpc_ctrl* pctrl = (usbrpc_ctrl*)pdev8030;

    usb8030_info* ret = NULL;
    usbrpc_lock(pctrl);

    usb8030_info* pinfo = NULL;
    list_for_each_entry (pinfo, &pctrl->info8030_list, node, usb8030_info) {
        if (pinfo->working_id == workid) {
            ret = pinfo;
            break;
        }
    }
    usbrpc_unlock(pctrl);
    return ret;
}

static void* usbrpc_get_plist(dev8030* pdev8030, uint32_t workid)
{
    usbrpc_ctrl* pctrl = (usbrpc_ctrl*)pdev8030;

    usb8030_info* p8030 = usbrpc_get_info(&pctrl->basedev, workid);

    if (p8030) {
        return p8030->plist;
    }
    return NULL;
}

static int usbrpc_fill_serial(dev8030* pdev8030, uint32_t workid, uint8_t* buff, size_t len)
{
    usbrpc_ctrl* pctrl = (usbrpc_ctrl*)pdev8030;

    if (!pctrl || !buff || len <= 0) {
        return -1;
    }

    usb8030_info* p8030 = usbrpc_get_info(&pctrl->basedev, workid);

    if (!p8030) {
        return -1;
    }

    if (p8030->pusb_info.seriallen >= 0 && p8030->pusb_info.seriallen < len) {
        memcpy(buff, p8030->pusb_info.serialbuff, p8030->pusb_info.seriallen);
        return p8030->pusb_info.seriallen;
    }
    return 0;
}

/**
 * @brief 循环扫描8030插拔 阻塞api
 *
 */
static int usbrpc_8030_poll(dev8030* pdev8030)
{
    usbrpc_ctrl* pctrl = container_of(pdev8030, usbrpc_ctrl, basedev);

#define MAX_8030_CNT (10)

    libusb_8030 sinfos[MAX_8030_CNT];
    memset(sinfos, 0, sizeof(sinfos));
    int cnt    = usb_8030_get_all_usb(pctrl->pusbctx, sinfos, MAX_8030_CNT);
    int evtflg = 0;

    // 检查关闭的8030
    usbrpc_lock(pctrl);
    while (1) {
        usb8030_info* pdel = usb8030_check_close(&pctrl->info8030_list, sinfos, cnt);
        // trig close
        if (pdel) {
            evtflg++;
            char buff[256];
            int  len = 0;
            len += sprintf(buff + len, "close 8030 usb @");
            libusb_8030* curinfo = &pdel->pusb_info;
            if (curinfo->path_len > 0) {
                len += sprintf(buff + len, "%d", curinfo->path[0]);
                for (int j = 1; j < curinfo->path_len; j++) {
                    len += sprintf(buff + len, ".%d", curinfo->path[j]);
                }
            }
            com_log(COM_USB_COM, buff);

            list_del(&pdel->node);
            bb_event_hotplug_t plug_evt = {
                .id            = pdel->working_id,
                .status        = 0,
                .bb_mac.maclen = pdel->pusb_info.seriallen,
            };

            memcpy(&plug_evt.bb_mac.mac, pdel->pusb_info.serialbuff, pdel->pusb_info.seriallen);

            usb_8030_unplug(pdel);
            dev8030_hotplug_event(pdev8030->pinfo, &plug_evt);

            pctrl->use_cnt--;
        } else {
            break;
        }
    }
    usbrpc_unlock(pctrl);

    // 检查新开的8030
    usbrpc_lock(pctrl);
    for (int i = 0; i < cnt; i++) {
        usb8030_info* padd = start_8030_proc(&sinfos[i]);
        if (padd) {
            padd->working_id = rpc_dev_get_nxt_working_id(pctrl->basedev.pinfo);
            list_add(&padd->node, &pctrl->info8030_list);
            bb_event_hotplug_t plug_evt = {
                .id            = padd->working_id,
                .status        = 1,
                .bb_mac.maclen = padd->pusb_info.seriallen,
            };
            memcpy(&plug_evt.bb_mac.mac, padd->pusb_info.serialbuff, padd->pusb_info.seriallen);

            dev8030_hotplug_event(pdev8030->pinfo, &plug_evt);
            pctrl->use_cnt++;
            evtflg++;
        }
    }
    usbrpc_unlock(pctrl);

    return evtflg;
}

static dev8030_act usb8030_dev_info;

static int _reg_usbrpc_platfrom(rpc_info* prpc, dev8030_act* pact)
{
    libusb_context* pusbctx;

    int ret = libusb_init(&pusbctx);

    if (ret) {
        com_log(COM_USB_COM, "libusb_init err = %d", ret);
        return -1;
    }

    usbrpc_ctrl* ctrl = malloc(sizeof(usbrpc_ctrl));
    memset(ctrl, 0, sizeof(usbrpc_ctrl));
    ctrl->pusbctx     = pusbctx;
    rpc_dev_add(prpc, &ctrl->basedev);

    ctrl->use_cnt = 0;

    if (pact) {
        ctrl->basedev.pact = pact;
    } else {
        ctrl->basedev.pact = &usb8030_dev_info;
    }

    ctrl->basedev.pinfo = prpc;

    INIT_LIST_HEAD(&ctrl->info8030_list);

    pthread_mutex_init(&ctrl->mtx, NULL);

    return 0;
}

int reg_usbrpc_platfrom(rpc_info* prpc)
{
    com_log(COM_USB_COM, "reg usb 8030 platform");

    _reg_usbrpc_platfrom(prpc, NULL);

    return 0;
}

static dev8030_act usb8030_dev_info = {
    .name        = "usb",
    .polldev     = usbrpc_8030_poll,
    .getplist    = usbrpc_get_plist,
    .getsz       = usbrpc_getsz,
    .getid_all   = usbrpc_getid_all,
    .fill_serial = usbrpc_fill_serial,
};

static dev8030_act usb8030_dev_info_ahost;
/**
 * @brief 安卓no root模式下 需要先设置跳过枚举 否则无法启动
 *
 * @param prpc
 * @return int
 */
int reg_usbrpc_hostjnimode_platform(rpc_info* prpc)
{
    com_log(COM_USB_COM, "usb skip device discovery");
    libusb_set_option(NULL, LIBUSB_OPTION_NO_DEVICE_DISCOVERY, NULL);
    _reg_usbrpc_platfrom(prpc, &usb8030_dev_info_ahost);
    return 0;
}

static int usbrpc_8030_poll_jnihost_mode(dev8030* pdev8030)
{
    usbrpc_ctrl* pctrl = container_of(pdev8030, usbrpc_ctrl, basedev);

    return 0;
}

static int usbrpc_host_del_fd(dev8030* pdev8030, int fd)
{
    int          err   = -1;
    usbrpc_ctrl* pctrl = container_of(pdev8030, usbrpc_ctrl, basedev);

    usbrpc_lock(pctrl);

    do {
        int           findflg = 0;
        usb8030_info* pdel    = NULL;
        list_for_each_entry (pdel, &pctrl->info8030_list, node, usb8030_info) {
            if (pdel->pusb_info.android_mode && pdel->pusb_info.android_fd == fd) {
                findflg = 1;
                break;
            }
        }

        if (!findflg) {
            break;
        }

        list_del(&pdel->node);
        bb_event_hotplug_t plug_evt = {
            .id            = pdel->working_id,
            .status        = 0,
            .bb_mac.maclen = pdel->pusb_info.seriallen,
        };

        memcpy(&plug_evt.bb_mac.mac, pdel->pusb_info.serialbuff, pdel->pusb_info.seriallen);

        usb_8030_unplug(pdel);
        dev8030_hotplug_event(pdev8030->pinfo, &plug_evt);
        pctrl->use_cnt--;
        err = 0;
    } while (0);

    usbrpc_unlock(pctrl);

    return err;
}

static int usbrpc_host_add_fd(dev8030*    pdev8030,
                              int         fd,
                              int         iface,
                              int         epin,
                              int         insz,
                              int         epout,
                              int         outsz,
                              const char* serial,
                              int         serlen,
                              const char* path,
                              int         pathlen)
{
    int          err   = -1;
    usbrpc_ctrl* pctrl = container_of(pdev8030, usbrpc_ctrl, basedev);

    usbrpc_lock(pctrl);

    do {
        libusb_device_handle* handle = NULL;

        err = libusb_wrap_sys_device(NULL, fd, &handle);
        if (err) {
            break;
        }

        libusb_8030 devusb = {
            .pusb_dev                  = NULL,
            .pusb_hd                   = handle,
            .openable                  = 1,
            .hid_endpoint_in           = epin,
            .hid_endpoint_in_max_size  = insz,
            .hid_endpoint_out          = epout,
            .hid_endpoint_out_max_size = outsz,
            .pusb_ctx                  = pctrl->pusbctx,
            .serialindex               = iface,
            .android_mode              = 1,
            .android_fd                = fd,
        };
#ifndef WIN32
        // win 沒有
        err = libusb_set_auto_detach_kernel_driver(handle, 1);
        if (err != 0) {
            break;
        }
#endif
        memcpy(devusb.path, path, pathlen);
        devusb.path_len = pathlen;
        memcpy(devusb.serialbuff, serial, serlen);
        devusb.seriallen = serlen;

        err = libusb_claim_interface(handle, iface);
        if (err != 0) {
            break;
        }

        usb8030_info* padd = start_8030_proc(&devusb);
        if (padd) {
            padd->working_id = rpc_dev_get_nxt_working_id(pctrl->basedev.pinfo);
            list_add(&padd->node, &pctrl->info8030_list);
            bb_event_hotplug_t plug_evt = {
                .id            = padd->working_id,
                .status        = 1,
                .bb_mac.maclen = padd->pusb_info.seriallen,
            };
            memcpy(&plug_evt.bb_mac.mac, padd->pusb_info.serialbuff, padd->pusb_info.seriallen);

            dev8030_hotplug_event(pdev8030->pinfo, &plug_evt);
            pctrl->use_cnt++;
        }
    } while (0);

    usbrpc_unlock(pctrl);
    return err;
}

#define usbhostname "usb-android-host"

/**
 * @brief 传入usb设备信息 所有数据只能由java 写入 内部无法获取
 *
 * @param prpc
 * @param fd
 * @param iface
 * @param epin
 * @param insz
 * @param epout
 * @param outsz
 * @param serial
 * @param serlen
 * @param path
 * @param pathlen
 * @return int
 */
int usbrpc_parse_usbfd(rpc_info*   prpc,
                       int         fd,
                       int         iface,
                       int         epin,
                       int         insz,
                       int         epout,
                       int         outsz,
                       const char* serial,
                       int         serlen,
                       const char* path,
                       int         pathlen)
{
    int evt = 0;

    dev8030* polls;
    list_for_each_entry (polls, &prpc->head, node, dev8030) {
        if (!strcmp(polls->pact->name, usbhostname)) {
            return usbrpc_host_add_fd(polls, fd, iface, epin, insz, epout, outsz, serial, serlen, path, pathlen);
        }
    }
    return -1;
}

/**
 * @brief 删除传入的usb设备信息
 *
 * @param prpc
 * @param fd
 */
int usbrpc_remove_usbfd(rpc_info* prpc, int fd)
{
    int evt = 0;

    dev8030* polls;
    list_for_each_entry (polls, &prpc->head, node, dev8030) {
        if (!strcmp(polls->pact->name, usbhostname)) {
            return usbrpc_host_del_fd(polls, fd);
        }
    }
    return -1;
}

static dev8030_act usb8030_dev_info_ahost = {
    .name        = usbhostname,
    .polldev     = usbrpc_8030_poll_jnihost_mode,
    .getplist    = usbrpc_get_plist,
    .getsz       = usbrpc_getsz,
    .getid_all   = usbrpc_getid_all,
    .fill_serial = usbrpc_fill_serial,
};
