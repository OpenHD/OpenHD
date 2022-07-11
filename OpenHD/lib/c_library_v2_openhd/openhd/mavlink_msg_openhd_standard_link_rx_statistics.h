#pragma once
// MESSAGE OPENHD_STANDARD_LINK_RX_STATISTICS PACKING

#define MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS 1214


typedef struct __mavlink_openhd_standard_link_rx_statistics_t {
 uint32_t count_p_received; /*<  All received (incoming) packets, not suported by all cards*/
 uint32_t count_p_lost; /*<  All injected (outgoing) packets, not suported by all cards*/
 uint8_t link_index; /*<  link_index*/
} mavlink_openhd_standard_link_rx_statistics_t;

#define MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN 9
#define MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_MIN_LEN 9
#define MAVLINK_MSG_ID_1214_LEN 9
#define MAVLINK_MSG_ID_1214_MIN_LEN 9

#define MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_CRC 131
#define MAVLINK_MSG_ID_1214_CRC 131



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_STANDARD_LINK_RX_STATISTICS { \
    1214, \
    "OPENHD_STANDARD_LINK_RX_STATISTICS", \
    3, \
    {  { "link_index", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_openhd_standard_link_rx_statistics_t, link_index) }, \
         { "count_p_received", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_openhd_standard_link_rx_statistics_t, count_p_received) }, \
         { "count_p_lost", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_openhd_standard_link_rx_statistics_t, count_p_lost) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_STANDARD_LINK_RX_STATISTICS { \
    "OPENHD_STANDARD_LINK_RX_STATISTICS", \
    3, \
    {  { "link_index", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_openhd_standard_link_rx_statistics_t, link_index) }, \
         { "count_p_received", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_openhd_standard_link_rx_statistics_t, count_p_received) }, \
         { "count_p_lost", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_openhd_standard_link_rx_statistics_t, count_p_lost) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_standard_link_rx_statistics message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param link_index  link_index
 * @param count_p_received  All received (incoming) packets, not suported by all cards
 * @param count_p_lost  All injected (outgoing) packets, not suported by all cards
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_standard_link_rx_statistics_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t link_index, uint32_t count_p_received, uint32_t count_p_lost)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN];
    _mav_put_uint32_t(buf, 0, count_p_received);
    _mav_put_uint32_t(buf, 4, count_p_lost);
    _mav_put_uint8_t(buf, 8, link_index);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN);
#else
    mavlink_openhd_standard_link_rx_statistics_t packet;
    packet.count_p_received = count_p_received;
    packet.count_p_lost = count_p_lost;
    packet.link_index = link_index;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_CRC);
}

/**
 * @brief Pack a openhd_standard_link_rx_statistics message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param link_index  link_index
 * @param count_p_received  All received (incoming) packets, not suported by all cards
 * @param count_p_lost  All injected (outgoing) packets, not suported by all cards
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_standard_link_rx_statistics_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t link_index,uint32_t count_p_received,uint32_t count_p_lost)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN];
    _mav_put_uint32_t(buf, 0, count_p_received);
    _mav_put_uint32_t(buf, 4, count_p_lost);
    _mav_put_uint8_t(buf, 8, link_index);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN);
#else
    mavlink_openhd_standard_link_rx_statistics_t packet;
    packet.count_p_received = count_p_received;
    packet.count_p_lost = count_p_lost;
    packet.link_index = link_index;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_CRC);
}

/**
 * @brief Encode a openhd_standard_link_rx_statistics struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_standard_link_rx_statistics C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_standard_link_rx_statistics_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_standard_link_rx_statistics_t* openhd_standard_link_rx_statistics)
{
    return mavlink_msg_openhd_standard_link_rx_statistics_pack(system_id, component_id, msg, openhd_standard_link_rx_statistics->link_index, openhd_standard_link_rx_statistics->count_p_received, openhd_standard_link_rx_statistics->count_p_lost);
}

/**
 * @brief Encode a openhd_standard_link_rx_statistics struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_standard_link_rx_statistics C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_standard_link_rx_statistics_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_standard_link_rx_statistics_t* openhd_standard_link_rx_statistics)
{
    return mavlink_msg_openhd_standard_link_rx_statistics_pack_chan(system_id, component_id, chan, msg, openhd_standard_link_rx_statistics->link_index, openhd_standard_link_rx_statistics->count_p_received, openhd_standard_link_rx_statistics->count_p_lost);
}

/**
 * @brief Send a openhd_standard_link_rx_statistics message
 * @param chan MAVLink channel to send the message
 *
 * @param link_index  link_index
 * @param count_p_received  All received (incoming) packets, not suported by all cards
 * @param count_p_lost  All injected (outgoing) packets, not suported by all cards
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_standard_link_rx_statistics_send(mavlink_channel_t chan, uint8_t link_index, uint32_t count_p_received, uint32_t count_p_lost)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN];
    _mav_put_uint32_t(buf, 0, count_p_received);
    _mav_put_uint32_t(buf, 4, count_p_lost);
    _mav_put_uint8_t(buf, 8, link_index);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS, buf, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_CRC);
#else
    mavlink_openhd_standard_link_rx_statistics_t packet;
    packet.count_p_received = count_p_received;
    packet.count_p_lost = count_p_lost;
    packet.link_index = link_index;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_CRC);
#endif
}

/**
 * @brief Send a openhd_standard_link_rx_statistics message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_standard_link_rx_statistics_send_struct(mavlink_channel_t chan, const mavlink_openhd_standard_link_rx_statistics_t* openhd_standard_link_rx_statistics)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_standard_link_rx_statistics_send(chan, openhd_standard_link_rx_statistics->link_index, openhd_standard_link_rx_statistics->count_p_received, openhd_standard_link_rx_statistics->count_p_lost);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS, (const char *)openhd_standard_link_rx_statistics, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_standard_link_rx_statistics_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t link_index, uint32_t count_p_received, uint32_t count_p_lost)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint32_t(buf, 0, count_p_received);
    _mav_put_uint32_t(buf, 4, count_p_lost);
    _mav_put_uint8_t(buf, 8, link_index);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS, buf, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_CRC);
#else
    mavlink_openhd_standard_link_rx_statistics_t *packet = (mavlink_openhd_standard_link_rx_statistics_t *)msgbuf;
    packet->count_p_received = count_p_received;
    packet->count_p_lost = count_p_lost;
    packet->link_index = link_index;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS, (const char *)packet, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_STANDARD_LINK_RX_STATISTICS UNPACKING


/**
 * @brief Get field link_index from openhd_standard_link_rx_statistics message
 *
 * @return  link_index
 */
static inline uint8_t mavlink_msg_openhd_standard_link_rx_statistics_get_link_index(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  8);
}

/**
 * @brief Get field count_p_received from openhd_standard_link_rx_statistics message
 *
 * @return  All received (incoming) packets, not suported by all cards
 */
static inline uint32_t mavlink_msg_openhd_standard_link_rx_statistics_get_count_p_received(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Get field count_p_lost from openhd_standard_link_rx_statistics message
 *
 * @return  All injected (outgoing) packets, not suported by all cards
 */
static inline uint32_t mavlink_msg_openhd_standard_link_rx_statistics_get_count_p_lost(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  4);
}

/**
 * @brief Decode a openhd_standard_link_rx_statistics message into a struct
 *
 * @param msg The message to decode
 * @param openhd_standard_link_rx_statistics C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_standard_link_rx_statistics_decode(const mavlink_message_t* msg, mavlink_openhd_standard_link_rx_statistics_t* openhd_standard_link_rx_statistics)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_standard_link_rx_statistics->count_p_received = mavlink_msg_openhd_standard_link_rx_statistics_get_count_p_received(msg);
    openhd_standard_link_rx_statistics->count_p_lost = mavlink_msg_openhd_standard_link_rx_statistics_get_count_p_lost(msg);
    openhd_standard_link_rx_statistics->link_index = mavlink_msg_openhd_standard_link_rx_statistics_get_link_index(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN;
        memset(openhd_standard_link_rx_statistics, 0, MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_LEN);
    memcpy(openhd_standard_link_rx_statistics, _MAV_PAYLOAD(msg), len);
#endif
}
