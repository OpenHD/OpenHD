#pragma once
// MESSAGE OPENHD_FEC_LINK_RX_STATISTICS PACKING

#define MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS 1213


typedef struct __mavlink_openhd_fec_link_rx_statistics_t {
 uint64_t count_blocks_total; /*<  count_blocks_total*/
 uint64_t count_blocks_lost; /*<  count_blocks_lost*/
 uint64_t count_blocks_recovered; /*<  count_blocks_recovered*/
 uint64_t count_fragments_recovered; /*<  count_fragments_recovered*/
 uint64_t count_bytes_forwarded; /*<  count_bytes_forwarded*/
 uint8_t link_index; /*<  link_index*/
} mavlink_openhd_fec_link_rx_statistics_t;

#define MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN 41
#define MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_MIN_LEN 41
#define MAVLINK_MSG_ID_1213_LEN 41
#define MAVLINK_MSG_ID_1213_MIN_LEN 41

#define MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_CRC 250
#define MAVLINK_MSG_ID_1213_CRC 250



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_FEC_LINK_RX_STATISTICS { \
    1213, \
    "OPENHD_FEC_LINK_RX_STATISTICS", \
    6, \
    {  { "link_index", NULL, MAVLINK_TYPE_UINT8_T, 0, 40, offsetof(mavlink_openhd_fec_link_rx_statistics_t, link_index) }, \
         { "count_blocks_total", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_fec_link_rx_statistics_t, count_blocks_total) }, \
         { "count_blocks_lost", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_openhd_fec_link_rx_statistics_t, count_blocks_lost) }, \
         { "count_blocks_recovered", NULL, MAVLINK_TYPE_UINT64_T, 0, 16, offsetof(mavlink_openhd_fec_link_rx_statistics_t, count_blocks_recovered) }, \
         { "count_fragments_recovered", NULL, MAVLINK_TYPE_UINT64_T, 0, 24, offsetof(mavlink_openhd_fec_link_rx_statistics_t, count_fragments_recovered) }, \
         { "count_bytes_forwarded", NULL, MAVLINK_TYPE_UINT64_T, 0, 32, offsetof(mavlink_openhd_fec_link_rx_statistics_t, count_bytes_forwarded) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_FEC_LINK_RX_STATISTICS { \
    "OPENHD_FEC_LINK_RX_STATISTICS", \
    6, \
    {  { "link_index", NULL, MAVLINK_TYPE_UINT8_T, 0, 40, offsetof(mavlink_openhd_fec_link_rx_statistics_t, link_index) }, \
         { "count_blocks_total", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_fec_link_rx_statistics_t, count_blocks_total) }, \
         { "count_blocks_lost", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_openhd_fec_link_rx_statistics_t, count_blocks_lost) }, \
         { "count_blocks_recovered", NULL, MAVLINK_TYPE_UINT64_T, 0, 16, offsetof(mavlink_openhd_fec_link_rx_statistics_t, count_blocks_recovered) }, \
         { "count_fragments_recovered", NULL, MAVLINK_TYPE_UINT64_T, 0, 24, offsetof(mavlink_openhd_fec_link_rx_statistics_t, count_fragments_recovered) }, \
         { "count_bytes_forwarded", NULL, MAVLINK_TYPE_UINT64_T, 0, 32, offsetof(mavlink_openhd_fec_link_rx_statistics_t, count_bytes_forwarded) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_fec_link_rx_statistics message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param link_index  link_index
 * @param count_blocks_total  count_blocks_total
 * @param count_blocks_lost  count_blocks_lost
 * @param count_blocks_recovered  count_blocks_recovered
 * @param count_fragments_recovered  count_fragments_recovered
 * @param count_bytes_forwarded  count_bytes_forwarded
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_fec_link_rx_statistics_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t link_index, uint64_t count_blocks_total, uint64_t count_blocks_lost, uint64_t count_blocks_recovered, uint64_t count_fragments_recovered, uint64_t count_bytes_forwarded)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN];
    _mav_put_uint64_t(buf, 0, count_blocks_total);
    _mav_put_uint64_t(buf, 8, count_blocks_lost);
    _mav_put_uint64_t(buf, 16, count_blocks_recovered);
    _mav_put_uint64_t(buf, 24, count_fragments_recovered);
    _mav_put_uint64_t(buf, 32, count_bytes_forwarded);
    _mav_put_uint8_t(buf, 40, link_index);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN);
#else
    mavlink_openhd_fec_link_rx_statistics_t packet;
    packet.count_blocks_total = count_blocks_total;
    packet.count_blocks_lost = count_blocks_lost;
    packet.count_blocks_recovered = count_blocks_recovered;
    packet.count_fragments_recovered = count_fragments_recovered;
    packet.count_bytes_forwarded = count_bytes_forwarded;
    packet.link_index = link_index;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_CRC);
}

/**
 * @brief Pack a openhd_fec_link_rx_statistics message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param link_index  link_index
 * @param count_blocks_total  count_blocks_total
 * @param count_blocks_lost  count_blocks_lost
 * @param count_blocks_recovered  count_blocks_recovered
 * @param count_fragments_recovered  count_fragments_recovered
 * @param count_bytes_forwarded  count_bytes_forwarded
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_fec_link_rx_statistics_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t link_index,uint64_t count_blocks_total,uint64_t count_blocks_lost,uint64_t count_blocks_recovered,uint64_t count_fragments_recovered,uint64_t count_bytes_forwarded)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN];
    _mav_put_uint64_t(buf, 0, count_blocks_total);
    _mav_put_uint64_t(buf, 8, count_blocks_lost);
    _mav_put_uint64_t(buf, 16, count_blocks_recovered);
    _mav_put_uint64_t(buf, 24, count_fragments_recovered);
    _mav_put_uint64_t(buf, 32, count_bytes_forwarded);
    _mav_put_uint8_t(buf, 40, link_index);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN);
#else
    mavlink_openhd_fec_link_rx_statistics_t packet;
    packet.count_blocks_total = count_blocks_total;
    packet.count_blocks_lost = count_blocks_lost;
    packet.count_blocks_recovered = count_blocks_recovered;
    packet.count_fragments_recovered = count_fragments_recovered;
    packet.count_bytes_forwarded = count_bytes_forwarded;
    packet.link_index = link_index;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_CRC);
}

/**
 * @brief Encode a openhd_fec_link_rx_statistics struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_fec_link_rx_statistics C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_fec_link_rx_statistics_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_fec_link_rx_statistics_t* openhd_fec_link_rx_statistics)
{
    return mavlink_msg_openhd_fec_link_rx_statistics_pack(system_id, component_id, msg, openhd_fec_link_rx_statistics->link_index, openhd_fec_link_rx_statistics->count_blocks_total, openhd_fec_link_rx_statistics->count_blocks_lost, openhd_fec_link_rx_statistics->count_blocks_recovered, openhd_fec_link_rx_statistics->count_fragments_recovered, openhd_fec_link_rx_statistics->count_bytes_forwarded);
}

/**
 * @brief Encode a openhd_fec_link_rx_statistics struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_fec_link_rx_statistics C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_fec_link_rx_statistics_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_fec_link_rx_statistics_t* openhd_fec_link_rx_statistics)
{
    return mavlink_msg_openhd_fec_link_rx_statistics_pack_chan(system_id, component_id, chan, msg, openhd_fec_link_rx_statistics->link_index, openhd_fec_link_rx_statistics->count_blocks_total, openhd_fec_link_rx_statistics->count_blocks_lost, openhd_fec_link_rx_statistics->count_blocks_recovered, openhd_fec_link_rx_statistics->count_fragments_recovered, openhd_fec_link_rx_statistics->count_bytes_forwarded);
}

/**
 * @brief Send a openhd_fec_link_rx_statistics message
 * @param chan MAVLink channel to send the message
 *
 * @param link_index  link_index
 * @param count_blocks_total  count_blocks_total
 * @param count_blocks_lost  count_blocks_lost
 * @param count_blocks_recovered  count_blocks_recovered
 * @param count_fragments_recovered  count_fragments_recovered
 * @param count_bytes_forwarded  count_bytes_forwarded
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_fec_link_rx_statistics_send(mavlink_channel_t chan, uint8_t link_index, uint64_t count_blocks_total, uint64_t count_blocks_lost, uint64_t count_blocks_recovered, uint64_t count_fragments_recovered, uint64_t count_bytes_forwarded)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN];
    _mav_put_uint64_t(buf, 0, count_blocks_total);
    _mav_put_uint64_t(buf, 8, count_blocks_lost);
    _mav_put_uint64_t(buf, 16, count_blocks_recovered);
    _mav_put_uint64_t(buf, 24, count_fragments_recovered);
    _mav_put_uint64_t(buf, 32, count_bytes_forwarded);
    _mav_put_uint8_t(buf, 40, link_index);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS, buf, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_CRC);
#else
    mavlink_openhd_fec_link_rx_statistics_t packet;
    packet.count_blocks_total = count_blocks_total;
    packet.count_blocks_lost = count_blocks_lost;
    packet.count_blocks_recovered = count_blocks_recovered;
    packet.count_fragments_recovered = count_fragments_recovered;
    packet.count_bytes_forwarded = count_bytes_forwarded;
    packet.link_index = link_index;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_CRC);
#endif
}

/**
 * @brief Send a openhd_fec_link_rx_statistics message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_fec_link_rx_statistics_send_struct(mavlink_channel_t chan, const mavlink_openhd_fec_link_rx_statistics_t* openhd_fec_link_rx_statistics)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_fec_link_rx_statistics_send(chan, openhd_fec_link_rx_statistics->link_index, openhd_fec_link_rx_statistics->count_blocks_total, openhd_fec_link_rx_statistics->count_blocks_lost, openhd_fec_link_rx_statistics->count_blocks_recovered, openhd_fec_link_rx_statistics->count_fragments_recovered, openhd_fec_link_rx_statistics->count_bytes_forwarded);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS, (const char *)openhd_fec_link_rx_statistics, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_fec_link_rx_statistics_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t link_index, uint64_t count_blocks_total, uint64_t count_blocks_lost, uint64_t count_blocks_recovered, uint64_t count_fragments_recovered, uint64_t count_bytes_forwarded)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint64_t(buf, 0, count_blocks_total);
    _mav_put_uint64_t(buf, 8, count_blocks_lost);
    _mav_put_uint64_t(buf, 16, count_blocks_recovered);
    _mav_put_uint64_t(buf, 24, count_fragments_recovered);
    _mav_put_uint64_t(buf, 32, count_bytes_forwarded);
    _mav_put_uint8_t(buf, 40, link_index);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS, buf, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_CRC);
#else
    mavlink_openhd_fec_link_rx_statistics_t *packet = (mavlink_openhd_fec_link_rx_statistics_t *)msgbuf;
    packet->count_blocks_total = count_blocks_total;
    packet->count_blocks_lost = count_blocks_lost;
    packet->count_blocks_recovered = count_blocks_recovered;
    packet->count_fragments_recovered = count_fragments_recovered;
    packet->count_bytes_forwarded = count_bytes_forwarded;
    packet->link_index = link_index;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS, (const char *)packet, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_FEC_LINK_RX_STATISTICS UNPACKING


/**
 * @brief Get field link_index from openhd_fec_link_rx_statistics message
 *
 * @return  link_index
 */
static inline uint8_t mavlink_msg_openhd_fec_link_rx_statistics_get_link_index(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  40);
}

/**
 * @brief Get field count_blocks_total from openhd_fec_link_rx_statistics message
 *
 * @return  count_blocks_total
 */
static inline uint64_t mavlink_msg_openhd_fec_link_rx_statistics_get_count_blocks_total(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  0);
}

/**
 * @brief Get field count_blocks_lost from openhd_fec_link_rx_statistics message
 *
 * @return  count_blocks_lost
 */
static inline uint64_t mavlink_msg_openhd_fec_link_rx_statistics_get_count_blocks_lost(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  8);
}

/**
 * @brief Get field count_blocks_recovered from openhd_fec_link_rx_statistics message
 *
 * @return  count_blocks_recovered
 */
static inline uint64_t mavlink_msg_openhd_fec_link_rx_statistics_get_count_blocks_recovered(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  16);
}

/**
 * @brief Get field count_fragments_recovered from openhd_fec_link_rx_statistics message
 *
 * @return  count_fragments_recovered
 */
static inline uint64_t mavlink_msg_openhd_fec_link_rx_statistics_get_count_fragments_recovered(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  24);
}

/**
 * @brief Get field count_bytes_forwarded from openhd_fec_link_rx_statistics message
 *
 * @return  count_bytes_forwarded
 */
static inline uint64_t mavlink_msg_openhd_fec_link_rx_statistics_get_count_bytes_forwarded(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  32);
}

/**
 * @brief Decode a openhd_fec_link_rx_statistics message into a struct
 *
 * @param msg The message to decode
 * @param openhd_fec_link_rx_statistics C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_fec_link_rx_statistics_decode(const mavlink_message_t* msg, mavlink_openhd_fec_link_rx_statistics_t* openhd_fec_link_rx_statistics)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_fec_link_rx_statistics->count_blocks_total = mavlink_msg_openhd_fec_link_rx_statistics_get_count_blocks_total(msg);
    openhd_fec_link_rx_statistics->count_blocks_lost = mavlink_msg_openhd_fec_link_rx_statistics_get_count_blocks_lost(msg);
    openhd_fec_link_rx_statistics->count_blocks_recovered = mavlink_msg_openhd_fec_link_rx_statistics_get_count_blocks_recovered(msg);
    openhd_fec_link_rx_statistics->count_fragments_recovered = mavlink_msg_openhd_fec_link_rx_statistics_get_count_fragments_recovered(msg);
    openhd_fec_link_rx_statistics->count_bytes_forwarded = mavlink_msg_openhd_fec_link_rx_statistics_get_count_bytes_forwarded(msg);
    openhd_fec_link_rx_statistics->link_index = mavlink_msg_openhd_fec_link_rx_statistics_get_link_index(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN;
        memset(openhd_fec_link_rx_statistics, 0, MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_LEN);
    memcpy(openhd_fec_link_rx_statistics, _MAV_PAYLOAD(msg), len);
#endif
}
