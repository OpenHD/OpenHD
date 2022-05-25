#pragma once
// MESSAGE OPENHD_WIFIBROADCAST_STATISTICS PACKING

#define MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS 1211


typedef struct __mavlink_openhd_wifibroadcast_statistics_t {
 uint8_t radio_port; /*<  the unique stream ID this data refers to*/
 uint8_t count_p_all; /*<  count_p_all*/
 uint8_t count_p_bad; /*<  count_p_bad*/
 uint8_t count_p_dec_err; /*<  count_p_dec_err*/
 uint8_t count_p_dec_ok; /*<  count_p_dec_ok*/
 uint8_t count_p_fec_recovered; /*<  count_p_fec_recovered*/
 uint8_t count_p_lost; /*<  count_p_lost*/
} mavlink_openhd_wifibroadcast_statistics_t;

#define MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN 7
#define MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_MIN_LEN 7
#define MAVLINK_MSG_ID_1211_LEN 7
#define MAVLINK_MSG_ID_1211_MIN_LEN 7

#define MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_CRC 4
#define MAVLINK_MSG_ID_1211_CRC 4



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_WIFIBROADCAST_STATISTICS { \
    1211, \
    "OPENHD_WIFIBROADCAST_STATISTICS", \
    7, \
    {  { "radio_port", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_openhd_wifibroadcast_statistics_t, radio_port) }, \
         { "count_p_all", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_openhd_wifibroadcast_statistics_t, count_p_all) }, \
         { "count_p_bad", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_openhd_wifibroadcast_statistics_t, count_p_bad) }, \
         { "count_p_dec_err", NULL, MAVLINK_TYPE_UINT8_T, 0, 3, offsetof(mavlink_openhd_wifibroadcast_statistics_t, count_p_dec_err) }, \
         { "count_p_dec_ok", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_openhd_wifibroadcast_statistics_t, count_p_dec_ok) }, \
         { "count_p_fec_recovered", NULL, MAVLINK_TYPE_UINT8_T, 0, 5, offsetof(mavlink_openhd_wifibroadcast_statistics_t, count_p_fec_recovered) }, \
         { "count_p_lost", NULL, MAVLINK_TYPE_UINT8_T, 0, 6, offsetof(mavlink_openhd_wifibroadcast_statistics_t, count_p_lost) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_WIFIBROADCAST_STATISTICS { \
    "OPENHD_WIFIBROADCAST_STATISTICS", \
    7, \
    {  { "radio_port", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_openhd_wifibroadcast_statistics_t, radio_port) }, \
         { "count_p_all", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_openhd_wifibroadcast_statistics_t, count_p_all) }, \
         { "count_p_bad", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_openhd_wifibroadcast_statistics_t, count_p_bad) }, \
         { "count_p_dec_err", NULL, MAVLINK_TYPE_UINT8_T, 0, 3, offsetof(mavlink_openhd_wifibroadcast_statistics_t, count_p_dec_err) }, \
         { "count_p_dec_ok", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_openhd_wifibroadcast_statistics_t, count_p_dec_ok) }, \
         { "count_p_fec_recovered", NULL, MAVLINK_TYPE_UINT8_T, 0, 5, offsetof(mavlink_openhd_wifibroadcast_statistics_t, count_p_fec_recovered) }, \
         { "count_p_lost", NULL, MAVLINK_TYPE_UINT8_T, 0, 6, offsetof(mavlink_openhd_wifibroadcast_statistics_t, count_p_lost) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_wifibroadcast_statistics message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param radio_port  the unique stream ID this data refers to
 * @param count_p_all  count_p_all
 * @param count_p_bad  count_p_bad
 * @param count_p_dec_err  count_p_dec_err
 * @param count_p_dec_ok  count_p_dec_ok
 * @param count_p_fec_recovered  count_p_fec_recovered
 * @param count_p_lost  count_p_lost
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_wifibroadcast_statistics_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t radio_port, uint8_t count_p_all, uint8_t count_p_bad, uint8_t count_p_dec_err, uint8_t count_p_dec_ok, uint8_t count_p_fec_recovered, uint8_t count_p_lost)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN];
    _mav_put_uint8_t(buf, 0, radio_port);
    _mav_put_uint8_t(buf, 1, count_p_all);
    _mav_put_uint8_t(buf, 2, count_p_bad);
    _mav_put_uint8_t(buf, 3, count_p_dec_err);
    _mav_put_uint8_t(buf, 4, count_p_dec_ok);
    _mav_put_uint8_t(buf, 5, count_p_fec_recovered);
    _mav_put_uint8_t(buf, 6, count_p_lost);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN);
#else
    mavlink_openhd_wifibroadcast_statistics_t packet;
    packet.radio_port = radio_port;
    packet.count_p_all = count_p_all;
    packet.count_p_bad = count_p_bad;
    packet.count_p_dec_err = count_p_dec_err;
    packet.count_p_dec_ok = count_p_dec_ok;
    packet.count_p_fec_recovered = count_p_fec_recovered;
    packet.count_p_lost = count_p_lost;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_CRC);
}

/**
 * @brief Pack a openhd_wifibroadcast_statistics message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param radio_port  the unique stream ID this data refers to
 * @param count_p_all  count_p_all
 * @param count_p_bad  count_p_bad
 * @param count_p_dec_err  count_p_dec_err
 * @param count_p_dec_ok  count_p_dec_ok
 * @param count_p_fec_recovered  count_p_fec_recovered
 * @param count_p_lost  count_p_lost
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_wifibroadcast_statistics_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t radio_port,uint8_t count_p_all,uint8_t count_p_bad,uint8_t count_p_dec_err,uint8_t count_p_dec_ok,uint8_t count_p_fec_recovered,uint8_t count_p_lost)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN];
    _mav_put_uint8_t(buf, 0, radio_port);
    _mav_put_uint8_t(buf, 1, count_p_all);
    _mav_put_uint8_t(buf, 2, count_p_bad);
    _mav_put_uint8_t(buf, 3, count_p_dec_err);
    _mav_put_uint8_t(buf, 4, count_p_dec_ok);
    _mav_put_uint8_t(buf, 5, count_p_fec_recovered);
    _mav_put_uint8_t(buf, 6, count_p_lost);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN);
#else
    mavlink_openhd_wifibroadcast_statistics_t packet;
    packet.radio_port = radio_port;
    packet.count_p_all = count_p_all;
    packet.count_p_bad = count_p_bad;
    packet.count_p_dec_err = count_p_dec_err;
    packet.count_p_dec_ok = count_p_dec_ok;
    packet.count_p_fec_recovered = count_p_fec_recovered;
    packet.count_p_lost = count_p_lost;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_CRC);
}

/**
 * @brief Encode a openhd_wifibroadcast_statistics struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_wifibroadcast_statistics C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_wifibroadcast_statistics_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_wifibroadcast_statistics_t* openhd_wifibroadcast_statistics)
{
    return mavlink_msg_openhd_wifibroadcast_statistics_pack(system_id, component_id, msg, openhd_wifibroadcast_statistics->radio_port, openhd_wifibroadcast_statistics->count_p_all, openhd_wifibroadcast_statistics->count_p_bad, openhd_wifibroadcast_statistics->count_p_dec_err, openhd_wifibroadcast_statistics->count_p_dec_ok, openhd_wifibroadcast_statistics->count_p_fec_recovered, openhd_wifibroadcast_statistics->count_p_lost);
}

/**
 * @brief Encode a openhd_wifibroadcast_statistics struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_wifibroadcast_statistics C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_wifibroadcast_statistics_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_wifibroadcast_statistics_t* openhd_wifibroadcast_statistics)
{
    return mavlink_msg_openhd_wifibroadcast_statistics_pack_chan(system_id, component_id, chan, msg, openhd_wifibroadcast_statistics->radio_port, openhd_wifibroadcast_statistics->count_p_all, openhd_wifibroadcast_statistics->count_p_bad, openhd_wifibroadcast_statistics->count_p_dec_err, openhd_wifibroadcast_statistics->count_p_dec_ok, openhd_wifibroadcast_statistics->count_p_fec_recovered, openhd_wifibroadcast_statistics->count_p_lost);
}

/**
 * @brief Send a openhd_wifibroadcast_statistics message
 * @param chan MAVLink channel to send the message
 *
 * @param radio_port  the unique stream ID this data refers to
 * @param count_p_all  count_p_all
 * @param count_p_bad  count_p_bad
 * @param count_p_dec_err  count_p_dec_err
 * @param count_p_dec_ok  count_p_dec_ok
 * @param count_p_fec_recovered  count_p_fec_recovered
 * @param count_p_lost  count_p_lost
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_wifibroadcast_statistics_send(mavlink_channel_t chan, uint8_t radio_port, uint8_t count_p_all, uint8_t count_p_bad, uint8_t count_p_dec_err, uint8_t count_p_dec_ok, uint8_t count_p_fec_recovered, uint8_t count_p_lost)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN];
    _mav_put_uint8_t(buf, 0, radio_port);
    _mav_put_uint8_t(buf, 1, count_p_all);
    _mav_put_uint8_t(buf, 2, count_p_bad);
    _mav_put_uint8_t(buf, 3, count_p_dec_err);
    _mav_put_uint8_t(buf, 4, count_p_dec_ok);
    _mav_put_uint8_t(buf, 5, count_p_fec_recovered);
    _mav_put_uint8_t(buf, 6, count_p_lost);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS, buf, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_CRC);
#else
    mavlink_openhd_wifibroadcast_statistics_t packet;
    packet.radio_port = radio_port;
    packet.count_p_all = count_p_all;
    packet.count_p_bad = count_p_bad;
    packet.count_p_dec_err = count_p_dec_err;
    packet.count_p_dec_ok = count_p_dec_ok;
    packet.count_p_fec_recovered = count_p_fec_recovered;
    packet.count_p_lost = count_p_lost;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_CRC);
#endif
}

/**
 * @brief Send a openhd_wifibroadcast_statistics message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_wifibroadcast_statistics_send_struct(mavlink_channel_t chan, const mavlink_openhd_wifibroadcast_statistics_t* openhd_wifibroadcast_statistics)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_wifibroadcast_statistics_send(chan, openhd_wifibroadcast_statistics->radio_port, openhd_wifibroadcast_statistics->count_p_all, openhd_wifibroadcast_statistics->count_p_bad, openhd_wifibroadcast_statistics->count_p_dec_err, openhd_wifibroadcast_statistics->count_p_dec_ok, openhd_wifibroadcast_statistics->count_p_fec_recovered, openhd_wifibroadcast_statistics->count_p_lost);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS, (const char *)openhd_wifibroadcast_statistics, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_wifibroadcast_statistics_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t radio_port, uint8_t count_p_all, uint8_t count_p_bad, uint8_t count_p_dec_err, uint8_t count_p_dec_ok, uint8_t count_p_fec_recovered, uint8_t count_p_lost)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint8_t(buf, 0, radio_port);
    _mav_put_uint8_t(buf, 1, count_p_all);
    _mav_put_uint8_t(buf, 2, count_p_bad);
    _mav_put_uint8_t(buf, 3, count_p_dec_err);
    _mav_put_uint8_t(buf, 4, count_p_dec_ok);
    _mav_put_uint8_t(buf, 5, count_p_fec_recovered);
    _mav_put_uint8_t(buf, 6, count_p_lost);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS, buf, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_CRC);
#else
    mavlink_openhd_wifibroadcast_statistics_t *packet = (mavlink_openhd_wifibroadcast_statistics_t *)msgbuf;
    packet->radio_port = radio_port;
    packet->count_p_all = count_p_all;
    packet->count_p_bad = count_p_bad;
    packet->count_p_dec_err = count_p_dec_err;
    packet->count_p_dec_ok = count_p_dec_ok;
    packet->count_p_fec_recovered = count_p_fec_recovered;
    packet->count_p_lost = count_p_lost;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS, (const char *)packet, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_WIFIBROADCAST_STATISTICS UNPACKING


/**
 * @brief Get field radio_port from openhd_wifibroadcast_statistics message
 *
 * @return  the unique stream ID this data refers to
 */
static inline uint8_t mavlink_msg_openhd_wifibroadcast_statistics_get_radio_port(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field count_p_all from openhd_wifibroadcast_statistics message
 *
 * @return  count_p_all
 */
static inline uint8_t mavlink_msg_openhd_wifibroadcast_statistics_get_count_p_all(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Get field count_p_bad from openhd_wifibroadcast_statistics message
 *
 * @return  count_p_bad
 */
static inline uint8_t mavlink_msg_openhd_wifibroadcast_statistics_get_count_p_bad(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  2);
}

/**
 * @brief Get field count_p_dec_err from openhd_wifibroadcast_statistics message
 *
 * @return  count_p_dec_err
 */
static inline uint8_t mavlink_msg_openhd_wifibroadcast_statistics_get_count_p_dec_err(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  3);
}

/**
 * @brief Get field count_p_dec_ok from openhd_wifibroadcast_statistics message
 *
 * @return  count_p_dec_ok
 */
static inline uint8_t mavlink_msg_openhd_wifibroadcast_statistics_get_count_p_dec_ok(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  4);
}

/**
 * @brief Get field count_p_fec_recovered from openhd_wifibroadcast_statistics message
 *
 * @return  count_p_fec_recovered
 */
static inline uint8_t mavlink_msg_openhd_wifibroadcast_statistics_get_count_p_fec_recovered(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  5);
}

/**
 * @brief Get field count_p_lost from openhd_wifibroadcast_statistics message
 *
 * @return  count_p_lost
 */
static inline uint8_t mavlink_msg_openhd_wifibroadcast_statistics_get_count_p_lost(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  6);
}

/**
 * @brief Decode a openhd_wifibroadcast_statistics message into a struct
 *
 * @param msg The message to decode
 * @param openhd_wifibroadcast_statistics C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_wifibroadcast_statistics_decode(const mavlink_message_t* msg, mavlink_openhd_wifibroadcast_statistics_t* openhd_wifibroadcast_statistics)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_wifibroadcast_statistics->radio_port = mavlink_msg_openhd_wifibroadcast_statistics_get_radio_port(msg);
    openhd_wifibroadcast_statistics->count_p_all = mavlink_msg_openhd_wifibroadcast_statistics_get_count_p_all(msg);
    openhd_wifibroadcast_statistics->count_p_bad = mavlink_msg_openhd_wifibroadcast_statistics_get_count_p_bad(msg);
    openhd_wifibroadcast_statistics->count_p_dec_err = mavlink_msg_openhd_wifibroadcast_statistics_get_count_p_dec_err(msg);
    openhd_wifibroadcast_statistics->count_p_dec_ok = mavlink_msg_openhd_wifibroadcast_statistics_get_count_p_dec_ok(msg);
    openhd_wifibroadcast_statistics->count_p_fec_recovered = mavlink_msg_openhd_wifibroadcast_statistics_get_count_p_fec_recovered(msg);
    openhd_wifibroadcast_statistics->count_p_lost = mavlink_msg_openhd_wifibroadcast_statistics_get_count_p_lost(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN;
        memset(openhd_wifibroadcast_statistics, 0, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_LEN);
    memcpy(openhd_wifibroadcast_statistics, _MAV_PAYLOAD(msg), len);
#endif
}
