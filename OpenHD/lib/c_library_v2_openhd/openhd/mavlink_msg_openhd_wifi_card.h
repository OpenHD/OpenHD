#pragma once
// MESSAGE OPENHD_WIFI_CARD PACKING

#define MAVLINK_MSG_ID_OPENHD_WIFI_CARD 1212


typedef struct __mavlink_openhd_wifi_card_t {
 int32_t signal_millidBm; /*<  Current rx signal strength, in milli dBm (1000== 10.0dBm)*/
 uint32_t count_p_received; /*<  All received (incoming) packets, not suported by all cards*/
 uint32_t count_p_injected; /*<  All injected (outgoing) packets, not suported by all cards*/
 uint32_t count_p_tx_err; /*<  count_p_tx_err*/
 uint32_t count_p_rx_err; /*<  count_p_rx_err*/
 uint8_t card_index; /*<  A system / component might have more than one card for diversity*/
} mavlink_openhd_wifi_card_t;

#define MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN 21
#define MAVLINK_MSG_ID_OPENHD_WIFI_CARD_MIN_LEN 21
#define MAVLINK_MSG_ID_1212_LEN 21
#define MAVLINK_MSG_ID_1212_MIN_LEN 21

#define MAVLINK_MSG_ID_OPENHD_WIFI_CARD_CRC 224
#define MAVLINK_MSG_ID_1212_CRC 224



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_WIFI_CARD { \
    1212, \
    "OPENHD_WIFI_CARD", \
    6, \
    {  { "card_index", NULL, MAVLINK_TYPE_UINT8_T, 0, 20, offsetof(mavlink_openhd_wifi_card_t, card_index) }, \
         { "signal_millidBm", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_openhd_wifi_card_t, signal_millidBm) }, \
         { "count_p_received", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_openhd_wifi_card_t, count_p_received) }, \
         { "count_p_injected", NULL, MAVLINK_TYPE_UINT32_T, 0, 8, offsetof(mavlink_openhd_wifi_card_t, count_p_injected) }, \
         { "count_p_tx_err", NULL, MAVLINK_TYPE_UINT32_T, 0, 12, offsetof(mavlink_openhd_wifi_card_t, count_p_tx_err) }, \
         { "count_p_rx_err", NULL, MAVLINK_TYPE_UINT32_T, 0, 16, offsetof(mavlink_openhd_wifi_card_t, count_p_rx_err) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_WIFI_CARD { \
    "OPENHD_WIFI_CARD", \
    6, \
    {  { "card_index", NULL, MAVLINK_TYPE_UINT8_T, 0, 20, offsetof(mavlink_openhd_wifi_card_t, card_index) }, \
         { "signal_millidBm", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_openhd_wifi_card_t, signal_millidBm) }, \
         { "count_p_received", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_openhd_wifi_card_t, count_p_received) }, \
         { "count_p_injected", NULL, MAVLINK_TYPE_UINT32_T, 0, 8, offsetof(mavlink_openhd_wifi_card_t, count_p_injected) }, \
         { "count_p_tx_err", NULL, MAVLINK_TYPE_UINT32_T, 0, 12, offsetof(mavlink_openhd_wifi_card_t, count_p_tx_err) }, \
         { "count_p_rx_err", NULL, MAVLINK_TYPE_UINT32_T, 0, 16, offsetof(mavlink_openhd_wifi_card_t, count_p_rx_err) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_wifi_card message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param card_index  A system / component might have more than one card for diversity
 * @param signal_millidBm  Current rx signal strength, in milli dBm (1000== 10.0dBm)
 * @param count_p_received  All received (incoming) packets, not suported by all cards
 * @param count_p_injected  All injected (outgoing) packets, not suported by all cards
 * @param count_p_tx_err  count_p_tx_err
 * @param count_p_rx_err  count_p_rx_err
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_wifi_card_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t card_index, int32_t signal_millidBm, uint32_t count_p_received, uint32_t count_p_injected, uint32_t count_p_tx_err, uint32_t count_p_rx_err)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN];
    _mav_put_int32_t(buf, 0, signal_millidBm);
    _mav_put_uint32_t(buf, 4, count_p_received);
    _mav_put_uint32_t(buf, 8, count_p_injected);
    _mav_put_uint32_t(buf, 12, count_p_tx_err);
    _mav_put_uint32_t(buf, 16, count_p_rx_err);
    _mav_put_uint8_t(buf, 20, card_index);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN);
#else
    mavlink_openhd_wifi_card_t packet;
    packet.signal_millidBm = signal_millidBm;
    packet.count_p_received = count_p_received;
    packet.count_p_injected = count_p_injected;
    packet.count_p_tx_err = count_p_tx_err;
    packet.count_p_rx_err = count_p_rx_err;
    packet.card_index = card_index;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_WIFI_CARD;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_CRC);
}

/**
 * @brief Pack a openhd_wifi_card message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param card_index  A system / component might have more than one card for diversity
 * @param signal_millidBm  Current rx signal strength, in milli dBm (1000== 10.0dBm)
 * @param count_p_received  All received (incoming) packets, not suported by all cards
 * @param count_p_injected  All injected (outgoing) packets, not suported by all cards
 * @param count_p_tx_err  count_p_tx_err
 * @param count_p_rx_err  count_p_rx_err
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_wifi_card_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t card_index,int32_t signal_millidBm,uint32_t count_p_received,uint32_t count_p_injected,uint32_t count_p_tx_err,uint32_t count_p_rx_err)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN];
    _mav_put_int32_t(buf, 0, signal_millidBm);
    _mav_put_uint32_t(buf, 4, count_p_received);
    _mav_put_uint32_t(buf, 8, count_p_injected);
    _mav_put_uint32_t(buf, 12, count_p_tx_err);
    _mav_put_uint32_t(buf, 16, count_p_rx_err);
    _mav_put_uint8_t(buf, 20, card_index);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN);
#else
    mavlink_openhd_wifi_card_t packet;
    packet.signal_millidBm = signal_millidBm;
    packet.count_p_received = count_p_received;
    packet.count_p_injected = count_p_injected;
    packet.count_p_tx_err = count_p_tx_err;
    packet.count_p_rx_err = count_p_rx_err;
    packet.card_index = card_index;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_WIFI_CARD;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_CRC);
}

/**
 * @brief Encode a openhd_wifi_card struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_wifi_card C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_wifi_card_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_wifi_card_t* openhd_wifi_card)
{
    return mavlink_msg_openhd_wifi_card_pack(system_id, component_id, msg, openhd_wifi_card->card_index, openhd_wifi_card->signal_millidBm, openhd_wifi_card->count_p_received, openhd_wifi_card->count_p_injected, openhd_wifi_card->count_p_tx_err, openhd_wifi_card->count_p_rx_err);
}

/**
 * @brief Encode a openhd_wifi_card struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_wifi_card C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_wifi_card_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_wifi_card_t* openhd_wifi_card)
{
    return mavlink_msg_openhd_wifi_card_pack_chan(system_id, component_id, chan, msg, openhd_wifi_card->card_index, openhd_wifi_card->signal_millidBm, openhd_wifi_card->count_p_received, openhd_wifi_card->count_p_injected, openhd_wifi_card->count_p_tx_err, openhd_wifi_card->count_p_rx_err);
}

/**
 * @brief Send a openhd_wifi_card message
 * @param chan MAVLink channel to send the message
 *
 * @param card_index  A system / component might have more than one card for diversity
 * @param signal_millidBm  Current rx signal strength, in milli dBm (1000== 10.0dBm)
 * @param count_p_received  All received (incoming) packets, not suported by all cards
 * @param count_p_injected  All injected (outgoing) packets, not suported by all cards
 * @param count_p_tx_err  count_p_tx_err
 * @param count_p_rx_err  count_p_rx_err
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_wifi_card_send(mavlink_channel_t chan, uint8_t card_index, int32_t signal_millidBm, uint32_t count_p_received, uint32_t count_p_injected, uint32_t count_p_tx_err, uint32_t count_p_rx_err)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN];
    _mav_put_int32_t(buf, 0, signal_millidBm);
    _mav_put_uint32_t(buf, 4, count_p_received);
    _mav_put_uint32_t(buf, 8, count_p_injected);
    _mav_put_uint32_t(buf, 12, count_p_tx_err);
    _mav_put_uint32_t(buf, 16, count_p_rx_err);
    _mav_put_uint8_t(buf, 20, card_index);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFI_CARD, buf, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_CRC);
#else
    mavlink_openhd_wifi_card_t packet;
    packet.signal_millidBm = signal_millidBm;
    packet.count_p_received = count_p_received;
    packet.count_p_injected = count_p_injected;
    packet.count_p_tx_err = count_p_tx_err;
    packet.count_p_rx_err = count_p_rx_err;
    packet.card_index = card_index;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFI_CARD, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_CRC);
#endif
}

/**
 * @brief Send a openhd_wifi_card message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_wifi_card_send_struct(mavlink_channel_t chan, const mavlink_openhd_wifi_card_t* openhd_wifi_card)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_wifi_card_send(chan, openhd_wifi_card->card_index, openhd_wifi_card->signal_millidBm, openhd_wifi_card->count_p_received, openhd_wifi_card->count_p_injected, openhd_wifi_card->count_p_tx_err, openhd_wifi_card->count_p_rx_err);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFI_CARD, (const char *)openhd_wifi_card, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_wifi_card_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t card_index, int32_t signal_millidBm, uint32_t count_p_received, uint32_t count_p_injected, uint32_t count_p_tx_err, uint32_t count_p_rx_err)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_int32_t(buf, 0, signal_millidBm);
    _mav_put_uint32_t(buf, 4, count_p_received);
    _mav_put_uint32_t(buf, 8, count_p_injected);
    _mav_put_uint32_t(buf, 12, count_p_tx_err);
    _mav_put_uint32_t(buf, 16, count_p_rx_err);
    _mav_put_uint8_t(buf, 20, card_index);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFI_CARD, buf, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_CRC);
#else
    mavlink_openhd_wifi_card_t *packet = (mavlink_openhd_wifi_card_t *)msgbuf;
    packet->signal_millidBm = signal_millidBm;
    packet->count_p_received = count_p_received;
    packet->count_p_injected = count_p_injected;
    packet->count_p_tx_err = count_p_tx_err;
    packet->count_p_rx_err = count_p_rx_err;
    packet->card_index = card_index;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFI_CARD, (const char *)packet, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_WIFI_CARD UNPACKING


/**
 * @brief Get field card_index from openhd_wifi_card message
 *
 * @return  A system / component might have more than one card for diversity
 */
static inline uint8_t mavlink_msg_openhd_wifi_card_get_card_index(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  20);
}

/**
 * @brief Get field signal_millidBm from openhd_wifi_card message
 *
 * @return  Current rx signal strength, in milli dBm (1000== 10.0dBm)
 */
static inline int32_t mavlink_msg_openhd_wifi_card_get_signal_millidBm(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  0);
}

/**
 * @brief Get field count_p_received from openhd_wifi_card message
 *
 * @return  All received (incoming) packets, not suported by all cards
 */
static inline uint32_t mavlink_msg_openhd_wifi_card_get_count_p_received(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  4);
}

/**
 * @brief Get field count_p_injected from openhd_wifi_card message
 *
 * @return  All injected (outgoing) packets, not suported by all cards
 */
static inline uint32_t mavlink_msg_openhd_wifi_card_get_count_p_injected(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  8);
}

/**
 * @brief Get field count_p_tx_err from openhd_wifi_card message
 *
 * @return  count_p_tx_err
 */
static inline uint32_t mavlink_msg_openhd_wifi_card_get_count_p_tx_err(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  12);
}

/**
 * @brief Get field count_p_rx_err from openhd_wifi_card message
 *
 * @return  count_p_rx_err
 */
static inline uint32_t mavlink_msg_openhd_wifi_card_get_count_p_rx_err(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  16);
}

/**
 * @brief Decode a openhd_wifi_card message into a struct
 *
 * @param msg The message to decode
 * @param openhd_wifi_card C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_wifi_card_decode(const mavlink_message_t* msg, mavlink_openhd_wifi_card_t* openhd_wifi_card)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_wifi_card->signal_millidBm = mavlink_msg_openhd_wifi_card_get_signal_millidBm(msg);
    openhd_wifi_card->count_p_received = mavlink_msg_openhd_wifi_card_get_count_p_received(msg);
    openhd_wifi_card->count_p_injected = mavlink_msg_openhd_wifi_card_get_count_p_injected(msg);
    openhd_wifi_card->count_p_tx_err = mavlink_msg_openhd_wifi_card_get_count_p_tx_err(msg);
    openhd_wifi_card->count_p_rx_err = mavlink_msg_openhd_wifi_card_get_count_p_rx_err(msg);
    openhd_wifi_card->card_index = mavlink_msg_openhd_wifi_card_get_card_index(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN;
        memset(openhd_wifi_card, 0, MAVLINK_MSG_ID_OPENHD_WIFI_CARD_LEN);
    memcpy(openhd_wifi_card, _MAV_PAYLOAD(msg), len);
#endif
}
