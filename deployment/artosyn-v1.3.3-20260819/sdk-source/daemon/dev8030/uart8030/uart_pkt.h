#ifndef __UART_DEV_PKT__
#define __UART_DEV_PKT__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "ringbuffer.h"
#include "uart_errno.h"
#include "uart_dev.h"
#include "uart_util.h"

#define MSG_RPC_HDR_SIZE                19
#define MSG_HEADER_SIZE                 sizeof(uart_pkt_hdr_t)
#define MSG_CMD_HDR_LENGTH_MAX          (MSG_RPC_HDR_SIZE + MSG_RPC_HDR_SIZE)
#define MSG_LENGTH_MAX                  (1024 + MSG_RPC_HDR_SIZE)
#define MSG_CMD_LENGTH_MAX              (4096 + MSG_CMD_HDR_LENGTH_MAX)
#define FRAME_SYNC_HEAD_LEN             5

/* define maximum supported packet pool config */
#define USR_XDS_PKT_POOL_MAX            16

#define MAX_BUFFER_SIZE                   1*1024

#define RINGBUFFER_DISTANCE(rb, a, b)  (((b) + (rb)->size - (a)) % (rb)->size)
#define RB_POS_MOV(rb, ori, off) (((ori) + (off)) % (rb)->size)

#pragma pack(push)
#pragma pack(1)
typedef struct uart_pkt_hdr_s
{
    uint8_t     magic_header[5];
    uint16_t    data_len;
    uint8_t     msg_id;
    uint8_t     rsv;
    uint8_t     user_id;
    uint8_t     channel;
    uint8_t     sum_num;
    uint8_t     cur_num;
    uint16_t    check_sum;
    uint8_t     data[0];
} uart_pkt_hdr_t;
#pragma pack(pop)

/* Calculate the checksum */
uint32_t uart_pkt_check_sum_cal(uint8_t* data, uint16_t data_size);

/* Check the checksum */
bool uart_pkt_check_sum_valid_in_rb(ringbuffer_t *rb, uint16_t start, uint16_t data_size, uint16_t in_sum);

/* Check the info of the pkt that should have hdr */
uart_ret uart_pkt_info_get(uint8_t *buff, uint16_t *length);

/* Find out the pkt in ringbuffer */
uart_ret uart_find_packet(IN uart8030_info* uart_info, OUT uint32_t *start, OUT uint32_t *len, OUT uint8_t* msg_id);

/* Check the buf [0-4] whether is the hdr magic. */
bool uart_hdr_magic_valid(uint8_t* buf);

/* Set the header of the pkt */
void uart_add_sync_head(uart_pkt_hdr_t *sync_data,
                            uint16_t data_len,
                            uint8_t msg_id,
                            uint8_t user_id,
                            uint8_t channel,
                            uint8_t sum_num,
                            uint8_t cur_num,
                            uint16_t checksum);

/** Check whether the data is a valid usr xdata pkt.*/
bool uart_pkt_format_vaild(IN uint8_t* data, IN uint32_t bufflen);

#ifdef __cplusplus
}
#endif

#endif
