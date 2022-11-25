#pragma once
// MESSAGE OPENHD_STATS_MONITOR_MODE_WIFI_LINK PACKING

#define MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK 1212


typedef struct __mavlink_openhd_stats_monitor_mode_wifi_link_t {
 uint64_t curr_tx_pps; /*<  tx packets per second*/
 uint64_t curr_rx_pps; /*<  rx packets per second*/
 uint64_t curr_tx_bps; /*<  tx bits per second*/
 uint64_t curr_rx_bps; /*<  rx bits per second*/
 int32_t unused0; /*<  unused0*/
 int32_t unused1; /*<  unused1*/
 int32_t unused2; /*<  unused2*/
 int32_t unused3; /*<  unused3*/
} mavlink_openhd_stats_monitor_mode_wifi_link_t;

#define MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN 48
#define MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_MIN_LEN 48
#define MAVLINK_MSG_ID_1212_LEN 48
#define MAVLINK_MSG_ID_1212_MIN_LEN 48

#define MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_CRC 135
#define MAVLINK_MSG_ID_1212_CRC 135



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_STATS_MONITOR_MODE_WIFI_LINK { \
    1212, \
    "OPENHD_STATS_MONITOR_MODE_WIFI_LINK", \
    8, \
    {  { "curr_tx_pps", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_stats_monitor_mode_wifi_link_t, curr_tx_pps) }, \
         { "curr_rx_pps", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_openhd_stats_monitor_mode_wifi_link_t, curr_rx_pps) }, \
         { "curr_tx_bps", NULL, MAVLINK_TYPE_UINT64_T, 0, 16, offsetof(mavlink_openhd_stats_monitor_mode_wifi_link_t, curr_tx_bps) }, \
         { "curr_rx_bps", NULL, MAVLINK_TYPE_UINT64_T, 0, 24, offsetof(mavlink_openhd_stats_monitor_mode_wifi_link_t, curr_rx_bps) }, \
         { "unused0", NULL, MAVLINK_TYPE_INT32_T, 0, 32, offsetof(mavlink_openhd_stats_monitor_mode_wifi_link_t, unused0) }, \
         { "unused1", NULL, MAVLINK_TYPE_INT32_T, 0, 36, offsetof(mavlink_openhd_stats_monitor_mode_wifi_link_t, unused1) }, \
         { "unused2", NULL, MAVLINK_TYPE_INT32_T, 0, 40, offsetof(mavlink_openhd_stats_monitor_mode_wifi_link_t, unused2) }, \
         { "unused3", NULL, MAVLINK_TYPE_INT32_T, 0, 44, offsetof(mavlink_openhd_stats_monitor_mode_wifi_link_t, unused3) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_STATS_MONITOR_MODE_WIFI_LINK { \
    "OPENHD_STATS_MONITOR_MODE_WIFI_LINK", \
    8, \
    {  { "curr_tx_pps", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_stats_monitor_mode_wifi_link_t, curr_tx_pps) }, \
         { "curr_rx_pps", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_openhd_stats_monitor_mode_wifi_link_t, curr_rx_pps) }, \
         { "curr_tx_bps", NULL, MAVLINK_TYPE_UINT64_T, 0, 16, offsetof(mavlink_openhd_stats_monitor_mode_wifi_link_t, curr_tx_bps) }, \
         { "curr_rx_bps", NULL, MAVLINK_TYPE_UINT64_T, 0, 24, offsetof(mavlink_openhd_stats_monitor_mode_wifi_link_t, curr_rx_bps) }, \
         { "unused0", NULL, MAVLINK_TYPE_INT32_T, 0, 32, offsetof(mavlink_openhd_stats_monitor_mode_wifi_link_t, unused0) }, \
         { "unused1", NULL, MAVLINK_TYPE_INT32_T, 0, 36, offsetof(mavlink_openhd_stats_monitor_mode_wifi_link_t, unused1) }, \
         { "unused2", NULL, MAVLINK_TYPE_INT32_T, 0, 40, offsetof(mavlink_openhd_stats_monitor_mode_wifi_link_t, unused2) }, \
         { "unused3", NULL, MAVLINK_TYPE_INT32_T, 0, 44, offsetof(mavlink_openhd_stats_monitor_mode_wifi_link_t, unused3) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_stats_monitor_mode_wifi_link message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param curr_tx_pps  tx packets per second
 * @param curr_rx_pps  rx packets per second
 * @param curr_tx_bps  tx bits per second
 * @param curr_rx_bps  rx bits per second
 * @param unused0  unused0
 * @param unused1  unused1
 * @param unused2  unused2
 * @param unused3  unused3
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_stats_monitor_mode_wifi_link_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint64_t curr_tx_pps, uint64_t curr_rx_pps, uint64_t curr_tx_bps, uint64_t curr_rx_bps, int32_t unused0, int32_t unused1, int32_t unused2, int32_t unused3)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN];
    _mav_put_uint64_t(buf, 0, curr_tx_pps);
    _mav_put_uint64_t(buf, 8, curr_rx_pps);
    _mav_put_uint64_t(buf, 16, curr_tx_bps);
    _mav_put_uint64_t(buf, 24, curr_rx_bps);
    _mav_put_int32_t(buf, 32, unused0);
    _mav_put_int32_t(buf, 36, unused1);
    _mav_put_int32_t(buf, 40, unused2);
    _mav_put_int32_t(buf, 44, unused3);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN);
#else
    mavlink_openhd_stats_monitor_mode_wifi_link_t packet;
    packet.curr_tx_pps = curr_tx_pps;
    packet.curr_rx_pps = curr_rx_pps;
    packet.curr_tx_bps = curr_tx_bps;
    packet.curr_rx_bps = curr_rx_bps;
    packet.unused0 = unused0;
    packet.unused1 = unused1;
    packet.unused2 = unused2;
    packet.unused3 = unused3;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_CRC);
}

/**
 * @brief Pack a openhd_stats_monitor_mode_wifi_link message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param curr_tx_pps  tx packets per second
 * @param curr_rx_pps  rx packets per second
 * @param curr_tx_bps  tx bits per second
 * @param curr_rx_bps  rx bits per second
 * @param unused0  unused0
 * @param unused1  unused1
 * @param unused2  unused2
 * @param unused3  unused3
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_stats_monitor_mode_wifi_link_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint64_t curr_tx_pps,uint64_t curr_rx_pps,uint64_t curr_tx_bps,uint64_t curr_rx_bps,int32_t unused0,int32_t unused1,int32_t unused2,int32_t unused3)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN];
    _mav_put_uint64_t(buf, 0, curr_tx_pps);
    _mav_put_uint64_t(buf, 8, curr_rx_pps);
    _mav_put_uint64_t(buf, 16, curr_tx_bps);
    _mav_put_uint64_t(buf, 24, curr_rx_bps);
    _mav_put_int32_t(buf, 32, unused0);
    _mav_put_int32_t(buf, 36, unused1);
    _mav_put_int32_t(buf, 40, unused2);
    _mav_put_int32_t(buf, 44, unused3);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN);
#else
    mavlink_openhd_stats_monitor_mode_wifi_link_t packet;
    packet.curr_tx_pps = curr_tx_pps;
    packet.curr_rx_pps = curr_rx_pps;
    packet.curr_tx_bps = curr_tx_bps;
    packet.curr_rx_bps = curr_rx_bps;
    packet.unused0 = unused0;
    packet.unused1 = unused1;
    packet.unused2 = unused2;
    packet.unused3 = unused3;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_CRC);
}

/**
 * @brief Encode a openhd_stats_monitor_mode_wifi_link struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_stats_monitor_mode_wifi_link C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_stats_monitor_mode_wifi_link_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_stats_monitor_mode_wifi_link_t* openhd_stats_monitor_mode_wifi_link)
{
    return mavlink_msg_openhd_stats_monitor_mode_wifi_link_pack(system_id, component_id, msg, openhd_stats_monitor_mode_wifi_link->curr_tx_pps, openhd_stats_monitor_mode_wifi_link->curr_rx_pps, openhd_stats_monitor_mode_wifi_link->curr_tx_bps, openhd_stats_monitor_mode_wifi_link->curr_rx_bps, openhd_stats_monitor_mode_wifi_link->unused0, openhd_stats_monitor_mode_wifi_link->unused1, openhd_stats_monitor_mode_wifi_link->unused2, openhd_stats_monitor_mode_wifi_link->unused3);
}

/**
 * @brief Encode a openhd_stats_monitor_mode_wifi_link struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_stats_monitor_mode_wifi_link C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_stats_monitor_mode_wifi_link_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_stats_monitor_mode_wifi_link_t* openhd_stats_monitor_mode_wifi_link)
{
    return mavlink_msg_openhd_stats_monitor_mode_wifi_link_pack_chan(system_id, component_id, chan, msg, openhd_stats_monitor_mode_wifi_link->curr_tx_pps, openhd_stats_monitor_mode_wifi_link->curr_rx_pps, openhd_stats_monitor_mode_wifi_link->curr_tx_bps, openhd_stats_monitor_mode_wifi_link->curr_rx_bps, openhd_stats_monitor_mode_wifi_link->unused0, openhd_stats_monitor_mode_wifi_link->unused1, openhd_stats_monitor_mode_wifi_link->unused2, openhd_stats_monitor_mode_wifi_link->unused3);
}

/**
 * @brief Send a openhd_stats_monitor_mode_wifi_link message
 * @param chan MAVLink channel to send the message
 *
 * @param curr_tx_pps  tx packets per second
 * @param curr_rx_pps  rx packets per second
 * @param curr_tx_bps  tx bits per second
 * @param curr_rx_bps  rx bits per second
 * @param unused0  unused0
 * @param unused1  unused1
 * @param unused2  unused2
 * @param unused3  unused3
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_stats_monitor_mode_wifi_link_send(mavlink_channel_t chan, uint64_t curr_tx_pps, uint64_t curr_rx_pps, uint64_t curr_tx_bps, uint64_t curr_rx_bps, int32_t unused0, int32_t unused1, int32_t unused2, int32_t unused3)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN];
    _mav_put_uint64_t(buf, 0, curr_tx_pps);
    _mav_put_uint64_t(buf, 8, curr_rx_pps);
    _mav_put_uint64_t(buf, 16, curr_tx_bps);
    _mav_put_uint64_t(buf, 24, curr_rx_bps);
    _mav_put_int32_t(buf, 32, unused0);
    _mav_put_int32_t(buf, 36, unused1);
    _mav_put_int32_t(buf, 40, unused2);
    _mav_put_int32_t(buf, 44, unused3);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK, buf, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_CRC);
#else
    mavlink_openhd_stats_monitor_mode_wifi_link_t packet;
    packet.curr_tx_pps = curr_tx_pps;
    packet.curr_rx_pps = curr_rx_pps;
    packet.curr_tx_bps = curr_tx_bps;
    packet.curr_rx_bps = curr_rx_bps;
    packet.unused0 = unused0;
    packet.unused1 = unused1;
    packet.unused2 = unused2;
    packet.unused3 = unused3;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_CRC);
#endif
}

/**
 * @brief Send a openhd_stats_monitor_mode_wifi_link message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_stats_monitor_mode_wifi_link_send_struct(mavlink_channel_t chan, const mavlink_openhd_stats_monitor_mode_wifi_link_t* openhd_stats_monitor_mode_wifi_link)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_stats_monitor_mode_wifi_link_send(chan, openhd_stats_monitor_mode_wifi_link->curr_tx_pps, openhd_stats_monitor_mode_wifi_link->curr_rx_pps, openhd_stats_monitor_mode_wifi_link->curr_tx_bps, openhd_stats_monitor_mode_wifi_link->curr_rx_bps, openhd_stats_monitor_mode_wifi_link->unused0, openhd_stats_monitor_mode_wifi_link->unused1, openhd_stats_monitor_mode_wifi_link->unused2, openhd_stats_monitor_mode_wifi_link->unused3);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK, (const char *)openhd_stats_monitor_mode_wifi_link, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_stats_monitor_mode_wifi_link_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint64_t curr_tx_pps, uint64_t curr_rx_pps, uint64_t curr_tx_bps, uint64_t curr_rx_bps, int32_t unused0, int32_t unused1, int32_t unused2, int32_t unused3)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint64_t(buf, 0, curr_tx_pps);
    _mav_put_uint64_t(buf, 8, curr_rx_pps);
    _mav_put_uint64_t(buf, 16, curr_tx_bps);
    _mav_put_uint64_t(buf, 24, curr_rx_bps);
    _mav_put_int32_t(buf, 32, unused0);
    _mav_put_int32_t(buf, 36, unused1);
    _mav_put_int32_t(buf, 40, unused2);
    _mav_put_int32_t(buf, 44, unused3);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK, buf, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_CRC);
#else
    mavlink_openhd_stats_monitor_mode_wifi_link_t *packet = (mavlink_openhd_stats_monitor_mode_wifi_link_t *)msgbuf;
    packet->curr_tx_pps = curr_tx_pps;
    packet->curr_rx_pps = curr_rx_pps;
    packet->curr_tx_bps = curr_tx_bps;
    packet->curr_rx_bps = curr_rx_bps;
    packet->unused0 = unused0;
    packet->unused1 = unused1;
    packet->unused2 = unused2;
    packet->unused3 = unused3;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK, (const char *)packet, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_STATS_MONITOR_MODE_WIFI_LINK UNPACKING


/**
 * @brief Get field curr_tx_pps from openhd_stats_monitor_mode_wifi_link message
 *
 * @return  tx packets per second
 */
static inline uint64_t mavlink_msg_openhd_stats_monitor_mode_wifi_link_get_curr_tx_pps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  0);
}

/**
 * @brief Get field curr_rx_pps from openhd_stats_monitor_mode_wifi_link message
 *
 * @return  rx packets per second
 */
static inline uint64_t mavlink_msg_openhd_stats_monitor_mode_wifi_link_get_curr_rx_pps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  8);
}

/**
 * @brief Get field curr_tx_bps from openhd_stats_monitor_mode_wifi_link message
 *
 * @return  tx bits per second
 */
static inline uint64_t mavlink_msg_openhd_stats_monitor_mode_wifi_link_get_curr_tx_bps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  16);
}

/**
 * @brief Get field curr_rx_bps from openhd_stats_monitor_mode_wifi_link message
 *
 * @return  rx bits per second
 */
static inline uint64_t mavlink_msg_openhd_stats_monitor_mode_wifi_link_get_curr_rx_bps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  24);
}

/**
 * @brief Get field unused0 from openhd_stats_monitor_mode_wifi_link message
 *
 * @return  unused0
 */
static inline int32_t mavlink_msg_openhd_stats_monitor_mode_wifi_link_get_unused0(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  32);
}

/**
 * @brief Get field unused1 from openhd_stats_monitor_mode_wifi_link message
 *
 * @return  unused1
 */
static inline int32_t mavlink_msg_openhd_stats_monitor_mode_wifi_link_get_unused1(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  36);
}

/**
 * @brief Get field unused2 from openhd_stats_monitor_mode_wifi_link message
 *
 * @return  unused2
 */
static inline int32_t mavlink_msg_openhd_stats_monitor_mode_wifi_link_get_unused2(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  40);
}

/**
 * @brief Get field unused3 from openhd_stats_monitor_mode_wifi_link message
 *
 * @return  unused3
 */
static inline int32_t mavlink_msg_openhd_stats_monitor_mode_wifi_link_get_unused3(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  44);
}

/**
 * @brief Decode a openhd_stats_monitor_mode_wifi_link message into a struct
 *
 * @param msg The message to decode
 * @param openhd_stats_monitor_mode_wifi_link C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_stats_monitor_mode_wifi_link_decode(const mavlink_message_t* msg, mavlink_openhd_stats_monitor_mode_wifi_link_t* openhd_stats_monitor_mode_wifi_link)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_stats_monitor_mode_wifi_link->curr_tx_pps = mavlink_msg_openhd_stats_monitor_mode_wifi_link_get_curr_tx_pps(msg);
    openhd_stats_monitor_mode_wifi_link->curr_rx_pps = mavlink_msg_openhd_stats_monitor_mode_wifi_link_get_curr_rx_pps(msg);
    openhd_stats_monitor_mode_wifi_link->curr_tx_bps = mavlink_msg_openhd_stats_monitor_mode_wifi_link_get_curr_tx_bps(msg);
    openhd_stats_monitor_mode_wifi_link->curr_rx_bps = mavlink_msg_openhd_stats_monitor_mode_wifi_link_get_curr_rx_bps(msg);
    openhd_stats_monitor_mode_wifi_link->unused0 = mavlink_msg_openhd_stats_monitor_mode_wifi_link_get_unused0(msg);
    openhd_stats_monitor_mode_wifi_link->unused1 = mavlink_msg_openhd_stats_monitor_mode_wifi_link_get_unused1(msg);
    openhd_stats_monitor_mode_wifi_link->unused2 = mavlink_msg_openhd_stats_monitor_mode_wifi_link_get_unused2(msg);
    openhd_stats_monitor_mode_wifi_link->unused3 = mavlink_msg_openhd_stats_monitor_mode_wifi_link_get_unused3(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN;
        memset(openhd_stats_monitor_mode_wifi_link, 0, MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_LEN);
    memcpy(openhd_stats_monitor_mode_wifi_link, _MAV_PAYLOAD(msg), len);
#endif
}
