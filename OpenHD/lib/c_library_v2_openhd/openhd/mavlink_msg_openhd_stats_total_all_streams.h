#pragma once
// MESSAGE OPENHD_STATS_TOTAL_ALL_STREAMS PACKING

#define MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS 1215


typedef struct __mavlink_openhd_stats_total_all_streams_t {
 uint64_t count_wifi_packets_received; /*<  current count of all received Wi-Fi packets*/
 uint64_t count_bytes_received; /*<  current count of all received bytes, does not include IEE802 header or similar, but does include FEC overhead*/
 uint64_t count_wifi_packets_injected; /*<  current count of all injected Wi-Fi packets*/
 uint64_t count_bytes_injected; /*<  current count of all outgoing bytes, does not include IEE802 header or similar, but does include FEC overhead*/
} mavlink_openhd_stats_total_all_streams_t;

#define MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN 32
#define MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_MIN_LEN 32
#define MAVLINK_MSG_ID_1215_LEN 32
#define MAVLINK_MSG_ID_1215_MIN_LEN 32

#define MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_CRC 6
#define MAVLINK_MSG_ID_1215_CRC 6



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_STATS_TOTAL_ALL_STREAMS { \
    1215, \
    "OPENHD_STATS_TOTAL_ALL_STREAMS", \
    4, \
    {  { "count_wifi_packets_received", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_stats_total_all_streams_t, count_wifi_packets_received) }, \
         { "count_bytes_received", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_openhd_stats_total_all_streams_t, count_bytes_received) }, \
         { "count_wifi_packets_injected", NULL, MAVLINK_TYPE_UINT64_T, 0, 16, offsetof(mavlink_openhd_stats_total_all_streams_t, count_wifi_packets_injected) }, \
         { "count_bytes_injected", NULL, MAVLINK_TYPE_UINT64_T, 0, 24, offsetof(mavlink_openhd_stats_total_all_streams_t, count_bytes_injected) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_STATS_TOTAL_ALL_STREAMS { \
    "OPENHD_STATS_TOTAL_ALL_STREAMS", \
    4, \
    {  { "count_wifi_packets_received", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_stats_total_all_streams_t, count_wifi_packets_received) }, \
         { "count_bytes_received", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_openhd_stats_total_all_streams_t, count_bytes_received) }, \
         { "count_wifi_packets_injected", NULL, MAVLINK_TYPE_UINT64_T, 0, 16, offsetof(mavlink_openhd_stats_total_all_streams_t, count_wifi_packets_injected) }, \
         { "count_bytes_injected", NULL, MAVLINK_TYPE_UINT64_T, 0, 24, offsetof(mavlink_openhd_stats_total_all_streams_t, count_bytes_injected) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_stats_total_all_streams message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param count_wifi_packets_received  current count of all received Wi-Fi packets
 * @param count_bytes_received  current count of all received bytes, does not include IEE802 header or similar, but does include FEC overhead
 * @param count_wifi_packets_injected  current count of all injected Wi-Fi packets
 * @param count_bytes_injected  current count of all outgoing bytes, does not include IEE802 header or similar, but does include FEC overhead
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_stats_total_all_streams_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint64_t count_wifi_packets_received, uint64_t count_bytes_received, uint64_t count_wifi_packets_injected, uint64_t count_bytes_injected)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN];
    _mav_put_uint64_t(buf, 0, count_wifi_packets_received);
    _mav_put_uint64_t(buf, 8, count_bytes_received);
    _mav_put_uint64_t(buf, 16, count_wifi_packets_injected);
    _mav_put_uint64_t(buf, 24, count_bytes_injected);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN);
#else
    mavlink_openhd_stats_total_all_streams_t packet;
    packet.count_wifi_packets_received = count_wifi_packets_received;
    packet.count_bytes_received = count_bytes_received;
    packet.count_wifi_packets_injected = count_wifi_packets_injected;
    packet.count_bytes_injected = count_bytes_injected;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_CRC);
}

/**
 * @brief Pack a openhd_stats_total_all_streams message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param count_wifi_packets_received  current count of all received Wi-Fi packets
 * @param count_bytes_received  current count of all received bytes, does not include IEE802 header or similar, but does include FEC overhead
 * @param count_wifi_packets_injected  current count of all injected Wi-Fi packets
 * @param count_bytes_injected  current count of all outgoing bytes, does not include IEE802 header or similar, but does include FEC overhead
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_stats_total_all_streams_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint64_t count_wifi_packets_received,uint64_t count_bytes_received,uint64_t count_wifi_packets_injected,uint64_t count_bytes_injected)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN];
    _mav_put_uint64_t(buf, 0, count_wifi_packets_received);
    _mav_put_uint64_t(buf, 8, count_bytes_received);
    _mav_put_uint64_t(buf, 16, count_wifi_packets_injected);
    _mav_put_uint64_t(buf, 24, count_bytes_injected);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN);
#else
    mavlink_openhd_stats_total_all_streams_t packet;
    packet.count_wifi_packets_received = count_wifi_packets_received;
    packet.count_bytes_received = count_bytes_received;
    packet.count_wifi_packets_injected = count_wifi_packets_injected;
    packet.count_bytes_injected = count_bytes_injected;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_CRC);
}

/**
 * @brief Encode a openhd_stats_total_all_streams struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_stats_total_all_streams C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_stats_total_all_streams_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_stats_total_all_streams_t* openhd_stats_total_all_streams)
{
    return mavlink_msg_openhd_stats_total_all_streams_pack(system_id, component_id, msg, openhd_stats_total_all_streams->count_wifi_packets_received, openhd_stats_total_all_streams->count_bytes_received, openhd_stats_total_all_streams->count_wifi_packets_injected, openhd_stats_total_all_streams->count_bytes_injected);
}

/**
 * @brief Encode a openhd_stats_total_all_streams struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_stats_total_all_streams C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_stats_total_all_streams_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_stats_total_all_streams_t* openhd_stats_total_all_streams)
{
    return mavlink_msg_openhd_stats_total_all_streams_pack_chan(system_id, component_id, chan, msg, openhd_stats_total_all_streams->count_wifi_packets_received, openhd_stats_total_all_streams->count_bytes_received, openhd_stats_total_all_streams->count_wifi_packets_injected, openhd_stats_total_all_streams->count_bytes_injected);
}

/**
 * @brief Send a openhd_stats_total_all_streams message
 * @param chan MAVLink channel to send the message
 *
 * @param count_wifi_packets_received  current count of all received Wi-Fi packets
 * @param count_bytes_received  current count of all received bytes, does not include IEE802 header or similar, but does include FEC overhead
 * @param count_wifi_packets_injected  current count of all injected Wi-Fi packets
 * @param count_bytes_injected  current count of all outgoing bytes, does not include IEE802 header or similar, but does include FEC overhead
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_stats_total_all_streams_send(mavlink_channel_t chan, uint64_t count_wifi_packets_received, uint64_t count_bytes_received, uint64_t count_wifi_packets_injected, uint64_t count_bytes_injected)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN];
    _mav_put_uint64_t(buf, 0, count_wifi_packets_received);
    _mav_put_uint64_t(buf, 8, count_bytes_received);
    _mav_put_uint64_t(buf, 16, count_wifi_packets_injected);
    _mav_put_uint64_t(buf, 24, count_bytes_injected);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS, buf, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_CRC);
#else
    mavlink_openhd_stats_total_all_streams_t packet;
    packet.count_wifi_packets_received = count_wifi_packets_received;
    packet.count_bytes_received = count_bytes_received;
    packet.count_wifi_packets_injected = count_wifi_packets_injected;
    packet.count_bytes_injected = count_bytes_injected;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_CRC);
#endif
}

/**
 * @brief Send a openhd_stats_total_all_streams message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_stats_total_all_streams_send_struct(mavlink_channel_t chan, const mavlink_openhd_stats_total_all_streams_t* openhd_stats_total_all_streams)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_stats_total_all_streams_send(chan, openhd_stats_total_all_streams->count_wifi_packets_received, openhd_stats_total_all_streams->count_bytes_received, openhd_stats_total_all_streams->count_wifi_packets_injected, openhd_stats_total_all_streams->count_bytes_injected);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS, (const char *)openhd_stats_total_all_streams, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_stats_total_all_streams_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint64_t count_wifi_packets_received, uint64_t count_bytes_received, uint64_t count_wifi_packets_injected, uint64_t count_bytes_injected)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint64_t(buf, 0, count_wifi_packets_received);
    _mav_put_uint64_t(buf, 8, count_bytes_received);
    _mav_put_uint64_t(buf, 16, count_wifi_packets_injected);
    _mav_put_uint64_t(buf, 24, count_bytes_injected);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS, buf, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_CRC);
#else
    mavlink_openhd_stats_total_all_streams_t *packet = (mavlink_openhd_stats_total_all_streams_t *)msgbuf;
    packet->count_wifi_packets_received = count_wifi_packets_received;
    packet->count_bytes_received = count_bytes_received;
    packet->count_wifi_packets_injected = count_wifi_packets_injected;
    packet->count_bytes_injected = count_bytes_injected;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS, (const char *)packet, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_STATS_TOTAL_ALL_STREAMS UNPACKING


/**
 * @brief Get field count_wifi_packets_received from openhd_stats_total_all_streams message
 *
 * @return  current count of all received Wi-Fi packets
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_streams_get_count_wifi_packets_received(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  0);
}

/**
 * @brief Get field count_bytes_received from openhd_stats_total_all_streams message
 *
 * @return  current count of all received bytes, does not include IEE802 header or similar, but does include FEC overhead
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_streams_get_count_bytes_received(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  8);
}

/**
 * @brief Get field count_wifi_packets_injected from openhd_stats_total_all_streams message
 *
 * @return  current count of all injected Wi-Fi packets
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_streams_get_count_wifi_packets_injected(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  16);
}

/**
 * @brief Get field count_bytes_injected from openhd_stats_total_all_streams message
 *
 * @return  current count of all outgoing bytes, does not include IEE802 header or similar, but does include FEC overhead
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_streams_get_count_bytes_injected(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  24);
}

/**
 * @brief Decode a openhd_stats_total_all_streams message into a struct
 *
 * @param msg The message to decode
 * @param openhd_stats_total_all_streams C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_stats_total_all_streams_decode(const mavlink_message_t* msg, mavlink_openhd_stats_total_all_streams_t* openhd_stats_total_all_streams)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_stats_total_all_streams->count_wifi_packets_received = mavlink_msg_openhd_stats_total_all_streams_get_count_wifi_packets_received(msg);
    openhd_stats_total_all_streams->count_bytes_received = mavlink_msg_openhd_stats_total_all_streams_get_count_bytes_received(msg);
    openhd_stats_total_all_streams->count_wifi_packets_injected = mavlink_msg_openhd_stats_total_all_streams_get_count_wifi_packets_injected(msg);
    openhd_stats_total_all_streams->count_bytes_injected = mavlink_msg_openhd_stats_total_all_streams_get_count_bytes_injected(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN;
        memset(openhd_stats_total_all_streams, 0, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_STREAMS_LEN);
    memcpy(openhd_stats_total_all_streams, _MAV_PAYLOAD(msg), len);
#endif
}
