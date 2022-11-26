#pragma once
// MESSAGE OPENHD_STATS_TELEMETRY PACKING

#define MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY 1213


typedef struct __mavlink_openhd_stats_telemetry_t {
 uint64_t unused_0; /*<  unused_0*/
 uint64_t unused_1; /*<  unused_1*/
 int32_t curr_tx_pps; /*<  tx packets per second*/
 int32_t curr_rx_pps; /*<  rx packets per second*/
 int32_t curr_tx_bps; /*<  tx bits per second*/
 int32_t curr_rx_bps; /*<  rx bits per second*/
 int16_t curr_rx_packet_loss_perc; /*<  curr_rx_packet_loss_perc*/
} mavlink_openhd_stats_telemetry_t;

#define MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN 34
#define MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_MIN_LEN 34
#define MAVLINK_MSG_ID_1213_LEN 34
#define MAVLINK_MSG_ID_1213_MIN_LEN 34

#define MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_CRC 198
#define MAVLINK_MSG_ID_1213_CRC 198



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_STATS_TELEMETRY { \
    1213, \
    "OPENHD_STATS_TELEMETRY", \
    7, \
    {  { "curr_tx_pps", NULL, MAVLINK_TYPE_INT32_T, 0, 16, offsetof(mavlink_openhd_stats_telemetry_t, curr_tx_pps) }, \
         { "curr_rx_pps", NULL, MAVLINK_TYPE_INT32_T, 0, 20, offsetof(mavlink_openhd_stats_telemetry_t, curr_rx_pps) }, \
         { "curr_tx_bps", NULL, MAVLINK_TYPE_INT32_T, 0, 24, offsetof(mavlink_openhd_stats_telemetry_t, curr_tx_bps) }, \
         { "curr_rx_bps", NULL, MAVLINK_TYPE_INT32_T, 0, 28, offsetof(mavlink_openhd_stats_telemetry_t, curr_rx_bps) }, \
         { "curr_rx_packet_loss_perc", NULL, MAVLINK_TYPE_INT16_T, 0, 32, offsetof(mavlink_openhd_stats_telemetry_t, curr_rx_packet_loss_perc) }, \
         { "unused_0", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_stats_telemetry_t, unused_0) }, \
         { "unused_1", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_openhd_stats_telemetry_t, unused_1) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_STATS_TELEMETRY { \
    "OPENHD_STATS_TELEMETRY", \
    7, \
    {  { "curr_tx_pps", NULL, MAVLINK_TYPE_INT32_T, 0, 16, offsetof(mavlink_openhd_stats_telemetry_t, curr_tx_pps) }, \
         { "curr_rx_pps", NULL, MAVLINK_TYPE_INT32_T, 0, 20, offsetof(mavlink_openhd_stats_telemetry_t, curr_rx_pps) }, \
         { "curr_tx_bps", NULL, MAVLINK_TYPE_INT32_T, 0, 24, offsetof(mavlink_openhd_stats_telemetry_t, curr_tx_bps) }, \
         { "curr_rx_bps", NULL, MAVLINK_TYPE_INT32_T, 0, 28, offsetof(mavlink_openhd_stats_telemetry_t, curr_rx_bps) }, \
         { "curr_rx_packet_loss_perc", NULL, MAVLINK_TYPE_INT16_T, 0, 32, offsetof(mavlink_openhd_stats_telemetry_t, curr_rx_packet_loss_perc) }, \
         { "unused_0", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_stats_telemetry_t, unused_0) }, \
         { "unused_1", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_openhd_stats_telemetry_t, unused_1) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_stats_telemetry message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param curr_tx_pps  tx packets per second
 * @param curr_rx_pps  rx packets per second
 * @param curr_tx_bps  tx bits per second
 * @param curr_rx_bps  rx bits per second
 * @param curr_rx_packet_loss_perc  curr_rx_packet_loss_perc
 * @param unused_0  unused_0
 * @param unused_1  unused_1
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_stats_telemetry_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               int32_t curr_tx_pps, int32_t curr_rx_pps, int32_t curr_tx_bps, int32_t curr_rx_bps, int16_t curr_rx_packet_loss_perc, uint64_t unused_0, uint64_t unused_1)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN];
    _mav_put_uint64_t(buf, 0, unused_0);
    _mav_put_uint64_t(buf, 8, unused_1);
    _mav_put_int32_t(buf, 16, curr_tx_pps);
    _mav_put_int32_t(buf, 20, curr_rx_pps);
    _mav_put_int32_t(buf, 24, curr_tx_bps);
    _mav_put_int32_t(buf, 28, curr_rx_bps);
    _mav_put_int16_t(buf, 32, curr_rx_packet_loss_perc);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN);
#else
    mavlink_openhd_stats_telemetry_t packet;
    packet.unused_0 = unused_0;
    packet.unused_1 = unused_1;
    packet.curr_tx_pps = curr_tx_pps;
    packet.curr_rx_pps = curr_rx_pps;
    packet.curr_tx_bps = curr_tx_bps;
    packet.curr_rx_bps = curr_rx_bps;
    packet.curr_rx_packet_loss_perc = curr_rx_packet_loss_perc;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_CRC);
}

/**
 * @brief Pack a openhd_stats_telemetry message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param curr_tx_pps  tx packets per second
 * @param curr_rx_pps  rx packets per second
 * @param curr_tx_bps  tx bits per second
 * @param curr_rx_bps  rx bits per second
 * @param curr_rx_packet_loss_perc  curr_rx_packet_loss_perc
 * @param unused_0  unused_0
 * @param unused_1  unused_1
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_stats_telemetry_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   int32_t curr_tx_pps,int32_t curr_rx_pps,int32_t curr_tx_bps,int32_t curr_rx_bps,int16_t curr_rx_packet_loss_perc,uint64_t unused_0,uint64_t unused_1)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN];
    _mav_put_uint64_t(buf, 0, unused_0);
    _mav_put_uint64_t(buf, 8, unused_1);
    _mav_put_int32_t(buf, 16, curr_tx_pps);
    _mav_put_int32_t(buf, 20, curr_rx_pps);
    _mav_put_int32_t(buf, 24, curr_tx_bps);
    _mav_put_int32_t(buf, 28, curr_rx_bps);
    _mav_put_int16_t(buf, 32, curr_rx_packet_loss_perc);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN);
#else
    mavlink_openhd_stats_telemetry_t packet;
    packet.unused_0 = unused_0;
    packet.unused_1 = unused_1;
    packet.curr_tx_pps = curr_tx_pps;
    packet.curr_rx_pps = curr_rx_pps;
    packet.curr_tx_bps = curr_tx_bps;
    packet.curr_rx_bps = curr_rx_bps;
    packet.curr_rx_packet_loss_perc = curr_rx_packet_loss_perc;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_CRC);
}

/**
 * @brief Encode a openhd_stats_telemetry struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_stats_telemetry C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_stats_telemetry_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_stats_telemetry_t* openhd_stats_telemetry)
{
    return mavlink_msg_openhd_stats_telemetry_pack(system_id, component_id, msg, openhd_stats_telemetry->curr_tx_pps, openhd_stats_telemetry->curr_rx_pps, openhd_stats_telemetry->curr_tx_bps, openhd_stats_telemetry->curr_rx_bps, openhd_stats_telemetry->curr_rx_packet_loss_perc, openhd_stats_telemetry->unused_0, openhd_stats_telemetry->unused_1);
}

/**
 * @brief Encode a openhd_stats_telemetry struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_stats_telemetry C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_stats_telemetry_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_stats_telemetry_t* openhd_stats_telemetry)
{
    return mavlink_msg_openhd_stats_telemetry_pack_chan(system_id, component_id, chan, msg, openhd_stats_telemetry->curr_tx_pps, openhd_stats_telemetry->curr_rx_pps, openhd_stats_telemetry->curr_tx_bps, openhd_stats_telemetry->curr_rx_bps, openhd_stats_telemetry->curr_rx_packet_loss_perc, openhd_stats_telemetry->unused_0, openhd_stats_telemetry->unused_1);
}

/**
 * @brief Send a openhd_stats_telemetry message
 * @param chan MAVLink channel to send the message
 *
 * @param curr_tx_pps  tx packets per second
 * @param curr_rx_pps  rx packets per second
 * @param curr_tx_bps  tx bits per second
 * @param curr_rx_bps  rx bits per second
 * @param curr_rx_packet_loss_perc  curr_rx_packet_loss_perc
 * @param unused_0  unused_0
 * @param unused_1  unused_1
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_stats_telemetry_send(mavlink_channel_t chan, int32_t curr_tx_pps, int32_t curr_rx_pps, int32_t curr_tx_bps, int32_t curr_rx_bps, int16_t curr_rx_packet_loss_perc, uint64_t unused_0, uint64_t unused_1)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN];
    _mav_put_uint64_t(buf, 0, unused_0);
    _mav_put_uint64_t(buf, 8, unused_1);
    _mav_put_int32_t(buf, 16, curr_tx_pps);
    _mav_put_int32_t(buf, 20, curr_rx_pps);
    _mav_put_int32_t(buf, 24, curr_tx_bps);
    _mav_put_int32_t(buf, 28, curr_rx_bps);
    _mav_put_int16_t(buf, 32, curr_rx_packet_loss_perc);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY, buf, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_CRC);
#else
    mavlink_openhd_stats_telemetry_t packet;
    packet.unused_0 = unused_0;
    packet.unused_1 = unused_1;
    packet.curr_tx_pps = curr_tx_pps;
    packet.curr_rx_pps = curr_rx_pps;
    packet.curr_tx_bps = curr_tx_bps;
    packet.curr_rx_bps = curr_rx_bps;
    packet.curr_rx_packet_loss_perc = curr_rx_packet_loss_perc;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_CRC);
#endif
}

/**
 * @brief Send a openhd_stats_telemetry message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_stats_telemetry_send_struct(mavlink_channel_t chan, const mavlink_openhd_stats_telemetry_t* openhd_stats_telemetry)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_stats_telemetry_send(chan, openhd_stats_telemetry->curr_tx_pps, openhd_stats_telemetry->curr_rx_pps, openhd_stats_telemetry->curr_tx_bps, openhd_stats_telemetry->curr_rx_bps, openhd_stats_telemetry->curr_rx_packet_loss_perc, openhd_stats_telemetry->unused_0, openhd_stats_telemetry->unused_1);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY, (const char *)openhd_stats_telemetry, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_stats_telemetry_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  int32_t curr_tx_pps, int32_t curr_rx_pps, int32_t curr_tx_bps, int32_t curr_rx_bps, int16_t curr_rx_packet_loss_perc, uint64_t unused_0, uint64_t unused_1)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint64_t(buf, 0, unused_0);
    _mav_put_uint64_t(buf, 8, unused_1);
    _mav_put_int32_t(buf, 16, curr_tx_pps);
    _mav_put_int32_t(buf, 20, curr_rx_pps);
    _mav_put_int32_t(buf, 24, curr_tx_bps);
    _mav_put_int32_t(buf, 28, curr_rx_bps);
    _mav_put_int16_t(buf, 32, curr_rx_packet_loss_perc);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY, buf, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_CRC);
#else
    mavlink_openhd_stats_telemetry_t *packet = (mavlink_openhd_stats_telemetry_t *)msgbuf;
    packet->unused_0 = unused_0;
    packet->unused_1 = unused_1;
    packet->curr_tx_pps = curr_tx_pps;
    packet->curr_rx_pps = curr_rx_pps;
    packet->curr_tx_bps = curr_tx_bps;
    packet->curr_rx_bps = curr_rx_bps;
    packet->curr_rx_packet_loss_perc = curr_rx_packet_loss_perc;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY, (const char *)packet, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_STATS_TELEMETRY UNPACKING


/**
 * @brief Get field curr_tx_pps from openhd_stats_telemetry message
 *
 * @return  tx packets per second
 */
static inline int32_t mavlink_msg_openhd_stats_telemetry_get_curr_tx_pps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  16);
}

/**
 * @brief Get field curr_rx_pps from openhd_stats_telemetry message
 *
 * @return  rx packets per second
 */
static inline int32_t mavlink_msg_openhd_stats_telemetry_get_curr_rx_pps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  20);
}

/**
 * @brief Get field curr_tx_bps from openhd_stats_telemetry message
 *
 * @return  tx bits per second
 */
static inline int32_t mavlink_msg_openhd_stats_telemetry_get_curr_tx_bps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  24);
}

/**
 * @brief Get field curr_rx_bps from openhd_stats_telemetry message
 *
 * @return  rx bits per second
 */
static inline int32_t mavlink_msg_openhd_stats_telemetry_get_curr_rx_bps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  28);
}

/**
 * @brief Get field curr_rx_packet_loss_perc from openhd_stats_telemetry message
 *
 * @return  curr_rx_packet_loss_perc
 */
static inline int16_t mavlink_msg_openhd_stats_telemetry_get_curr_rx_packet_loss_perc(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int16_t(msg,  32);
}

/**
 * @brief Get field unused_0 from openhd_stats_telemetry message
 *
 * @return  unused_0
 */
static inline uint64_t mavlink_msg_openhd_stats_telemetry_get_unused_0(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  0);
}

/**
 * @brief Get field unused_1 from openhd_stats_telemetry message
 *
 * @return  unused_1
 */
static inline uint64_t mavlink_msg_openhd_stats_telemetry_get_unused_1(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  8);
}

/**
 * @brief Decode a openhd_stats_telemetry message into a struct
 *
 * @param msg The message to decode
 * @param openhd_stats_telemetry C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_stats_telemetry_decode(const mavlink_message_t* msg, mavlink_openhd_stats_telemetry_t* openhd_stats_telemetry)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_stats_telemetry->unused_0 = mavlink_msg_openhd_stats_telemetry_get_unused_0(msg);
    openhd_stats_telemetry->unused_1 = mavlink_msg_openhd_stats_telemetry_get_unused_1(msg);
    openhd_stats_telemetry->curr_tx_pps = mavlink_msg_openhd_stats_telemetry_get_curr_tx_pps(msg);
    openhd_stats_telemetry->curr_rx_pps = mavlink_msg_openhd_stats_telemetry_get_curr_rx_pps(msg);
    openhd_stats_telemetry->curr_tx_bps = mavlink_msg_openhd_stats_telemetry_get_curr_tx_bps(msg);
    openhd_stats_telemetry->curr_rx_bps = mavlink_msg_openhd_stats_telemetry_get_curr_rx_bps(msg);
    openhd_stats_telemetry->curr_rx_packet_loss_perc = mavlink_msg_openhd_stats_telemetry_get_curr_rx_packet_loss_perc(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN;
        memset(openhd_stats_telemetry, 0, MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_LEN);
    memcpy(openhd_stats_telemetry, _MAV_PAYLOAD(msg), len);
#endif
}
