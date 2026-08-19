#ifdef PKT_XDS_HDR
#include <stdlib.h>
#include <stdint.h>
#include "uart_pkt.h"
#include "uart_errno.h"
#include "uart_endian.h"
#include "uart_dev.h"
#include "uart_util.h"

/**
* @brief  Calculate the checksum of the payload
* @param  data              Buffer of the data
*         data_size         Length of the data
* @retval Checksum of the data.
* @note   Return the checksum as uint32, and you should turrn it
*         to oyher format by yourself.
*/
uint32_t uart_pkt_check_sum_cal(uint8_t* data, uint16_t data_size)
{
    uint32_t cs = 0;
    int i;
    for(i = 0; i < data_size; i++){
        cs += data[i];
    }
    return cs;
}

/**
* @brief  Check whether the checksum of the payload in ringbuffer is valid.
* @param  rb                Ringbuffer that store the data
*         start             Start position in ringbuffer
*         data_size         Length of the datas
*         in_sum            The checksum need to check
* @retval Whether the checksum of the data in buffer is the same with the given one.
* @note   You should notice that the the range of the ringbuffer,
*/
bool uart_pkt_check_sum_valid_in_rb(ringbuffer_t *rb, uint16_t start, uint16_t data_size, uint16_t in_sum)
{
    uint32_t cs = 0;
    int i;

    if (!rb) {
        return false;
    }

    for(i = 0; i < data_size; i++)
    {
        cs += rb->buffer[(start + i) % rb->size];
    }

    if(in_sum == (uint16_t)cs) {
        return true;
    }
    else {
        uart_log_warn("checksum err!start %d data_size %d pkt:(0x%x) calc(0x%x)", start, data_size, in_sum, (uint16_t)cs);

        return false;
    }
}

/**
* @brief  Get the length of the pkt.
* @param  buff              Buffer of the pkt.
*         length            Length of the packet(in sync header)
* @retval UART_ERROR_NONE    succ
*         Others            fail
* @note   Packet should be with sync header.
*/
uart_ret uart_pkt_info_get(uint8_t *buff, uint16_t *length)
{
    uart_ret ret = UART_ERROR_NONE;
    uint16_t pkt_len = 0;

    if (!buff || !length) {
        return UART_ERROR_PARAM;
    }

    /* Check whether the sync header is valid */
    if (!((buff[0] == 0xFF) &&
            (buff[1] == 0xA5) &&
            (buff[2] == 0xAA) &&
            (buff[3] == 0x5A) &&
            (buff[4] == 0xFF))) {
        uart_log_error("%s magic is invald 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x",
                         __FUNCTION__, buff[0], buff[1], buff[2], buff[3], buff[4]);
        return UART_ERROR_PKT_INVALID_MAGIC;
    }

    /* Total length of the packet */
    pkt_len = buff[5] + (buff[6] << 8) + MSG_HEADER_SIZE;
    if (pkt_len >= MSG_CMD_LENGTH_MAX) {
        uart_log_error("%s pkt_len %d is larger than max %d", __FUNCTION__, pkt_len, MSG_CMD_LENGTH_MAX);
        return UART_ERROR_PKT_INVALID_LEN;
    }

    *length = pkt_len;

    return ret;
}

/**
* @brief  Add the sync header to the pkt.
* @param  sync_data         Address of the sync header
*         data_len          Length of the payload
*         msg_id            msg_id
*         user_id           user_id
*         channel           channel
*         sum_num           sum_num
*         cur_num           cur_num
*         checksum          checksum
* @retval nont
* @note   Please Manage the buffer of the pkt by yourself.
*/
void uart_add_sync_head(uart_pkt_hdr_t *sync_data,
                            uint16_t data_len,
                            uint8_t msg_id,
                            uint8_t user_id,
                            uint8_t channel,
                            uint8_t sum_num,
                            uint8_t cur_num,
                            uint16_t checksum)
{
    uint8_t frame_sync_head[FRAME_SYNC_HEAD_LEN]={0xFF,0xA5,0xAA,0x5A,0xFF};

    memcpy(sync_data->magic_header, frame_sync_head, FRAME_SYNC_HEAD_LEN);
    sync_data->data_len = htole16(data_len);
    sync_data->msg_id = msg_id;
    sync_data->rsv = 0;
    sync_data->user_id = user_id;
    sync_data->channel = channel;
    sync_data->sum_num = sum_num;
    sync_data->cur_num = cur_num;
    sync_data->check_sum = htole16(checksum);
}

/**
* @brief  Check the buf [0-4] whether is the hdr magic.
* @param  buf                       buf if the data
* @retval true     buf [0-4] is the hdr magic
*         false    buf [0-4] is not the hdr magic
* @note   You should ensure that buf is larger than FRAME_SYNC_HEAD_LEN).
*/
bool uart_hdr_magic_valid(uint8_t* buf)
{
    int i;
    uint8_t frame_sync_head[FRAME_SYNC_HEAD_LEN]={0xFF,0xA5,0xAA,0x5A,0xFF};

    for (i = 0; i < FRAME_SYNC_HEAD_LEN; i++) {
        if (buf[i] != frame_sync_head[i]) {
            return false;
        }
    }

    return true;
}

/**
* @brief  Find packet hdr in ringbuffer, and return the bytes before the hdr.
* @param  uart_info                 Info of the uart(include ringbuffer info)
*         len                       Length of the data need to analyze
*         skip_bytes                OUT, the bytes need to skip
* @retval UART_ERROR_NONE            succ
*         UART_ERROR_PKT_NOT_COMP    pkt not finish yet
*         Others                    fail
* @note   Skip the analyzed data if hdr is no found out, otherwise skip the bytes
*         before the hdr.
*/
static uart_ret uart_find_sync_head(IN uart8030_info* uart_info, IN uint32_t len, OUT uint32_t *skip_bytes)
{
    uart_ret ret = UART_ERROR_NONE;
    uint8_t* buf = NULL;
    uint32_t i, j;
    uint32_t _offset;
    uint32_t rb_size;
    uint8_t frame_sync_head[FRAME_SYNC_HEAD_LEN]={0xFF,0xA5,0xAA,0x5A,0xFF};

    if (!uart_info) {
        uart_log_error("%s uart_info is NULL", __FUNCTION__);
        return UART_ERROR_PARAM;
    }

    if (!skip_bytes) {
        uart_log_error("%s skip_bytes is NULL", __FUNCTION__);
        return UART_ERROR_PARAM;
    }

    buf = uart_info->rb->buffer;
    rb_size = uart_info->rb->size;

    _offset = uart_info->analyze_offset;

    uart_log_dbg("rb_size %d _offset %d", rb_size, _offset);
    for(i = 0;i < (len - (FRAME_SYNC_HEAD_LEN - 1));i++)
    {
        for (j = 0;j < FRAME_SYNC_HEAD_LEN;j++) {
            if (buf[RB_POS_MOV(uart_info->rb, _offset, (i + j))] != frame_sync_head[j]) {
                break;
            }
        }

        /* If find out the hdr, return */
        if (j == FRAME_SYNC_HEAD_LEN) {
            *skip_bytes = i;
            return UART_ERROR_NONE;
        }
    }

    *skip_bytes = len - 4;
    return UART_ERROR_PKT_NOT_COMP;
}

/**
* @brief  Find packet in ringbuffer, and copy the data to the buffer.
* @param  uart_info                 IN, Info of the uart(include ringbuffer info)
*         start                     OUT, start offset to the last pkt in rb
*         len                       OUT, length of the accually data
*         msg_id                    OUT, msg id in pkt
* @retval UART_ERROR_NONE            succ
*         UART_ERROR_PKT_NOT_COMP    pkt not finish yet
*         Others                    fail
* @note   Ringbuffer won't pop the data after copying the pkt, but only
*         offser the start position and length of the pkt.
*/
uart_ret uart_find_packet(IN uart8030_info* uart_info, OUT uint32_t *start, OUT uint32_t *len, OUT uint8_t* msg_id)
{
    uart_ret ret = UART_ERROR_NONE;
    int key;
    bool invalid_frame_length = false;
    bool invalid_checksum = false;
    uint16_t checksum = 0;          // checksum in pkt
    uint32_t framelen = 0;          // payload length of new pkt
    uint32_t cur_len = 0;           // length of new pkt has been reveived
    uint32_t rb_size = 0;           // ringbuffer size
    uint32_t fifo_leave_size = 0;   // data not analyzed
    uint32_t skip_bytes = 0;
    uint32_t data_len_offset = 0;
    uint32_t checksum_offset = 0;
    ringbuffer_t *rb = NULL;

    if (!uart_info || !uart_info->rb) {
        return UART_ERROR_PARAM;
    }

    pthread_mutex_lock(&uart_info->rb_lock);

    /* Temp parameters */
    rb = uart_info->rb;
    rb_size = rb->size;

    /* Init the param for lookup the hdr */
    skip_bytes = 0;
    fifo_leave_size = RINGBUFFER_DISTANCE(rb, uart_info->analyze_offset, rb->write);

    uart_log_dbg("analyze_offset %d rb->write %d fifo_leave_size %d\n", uart_info->analyze_offset, rb->write, fifo_leave_size);

    if (uart_info->new_hdr_found) {
        /* New pkt hdr has been found out, try to get leave data */
        /* Get the offset of the data_len field in pkt */
        data_len_offset = RB_POS_MOV(rb, uart_info->new_pkt_offset, VAR_OFFSET(uart_pkt_hdr_t, data_len));
        /* Get payload length */
        framelen = (uart_info->rb->buffer[RB_POS_MOV(rb, data_len_offset, 0)]<<0)+ \
                   (uart_info->rb->buffer[RB_POS_MOV(rb, data_len_offset, 1)]<<8);

        /* Calculate the length has been received */
        cur_len = RINGBUFFER_DISTANCE(rb, uart_info->new_pkt_offset, uart_info->analyze_offset);

        if ((cur_len + fifo_leave_size) < (framelen + MSG_HEADER_SIZE)) {
            /*Still not finish yet, record and exit*/
            uart_info->analyze_offset = RB_POS_MOV(rb, uart_info->analyze_offset, fifo_leave_size);
            uart_info->process_done = true;
            pthread_mutex_unlock(&uart_info->rb_lock);
            uart_log_dbg("still not finish now rev %d(need %d)",
                        (cur_len + fifo_leave_size),
                        (framelen + MSG_HEADER_SIZE));
            return UART_ERROR_PKT_NOT_COMP;
        }
        else {
            /* Get enough data */
            uart_info->analyze_offset = RB_POS_MOV(rb,
                                                uart_info->analyze_offset,
                                                (framelen + MSG_HEADER_SIZE - cur_len));
        }
    }
    else {
        /* FIFO length is less than the hdr */
        if(fifo_leave_size < MSG_HEADER_SIZE){
            uart_log_verb("fifo_leave_size %d is too small", fifo_leave_size);
            uart_info->process_done = true;
            pthread_mutex_unlock(&uart_info->rb_lock);
            return UART_ERROR_PKT_NOT_COMP;
        }

        uart_log_dbg("fifo_leave_size %d", fifo_leave_size);
        uart_log_dbg("read %d write %d analyze_offset %d uart_info->new_pkt_offset %d",
                    rb->read, rb->write, uart_info->analyze_offset, uart_info->new_pkt_offset);

        /** Find the pkt header through magic number
         * If there's skip bytes, treat them as dropping
         * pkt. So the pkts won't have "skip_bytes" in their
         * heads.
        */
        ret = uart_find_sync_head(uart_info, fifo_leave_size, &skip_bytes);
        if(skip_bytes != 0){
            /* Skip bytes that not being hdr, generate as a dropping pkt for rb free*/
            if (start) {
                *start = uart_info->new_pkt_offset;
            }
            if (len) {
                *len = skip_bytes;
            }
            if (ret == UART_ERROR_PKT_NOT_COMP) {
                uart_info->process_done = true;
            }
            uart_info->analyze_offset = RB_POS_MOV(rb, uart_info->analyze_offset, skip_bytes);
            uart_info->new_pkt_offset = RB_POS_MOV(rb, uart_info->new_pkt_offset, skip_bytes);
            uart_log_warn("uart_find_sync_head need skip %d ret %d", skip_bytes, ret);
            /* stats */
            uart_info->drop_gap_bytes += skip_bytes;
            pthread_mutex_unlock(&uart_info->rb_lock);
            return UART_ERROR_PKT_DROP;
        }

        /* Header has been received if reach here, analyze the header for pkt length */
        data_len_offset = RB_POS_MOV(rb, uart_info->new_pkt_offset, VAR_OFFSET(uart_pkt_hdr_t, data_len));
        /* Get payload length */
        framelen = (uart_info->rb->buffer[RB_POS_MOV(rb, data_len_offset, 0)]<<0)+ \
                   (uart_info->rb->buffer[RB_POS_MOV(rb, data_len_offset, 1)]<<8);

        /* If framelen is invalid, drop it */
        if((framelen == 0) ||
            (framelen > MSG_CMD_LENGTH_MAX) ||
            (framelen > rb_size)){
            uart_log_warn("framelen %d is invalid", framelen);
            invalid_frame_length = true;
            goto drop;
        }

        /* Set the flag that header of new pkt has been found out */
        uart_info->new_hdr_found = true;

        /* Hdr has been received, but payload hasn't been transmitted */
        if((MSG_HEADER_SIZE + framelen) > fifo_leave_size){
            /* Not recieve the whole pkt yet */
            uart_log_verb("fifo_leave_size %d framelen %d err", fifo_leave_size, framelen);
            uart_info->analyze_offset = RB_POS_MOV(rb, uart_info->analyze_offset, fifo_leave_size);
            uart_info->process_done = true;
            pthread_mutex_unlock(&uart_info->rb_lock);
            return UART_ERROR_PKT_NOT_COMP;
        }

        /* Get checksum */
        checksum_offset = RB_POS_MOV(rb, uart_info->new_pkt_offset, VAR_OFFSET(uart_pkt_hdr_t, check_sum));
        checksum = (uart_info->rb->buffer[RB_POS_MOV(rb, checksum_offset, 0)]<<0)+ \
                   (uart_info->rb->buffer[RB_POS_MOV(rb, checksum_offset, 1)]<<8);
        if (!uart_pkt_check_sum_valid_in_rb(rb, RB_POS_MOV(rb, uart_info->new_pkt_offset, MSG_HEADER_SIZE), framelen, checksum)) {
            /* Drop the pkt if checksum is invalid */
            uart_log_warn("checksum invalid!pkt:(0x%x)", checksum);
            invalid_checksum = true;
            goto drop;
        }

        uart_info->analyze_offset = RB_POS_MOV(rb, uart_info->analyze_offset, framelen + MSG_HEADER_SIZE);
    }

    /* A complete packet has been received */
    uart_info->new_hdr_found = false;
    if (start) {
        *start = uart_info->new_pkt_offset;
    }
    if (len) {
        *len = framelen + MSG_HEADER_SIZE;
    }
    if (msg_id) {
        *msg_id = rb->buffer[RB_POS_MOV(rb, uart_info->new_pkt_offset, VAR_OFFSET(uart_pkt_hdr_t, msg_id))];
    }

    /* update the offset */
    uart_info->new_pkt_offset = RB_POS_MOV(rb, uart_info->new_pkt_offset, framelen + MSG_HEADER_SIZE);

    uart_log_dbg("complete!! uart_info->new_pkt_offset %d rb->write %d", uart_info->new_pkt_offset, rb->write);
    if (uart_info->new_pkt_offset == rb->write) {
        uart_info->process_done = true;
    }

    pthread_mutex_unlock(&uart_info->rb_lock);

    return UART_ERROR_NONE;

drop:
    /* skip the current magic */
    uart_info->analyze_offset = RB_POS_MOV(rb, uart_info->analyze_offset, FRAME_SYNC_HEAD_LEN);
    ret = uart_find_sync_head(uart_info, fifo_leave_size - FRAME_SYNC_HEAD_LEN, &skip_bytes);
    /* All data in ringbuffer has been analyzed if we get UART_ERROR_PKT_NOT_COMP */
    if (ret == UART_ERROR_PKT_NOT_COMP) {
        /* Finish current loop of ringbuffer analyze */
        uart_info->process_done = true;
    }

    /* Skip bytes that not being hdr, generate as a dropping pkt for rb free*/
    if (start) {
        *start = uart_info->new_pkt_offset;
    }
    if (len) {
        *len = skip_bytes + FRAME_SYNC_HEAD_LEN;
    }

    /* Move the analyze points */
    uart_info->analyze_offset = RB_POS_MOV(rb, uart_info->analyze_offset, skip_bytes);
    uart_info->new_pkt_offset = RB_POS_MOV(rb, uart_info->new_pkt_offset, skip_bytes + FRAME_SYNC_HEAD_LEN);

    uart_log_dbg("uart_find_sync_head failed ret %d, drop %d bytes.analyze_offset %d process_done %d",
                ret, skip_bytes, uart_info->analyze_offset, uart_info->process_done);

    /* stats */
    if (invalid_frame_length) {
        uart_info->drop_flen_bytes += (skip_bytes + FRAME_SYNC_HEAD_LEN);
    }
    else if (invalid_checksum) {
        uart_info->drop_checksum_bytes += (skip_bytes + FRAME_SYNC_HEAD_LEN);
    }

    /* Drop current pkt and find a new one */
    uart_info->new_hdr_found = false;

    pthread_mutex_unlock(&uart_info->rb_lock);

    return UART_ERROR_PKT_DROP;
}

/**
* @brief  Check whether the data is a valid usr xdata pkt.
* @param  data                      buffer of the data
*         bufflen                   length of the data
* @retval UART_ERROR_NONE            succ
*         UART_ERROR_PKT_NOT_COMP    pkt not finish yet
*         Others                    fail
* @note   Ringbuffer won't pop the data after copying the pkt, but only
*         offser the start position and length of the pkt.
*/
bool uart_pkt_format_vaild(IN uint8_t* data, IN uint32_t bufflen)
{
    int i = 0;
    uint16_t u16_msgLen = 0;
    uint16_t u16_chksum;
    uint16_t calc_chksum;

    if (bufflen < MSG_HEADER_SIZE) {
        return -1;
    }

    if ((data[0] == 0xFF) && (data[1] == 0xA5) && (data[2] == 0xAA) && (data[3] == 0x5A) && (data[4] == 0xFF)) {

        u16_msgLen = data[5] + (data[6] << 8);
        u16_chksum = data[13] + (data[14] << 8);

        if (bufflen < u16_msgLen + MSG_HEADER_SIZE) {
            return false;
        }

        calc_chksum = 0;

        for (i = 0; i < u16_msgLen; i++) {
            calc_chksum += data[MSG_HEADER_SIZE + i];
        }

        if (calc_chksum == u16_chksum) {
            return true;
        }
    }

    return false;
}
#endif