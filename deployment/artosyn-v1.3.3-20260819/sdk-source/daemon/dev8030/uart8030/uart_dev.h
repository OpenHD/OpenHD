#ifndef __UART_DEV_H__
#define __UART_DEV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "ringbuffer.h"
#include "dev8030.h"
#include "uart_opt.h"

typedef struct uartrpc_ctrl uartrpc_ctrl;

typedef enum {
    uart_work,
    uart_idle,
    uart_hotplug,
    uart_unplug,
} uart_sta;

uart_msg add_uart_dev(uartrpc_ctrl* ctrl, const char* devnames, uart_par* par);
typedef struct {
    struct list_head node;

    char      dev_file_name[128];
    uart_hd*  uhd;
    uart_opt* opts;

    int working_id;

    void*   plist;
    // uint8_t rcv_buff[20 * 1024];
    // int     rcv_offset;

    /** UART input ringbuffer */
    ringbuffer_t    *rb;
    bool            process_done;       /** All data has been processed in current read */
#ifdef PKT_XDS_HDR
    uint32_t        analyze_offset;     /** Analyze point of the rb, should be between read and write*/
    uint32_t        new_pkt_offset;     /** Offset to the new pkt */
    bool            new_hdr_found;      /** Whtether find out new pkt's hdr */
#endif
    pthread_mutex_t rb_lock;

    pthread_mutex_t read_lock;
    pthread_mutex_t write_lock;

    uartrpc_ctrl* pctrl;
    pthread_t     find_thread;

    uint8_t serial_num[4];
    uint8_t ser_len;

    struct timespec last_recv_spec;
    int             offline_flg;

    uart_par saved_par;
    int      saved_flg;

    uart_sta usta;


    /*stats*/
    uint64_t        total_bytes;        /** Total bytes input from ingress */
    uint64_t        total_pkts;         /** Total pkts input from ingress */
    uint64_t        ok_bytes;           /** Total ok bytes input from ingress */
    uint64_t        ok_pkts;            /** Total ok pkts input from ingress */
    uint64_t        drop_gap_bytes;     /** Total bytes dropped with gap between pkts */
    uint64_t        drop_flen_bytes;    /** Total bytes dropped with invalid frame length */
    uint64_t        drop_checksum_bytes;/** Total bytes dropped with invalid checksum length */
} uart8030_info;

typedef struct uartrpc_ctrl {
    dev8030          basedev;
    struct list_head head_work; ///< 下挂的uart设备
    struct list_head head_idle; ///< 空闲设备

    struct list_head head_hotplug;
    struct list_head head_unplug;

    pthread_mutex_t mtx;
    int             working_cnt;
    uart_opt*       opts;
    uart_par        savepar;
} uartrpc_ctrl;

typedef struct uart_rpc_init_param_s {
    int     uart_id;
    int     baudrate;
    int     parity;
    int     data_bits;
    int     stop_bits;
} uart_rpc_init_param_t;

int reg_uartrpc_platfrom(rpc_info* prpc, uart_rpc_init_param_t *param);

#ifdef __cplusplus
}
#endif

#endif
