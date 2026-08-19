
#include "com_log.h"
#include "dev8030.h"
#include "hotplug_rpc.h"
#include "list.h"
#include "rpc_node.h"
#include "timspec_helper.h"
#include "usb_event_list.h"
#include "usbpack.h"
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ringbuffer.h"

#ifdef WIN32
#include <windows.h>
#define sleep(n)  Sleep(n * 1000)
#define usleep(n) Sleep(n / 1000)
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "uart_dev.h"
#include "uart_pkt.h"

/** Size of the ringbuffer */
#define RB_SIZE     (20 * 1024)

/** Debug dump buffer length */
#define DBG_DUMP_BUF_LEN    256
/** Maximum number of dump per line */
#define MAX_DUMP_PER_LINE   16
/** Maximum data dump length */
#define MAX_DATA_DUMP_LEN   48
#define MIN(a, b)           ((a)<(b)?(a):(b))

#if !defined(UNUSED)
#define UNUSED(x)               ((void)(x))
#endif

#ifdef PKT_XDS_HDR
/** msg id of the xds hdr */
#define XDS_MSG_ID_RPC      0x50
#endif

/**
* @brief  Print data of the buffer.
* @param  prompt           Prompt of the buffer
*         buf              Start address of the buffer
*         len              Size of the buffer
* @note   Make sure that data_size is not out of the range.
*/
static void print_buf(IN char *prompt, IN uint8_t *buf, IN int len)
{
#ifndef UART_BUF_PRINT
    UNUSED(prompt);
    UNUSED(buf);
    UNUSED(len);
#else
    int i;
    char dbgdumpbuf[DBG_DUMP_BUF_LEN];
    char *ptr = dbgdumpbuf;
    static pthread_mutex_t mutex = NULL;

    if (!mutex) {
        pthread_mutex_init(&mutex, NULL);
    }

    pthread_mutex_lock(&mutex);

    uart_log_buf("%s: len=%d", prompt, len);
    for (i = 1; i <= MIN(len, MAX_DATA_DUMP_LEN); i++) {
        ptr += snprintf(ptr, 4, "%02x ", *buf);
        buf++;
        if (i % MAX_DUMP_PER_LINE == 0) {
            *ptr = 0;
            uart_log_buf("%s", dbgdumpbuf);
            ptr = dbgdumpbuf;
        }
    }
    if (len % MAX_DUMP_PER_LINE) {
        *ptr = 0;
        uart_log_buf("%s", dbgdumpbuf);
    }

    pthread_mutex_unlock(&mutex);
#endif
}

static int uart_write(void* handle, unsigned char* buff, int len, int timeout)
{
    uart8030_info* puart = (uart8030_info*)handle;
    uint8_t *write_data = NULL;
    uint16_t write_len = 0;
#ifdef PKT_XDS_HDR
    uart_pkt_hdr_t *pkt = NULL;
    uint16_t checksum;
#endif

    pthread_mutex_lock(&puart->write_lock);

#ifdef PKT_XDS_HDR
    pkt = malloc(MSG_HEADER_SIZE + len);
    if (!pkt) {
        pthread_mutex_unlock(&puart->write_lock);
        return 0;
    }

    memset(pkt, 0, MSG_HEADER_SIZE + len);
    checksum = (uint16_t)uart_pkt_check_sum_cal(buff, len);

    /* Add xds hdr */
    uart_add_sync_head(pkt, len, XDS_MSG_ID_RPC, 0, 0, 0, 0, checksum);
    memcpy(pkt->data, buff, len);

    write_data = (uint8_t*)pkt;
    write_len = MSG_HEADER_SIZE + len;
#else
    write_data = buff;
    write_len = len;
#endif

    puart->opts->wr(puart->uhd, write_data, write_len);

    pthread_mutex_unlock(&puart->write_lock);

    return 0;
}

static int uart_data_get(uart8030_info *dev, uint8_t* pkt_data, uint32_t *pkt_len)
{
    int ret = 0;
    int read_len = 0;
    bool drop = false;
    uint8_t  msg_id = 0;
    uint32_t start = 0;     //offset to the last pkt end
    uint32_t acc_len = 0;   //pkt len without skip_bytes
    static uint8_t data_buf[MSG_CMD_LENGTH_MAX] = {0};

    if (!dev) {
        uart_log_error("%s param is invalid!", __FUNCTION__);
        return -1;
    }

    dev->process_done = false;

    while (!dev->process_done) {
        drop = false;

#ifdef PKT_XDS_HDR
        /* Check the pkt, find the hdr */
        ret = uart_find_packet(dev, &start, &acc_len, &msg_id);
        if ((ret != UART_ERROR_NONE) && (ret != UART_ERROR_PKT_DROP)) {
            if (ret != UART_ERROR_PKT_NOT_COMP) {
                uart_log_warn("usr_xds_find_packet failed! ret %d", ret);
            }

            goto done;
        }
        else if (ret == UART_ERROR_PKT_DROP) {
            /* For free the space of ringbuffer */
            uart_log_warn("Drop the skip bytes");
            drop = true;
        }

        uart_log_dbg("start %d acc_len %d msg_id 0x%x drop %d", start, acc_len, msg_id, drop);
#else
        acc_len = MIN(ringbuffer_len(dev->rb), MSG_CMD_LENGTH_MAX);
#endif
        read_len = ringbuffer_out(dev->rb, data_buf, acc_len);
        uint64_t ms = 0;

        if (read_len != acc_len) {
            uart_log_warn("error ringbuffer_out , acc_len = %d, get = %d\n", acc_len, read_len);
        }

        uart_log_dbg("ringbuffer_out read_len %d, rb avail %d,size %d, len %d, write %d, read %d",
                    read_len, ringbuffer_avail(dev->rb), dev->rb->size, dev->rb->data_len,
                    dev->rb->write, dev->rb->read);

#ifndef PKT_XDS_HDR
        if (ringbuffer_len(dev->rb) == 0) {
            dev->process_done = true;
        }
#endif
        /*FIXME:deal with the data*/
        if (!drop) {
            dev->ok_pkts++;
            dev->ok_bytes += read_len;
#ifdef PKT_XDS_HDR
            print_buf("get data", data_buf + MSG_HEADER_SIZE, acc_len - MSG_HEADER_SIZE);
#else
            print_buf("get data", data_buf, acc_len);
#endif
            if (pkt_data) {
                memcpy(pkt_data, data_buf, read_len);
            }

            if (pkt_len) {
                *pkt_len = read_len;
            }

            return 0;
        }
    }

done:

    return ret;
}

static int uart_data_process(uart8030_info *dev)
{
    int ret = 0;
    int read_len = 0;
    bool drop = false;
    uint8_t  msg_id = 0;
    uint32_t start = 0;     //offset to the last pkt end
    uint32_t acc_len = 0;   //pkt len without skip_bytes
    static uint8_t data_buf[MSG_CMD_LENGTH_MAX] = {0};

    if (!dev) {
        uart_log_error("%s param is invalid!", __FUNCTION__);
        return -1;
    }

    dev->process_done = false;

    while (!dev->process_done) {
        drop = false;

#ifdef PKT_XDS_HDR
        /* Check the pkt, find the hdr */
        ret = uart_find_packet(dev, &start, &acc_len, &msg_id);
        if ((ret != UART_ERROR_NONE) && (ret != UART_ERROR_PKT_DROP)) {
            if (ret != UART_ERROR_PKT_NOT_COMP) {
                uart_log_warn("usr_xds_find_packet failed! ret %d", ret);
            }

            goto done;
        }
        else if (ret == UART_ERROR_PKT_DROP) {
            /* For free the space of ringbuffer */
            uart_log_warn("Drop the skip bytes");
            drop = true;
        }

        uart_log_dbg("start %d acc_len %d msg_id 0x%x drop %d", start, acc_len, msg_id, drop);
#else
        acc_len = MIN(ringbuffer_len(dev->rb), MSG_CMD_LENGTH_MAX);
#endif
        read_len = ringbuffer_out(dev->rb, data_buf, acc_len);
        uint64_t ms = 0;

        if (read_len != acc_len) {
            uart_log_warn("error ringbuffer_out , acc_len = %d, get = %d\n", acc_len, read_len);
        }

        uart_log_dbg("ringbuffer_out read_len %d, rb avail %d,size %d, len %d, write %d, read %d",
                    read_len, ringbuffer_avail(dev->rb), dev->rb->size, dev->rb->data_len,
                    dev->rb->write, dev->rb->read);

#ifndef PKT_XDS_HDR
        if (ringbuffer_len(dev->rb) == 0) {
            dev->process_done = true;
        }
#endif
        /*FIXME:deal with the data*/
        if (!drop) {
            dev->ok_pkts++;
            dev->ok_bytes += read_len;
#ifdef PKT_XDS_HDR
            print_buf("get data", data_buf + MSG_HEADER_SIZE, acc_len - MSG_HEADER_SIZE);
#else
            print_buf("get data", data_buf, acc_len);
#endif
            dev_node_list* plist = (dev_node_list*)dev->plist;
            if (plist->dev_rd_cb) {
                /* Remove the sync hdr */
#ifdef PKT_XDS_HDR
                plist->dev_rd_cb(plist->recv_priv, data_buf + MSG_HEADER_SIZE, acc_len - MSG_HEADER_SIZE);
#else
                plist->dev_rd_cb(plist->recv_priv, data_buf, acc_len);
#endif
            }
        }
    }

done:

    return ret;
}

static int uart_ev_loop(void* handle)
{
    uart8030_info* puart = (uart8030_info*)handle;
    ringbuffer_t *rb = NULL;
    int len = 0;
    int rd_len = 0;
    uint8_t* buf = NULL;
    const int timeoutms = 1000;
    struct timespec specnow;

    pthread_mutex_lock(&puart->read_lock);

    rb = puart->rb;
    if (!rb) {
        return -1;
    }

    buf = rb->buffer + rb->write;

    /** The max size that we can read and store to the ringbuffer.
     * Since the ringbuffer is not continuelly, we could only write
     * until the end of the ringbuffer.
     *
     * It should consider following cases(* means free):
     * 1. ....write****read...          --->        read - write
     * 2. ****read.....write**          --->        rb_size - write
     * */
    if (((rb->write + 1) % rb->size) == rb->read) {
        rd_len = 0;
    }
    else {
        if (rb->read > rb->write) {
            rd_len = rb->read - rb->write - 1;
        }
        else {
            if (rb->read == 0) {
                rd_len = rb->size - rb->write - 1;
            }
            else {
                rd_len = rb->size - rb->write;
            }
        }
    }

    uart_log_verb("rd_len %d read %d write %d size %d\n", rd_len,
            rb->read, rb->write, rb->size);

    pthread_mutex_unlock(&puart->read_lock);

    if (rd_len > 0) {
        len = puart->opts->rd(puart->uhd, buf, rd_len);
    }
    else {
        uart_log_warn("buf is full!");
        len = -1;
    }

#ifdef WIN32
    timespec_get(&specnow, TIME_UTC);
#else
    clock_gettime(CLOCK_REALTIME, &specnow);
#endif
    if (len <= 0) {
        int ms = get_timespec_diff_in_ms(&puart->last_recv_spec, &specnow);

        if (ms > timeoutms && puart->offline_flg == 0) {
            puart->offline_flg = 1;
            com_log(COM_UART_COM, "%s %dms keep alive timeout!!", puart->dev_file_name, timeoutms);
            uartrpc_ctrl* pctrl = puart->pctrl;
            pthread_mutex_lock(&pctrl->mtx);
            list_del(&puart->node);
            list_add_tail(&puart->node, &pctrl->head_unplug);
            puart->usta = uart_unplug;
            pthread_mutex_unlock(&pctrl->mtx);
        }
        return 0;
    }

    puart->last_recv_spec = specnow;

    if (len <= 0) {
        usleep(10000);//10ms
    }
    else {
        puart->total_pkts++;
        puart->total_bytes += len;

        puart->rb->data_len += len;
        puart->rb->write = RB_POS_MOV(puart->rb, puart->rb->write, len);

        uart_data_process(puart);
    }

    return 0;
}

static int uart_dev_name(void* devhandle, char* buff, int len)
{
    if (!devhandle || !buff || len <= 0) {
        return 0;
    }

    if (len >= 5) {
        strcpy(buff, "uart");
        return 5;
    }
    return 0;
}

static uart8030_info* uartrpc_get_info(uartrpc_ctrl* pctrl, uint32_t workid)
{
    uart8030_info* ret = NULL;

    pthread_mutex_lock(&pctrl->mtx);

    uart8030_info* pinfo = NULL;
    list_for_each_entry (pinfo, &pctrl->head_work, node, uart8030_info) {
        if (pinfo->working_id == workid) {
            ret = pinfo;
            break;
        }
    }
    pthread_mutex_unlock(&pctrl->mtx);
    return ret;
}

static void* uartrpc_get_plist(dev8030* pdev8030, uint32_t workid)
{
    uartrpc_ctrl*  pctrl = container_of(pdev8030, uartrpc_ctrl, basedev);
    uart8030_info* p8030 = uartrpc_get_info(pctrl, workid);

    if (p8030) {
        return p8030->plist;
    }

    return NULL;
}

static int uartrpc_fill_serial(dev8030* pdev8030, uint32_t workid, uint8_t* buff, size_t len)
{
    uartrpc_ctrl* pctrl = container_of(pdev8030, uartrpc_ctrl, basedev);

    if (!pctrl || !buff || len <= 0) {
        return -1;
    }

    uart8030_info* p8030 = uartrpc_get_info(pctrl, workid);

    if (!p8030) {
        return -1;
    }

    if (p8030->ser_len >= 0 && p8030->ser_len < len) {
        memcpy(buff, p8030->serial_num, p8030->ser_len);
        return p8030->ser_len;
    }
    return 0;
}

static int uartrpc_getid_all(dev8030* pdev8030, uint32_t* ids, size_t sz)
{
    uartrpc_ctrl* pctrl = container_of(pdev8030, uartrpc_ctrl, basedev);

    if (!pctrl || !ids || sz <= 0) {
        return 0;
    }

    pthread_mutex_lock(&pctrl->mtx);
    sz = sz > pctrl->working_cnt ? pctrl->working_cnt : sz;

    uart8030_info* pinfo = NULL;
    int            cnt   = 0;
    list_for_each_entry (pinfo, &pctrl->head_work, node, uart8030_info) {
        ids[cnt] = pinfo->working_id;
        if (++cnt >= sz) {
            break;
        }
    }
    pthread_mutex_unlock(&pctrl->mtx);
    return cnt;
}

static int uartrpc_getsz(dev8030* pdev8030)
{
    uartrpc_ctrl* pctrl = (uartrpc_ctrl*)pdev8030;
    return pctrl->working_cnt;
}

static int set_uart_devinfo(uart8030_info* puart)
{
    // set dev_node_list
    dev_node_list* plist = (dev_node_list*)malloc(sizeof(dev_node_list));
    memset(plist, 0, sizeof(dev_node_list));
    plist->devhandle = puart;
    plist->dev_send  = uart_write;
    plist->dev_rd_cb = NULL;
    plist->recv_priv = NULL;
    plist->dev_loop  = uart_ev_loop;
    plist->devname   = uart_dev_name;
    plist->running   = run_all;

    plist->remote_buff_len         = 0;
    plist->remote_buff_max_payload = 0;

    puart->plist = plist;

#ifdef WIN32
    timespec_get(&puart->last_recv_spec, TIME_UTC);
#else
    clock_gettime(CLOCK_REALTIME, &puart->last_recv_spec);
#endif

    dev_node_list_init(plist);
    return 0;
}

static void uart_8030_unplug(uart8030_info* puart)
{
    dev_node_force_exit(puart->plist);
    puart->opts->cl(puart->uhd);
    free(puart);
}

static int uartrpc_8030_poll(dev8030* pdev8030)
{
    int evt = 0;

    uartrpc_ctrl* pctrl = container_of(pdev8030, uartrpc_ctrl, basedev);

    pthread_mutex_lock(&pctrl->mtx);
    // hotplug
    while (!list_empty(&pctrl->head_hotplug)) {
        uart8030_info* padd = list_first_entry(&pctrl->head_hotplug, uart8030_info, node);
        list_del(&padd->node);
        list_add_tail(&padd->node, &pctrl->head_work);
        pctrl->working_cnt++;
        padd->usta = uart_work;
        pthread_mutex_unlock(&pctrl->mtx);

        // plugin event
        set_uart_devinfo(padd);
        padd->offline_flg = 0;
        padd->working_id  = rpc_dev_get_nxt_working_id(pctrl->basedev.pinfo);

        bb_event_hotplug_t plug_evt = {
            .id            = padd->working_id,
            .status        = 1,
            .bb_mac.maclen = padd->ser_len,
        };
        memcpy(&plug_evt.bb_mac.mac, padd->serial_num, padd->ser_len);
        dev8030_hotplug_event(pdev8030->pinfo, &plug_evt);

        com_log(COM_UART_COM,
                "%s plug in dev@%02x%02x%02x%02x",
                padd->dev_file_name,
                padd->serial_num[0],
                padd->serial_num[1],
                padd->serial_num[2],
                padd->serial_num[3]);
        pthread_mutex_lock(&pctrl->mtx);
        evt++;
    }
    // unplug
    while (!list_empty(&pctrl->head_unplug)) {
        uart8030_info* pdel = list_first_entry(&pctrl->head_unplug, uart8030_info, node);
        list_del(&pdel->node);
        pctrl->working_cnt--;
        pthread_mutex_unlock(&pctrl->mtx);

        // unplug event & re start com
        char devname[128];
        strcpy(devname, pdel->dev_file_name);
        int      flg = pdel->saved_flg;
        uart_par par;
        if (flg) {
            memcpy(&par, &pdel->saved_par, sizeof(uart_par));
        }
        bb_event_hotplug_t plug_evt = {
            .id            = pdel->working_id,
            .status        = 0,
            .bb_mac.maclen = pdel->ser_len,
        };
        memcpy(&plug_evt.bb_mac.mac, pdel->serial_num, pdel->ser_len);
        com_log(COM_UART_COM,
                "%s unplug dev@%02x%02x%02x%02x",
                pdel->dev_file_name,
                pdel->serial_num[0],
                pdel->serial_num[1],
                pdel->serial_num[2],
                pdel->serial_num[3]);
        uart_8030_unplug(pdel);
        dev8030_hotplug_event(pdev8030->pinfo, &plug_evt);
        add_uart_dev(pctrl, devname, flg ? &par : NULL);

        pthread_mutex_lock(&pctrl->mtx);
        evt++;
    }
    pthread_mutex_unlock(&pctrl->mtx);

    return evt;
}

static void* uart_search_dev(void* par)
{
    int            ret              = 0;
    uart8030_info* puart = (uart8030_info*)par;
    uint8_t        buff[1024]       = {0};
    uint8_t        rcv_buff[1024]   = {0};
    uint32_t       rcv_len          = 0;
    uint8_t*       write_data       = NULL;
    uint16_t       write_len        = 0;

    int             ms          = 0;
    ringbuffer_t    *rb         = NULL;
    int             len         = 0;
    int             rd_len      = 0;
    uint8_t*        buf         = NULL;
    const int       timeoutms   = 1000;
    struct timespec specnow, specstart;

    bool search_finish = false;

    com_log(COM_UART_COM, "%s start search", puart->dev_file_name);

    usbpack send_pack  = { 0 };
    send_pack.domainid = 0xff;
    send_pack.subcmdid = 0;
    send_pack.datalen  = 0;
    send_pack.datapack = NULL;
    int sdlen          = make_usbpack2buff(buff, sizeof(buff), &send_pack);
    if ((sdlen < 0) || (sdlen > sizeof(buff))) {
        return NULL;
    }

    while (1) {
#ifdef PKT_XDS_HDR
        uart_pkt_hdr_t *pkt = NULL;
        uint16_t checksum;
#endif

        pthread_mutex_lock(&puart->write_lock);

#ifdef PKT_XDS_HDR
        pkt = malloc(MSG_HEADER_SIZE + sdlen);
        if (!pkt) {
            pthread_mutex_unlock(&puart->write_lock);
            return 0;
        }

        memset(pkt, 0, MSG_HEADER_SIZE + sdlen);
        checksum = (uint16_t)uart_pkt_check_sum_cal(buff, sdlen);

        /* Add xds hdr */
        uart_add_sync_head(pkt, sdlen, XDS_MSG_ID_RPC, 0, 0, 0, 0, checksum);
        memcpy(pkt->data, buff, sdlen);

        write_data = (uint8_t*)pkt;
        write_len = MSG_HEADER_SIZE + sdlen;
#else
        write_data = buff;
        write_len = sdlen;
#endif

        if (puart && puart->opts && puart->opts->wr && puart->uhd) {
            puart->opts->wr(puart->uhd, write_data, write_len);
        }

        pthread_mutex_unlock(&puart->write_lock);

        /* wait for the reply */
#ifdef WIN32
        timespec_get(&specstart, TIME_UTC);
#else
        clock_gettime(CLOCK_REALTIME, &specstart);
#endif
        do {
            pthread_mutex_lock(&puart->read_lock);

            rb = puart->rb;
            if (!rb) {
                pthread_mutex_unlock(&puart->read_lock);
                break;
            }

            buf = rb->buffer + rb->write;

            /** The max size that we can read and store to the ringbuffer.
             * Since the ringbuffer is not continuelly, we could only write
             * until the end of the ringbuffer.
             *
             * It should consider following cases(* means free):
             * 1. ....write****read...          --->        read - write
             * 2. ****read.....write**          --->        rb_size - write
             * */
            if (((rb->write + 1) % rb->size) == rb->read) {
                rd_len = 0;
            }
            else {
                if (rb->read > rb->write) {
                    rd_len = rb->read - rb->write - 1;
                }
                else {
                    if (rb->read == 0) {
                        rd_len = rb->size - rb->write - 1;
                    }
                    else {
                        rd_len = rb->size - rb->write;
                    }
                }
            }

            uart_log_verb("rd_len %d read %d write %d size %d\n", rd_len,
                    rb->read, rb->write, rb->size);

            pthread_mutex_unlock(&puart->read_lock);

            if (rd_len > 0) {
                len = puart->opts->rd(puart->uhd, buf, rd_len);
            }
            else {
                uart_log_warn("buf is full!");
                len = -1;
            }

            if (len > 0) {

                puart->total_pkts++;
                puart->total_bytes += len;

                puart->rb->data_len += len;
                puart->rb->write = RB_POS_MOV(puart->rb, puart->rb->write, len);

                ret = uart_data_get(puart, rcv_buff, &rcv_len);
                if (ret == 0) {
                    uartrpc_ctrl* pctrl = puart->pctrl;

#ifdef PKT_XDS_HDR
                    if (rcv_len >= (8 + MSG_HEADER_SIZE)) {
#else
                    if (rcv_len >= 8) {
#endif
                        puart->ser_len = 4;
#ifdef PKT_XDS_HDR
                        memcpy(puart->serial_num, rcv_buff + 4 + MSG_HEADER_SIZE, 4);
#else
                        memcpy(puart->serial_num, rcv_buff + 4, 4);
#endif
                        com_log(COM_UART_COM,
                                "%s detect remote serial num %02x %02x %02x %02x",
                                puart->dev_file_name,
                                puart->serial_num[0],
                                puart->serial_num[1],
                                puart->serial_num[2],
                                puart->serial_num[3]);
                    } else {
                        com_log(COM_UART_COM, "%s no remote serial num", puart->dev_file_name);
                    }

                    pthread_mutex_lock(&pctrl->mtx);
                    list_del(&puart->node);
                    list_add_tail(&pctrl->head_hotplug, &puart->node);
                    puart->usta = uart_hotplug;
                    pthread_mutex_unlock(&pctrl->mtx);
                    search_finish = true;
                    break;
                }
            }

#ifdef WIN32
            timespec_get(&specnow, TIME_UTC);
#else
            clock_gettime(CLOCK_REALTIME, &specnow);
#endif
            ms = get_timespec_diff_in_ms(&specstart, &specnow);
        }while (ms < timeoutms);

        if (search_finish) {
            break;
        }
    }
    return NULL;
}

static dev8030_act uart8030_dev_info;

static uart8030_info* find_uart_dev_by_name(uartrpc_ctrl* ctrl, const char* devnames)
{
    uart8030_info* search = NULL;

    if (!ctrl) {
        return NULL;
    }

    list_for_each_entry (search, &ctrl->head_idle, node, uart8030_info) {
        if (!strcmp(search->dev_file_name, devnames)) {
            return search;
        }
    }
    list_for_each_entry (search, &ctrl->head_work, node, uart8030_info) {
        if (!strcmp(search->dev_file_name, devnames)) {
            return search;
        }
    }
    list_for_each_entry (search, &ctrl->head_hotplug, node, uart8030_info) {
        if (!strcmp(search->dev_file_name, devnames)) {
            return search;
        }
    }
    list_for_each_entry (search, &ctrl->head_unplug, node, uart8030_info) {
        if (!strcmp(search->dev_file_name, devnames)) {
            return search;
        }
    }
    return NULL;
}

static uart8030_info* find_uart_dev_by_name_lock(uartrpc_ctrl* ctrl, const char* devnames)
{
    pthread_mutex_lock(&ctrl->mtx);
    uart8030_info* ret = find_uart_dev_by_name(ctrl, devnames);
    pthread_mutex_unlock(&ctrl->mtx);
    return ret;
}

uart_msg add_uart_dev(uartrpc_ctrl* ctrl, const char* devnames, uart_par* par)
{
    uart8030_info* find = find_uart_dev_by_name_lock(ctrl, devnames);

    if (find) {
        com_log(COM_UART_COM, "%s had added , cur sta = %d", devnames, find->usta);
        return uart_had_added;
    }

    uart_opt* opt = ctrl->opts;
    uart_hd*  hdl = opt->op(devnames, par);
    if (!hdl) {
        return uart_open_fail;
    }

    uart8030_info* puart = malloc(sizeof(uart8030_info));
    memset(puart, 0, sizeof(uart8030_info));

    if (!puart) {
        return uart_open_fail;
    }

    strcpy(puart->dev_file_name, devnames);
    puart->plist   = NULL;
    puart->uhd     = hdl;
    puart->opts    = opt;
    puart->pctrl   = ctrl;
    puart->ser_len = 0;
    puart->usta    = uart_idle;

    if (par) {
        memcpy(&puart->saved_par, par, sizeof(uart_par));
        puart->saved_flg = 1;
    } else {
        puart->saved_flg = 0;
    }
    puart->rb = malloc(sizeof(ringbuffer_t));
    if (!puart->rb) {
        return uart_open_fail;
    }
    memset(puart->rb, 0, sizeof(ringbuffer_t));
    ringbuffer_reset(puart->rb);
    puart->rb->size = RB_SIZE;
    puart->rb->buffer = malloc(puart->rb->size);
    pthread_mutex_init(&puart->rb_lock, NULL);
    pthread_mutex_init(&puart->read_lock, NULL);
    pthread_mutex_init(&puart->write_lock, NULL);

    pthread_mutex_lock(&ctrl->mtx);
    list_add_tail(&puart->node, &ctrl->head_idle);
    pthread_mutex_unlock(&ctrl->mtx);

    pthread_create(&puart->find_thread, NULL, uart_search_dev, puart);

    return uart_add_ok;
}

int reg_uartrpc_platform(rpc_info* prpc, char* devs, uart_par* par)
{
    com_log(COM_UART_COM, "reg uart 8030 platform");

    uartrpc_ctrl* ctrl = malloc(sizeof(uartrpc_ctrl));
    memset(ctrl, 0, sizeof(uartrpc_ctrl));

    ctrl->opts          = get_back_end();
    ctrl->basedev.pact  = &uart8030_dev_info;
    ctrl->basedev.pinfo = prpc;
    ctrl->working_cnt   = 0;
    pthread_mutex_init(&ctrl->mtx, NULL);

    INIT_LIST_HEAD(&ctrl->head_idle);
    INIT_LIST_HEAD(&ctrl->head_work);
    INIT_LIST_HEAD(&ctrl->head_hotplug);
    INIT_LIST_HEAD(&ctrl->head_unplug);

    add_uart_dev(ctrl, devs, par);

    rpc_dev_add(prpc, &ctrl->basedev);

    return 0;
}

static int uartrpc_opt_ctrl(dev8030* pdev, uint32_t reqid, void* input, int iptlen, void* output, int* optlen)
{
    uartrpc_ctrl* pctrl = container_of(pdev, uartrpc_ctrl, basedev);

    switch (reqid) {
    case BB_RPC_SERIAL_LIST: {
        uart_list_hd* ulist = pctrl->opts->list_alloc();
        memcpy(output, ulist, sizeof(uart_list_hd));
        *optlen = sizeof(uart_list_hd);
        pctrl->opts->list_free(ulist);
    } break;
    case BB_RPC_SERIAL_SETUP: {
        uart_ioctl* uio = (uart_ioctl*)input;
        return add_uart_dev(pctrl, uio->uart_dev_name, &uio->par);
    } break;
    default:
        com_log(COM_UART_COM, "unknown opt = %x", reqid);
        break;
    }
    return -1;
}

static dev8030_act uart8030_dev_info = {
    .name        = "uart",
    .polldev     = uartrpc_8030_poll,
    .getplist    = uartrpc_get_plist,
    .getsz       = uartrpc_getsz,
    .getid_all   = uartrpc_getid_all,
    .fill_serial = uartrpc_fill_serial,
    .opt_ctrl    = uartrpc_opt_ctrl,
};
