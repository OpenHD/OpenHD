#pragma once
// MESSAGE OPENHD_WIFIBROADCAST_WIFI_CARD PACKING

#define MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD 1211


typedef struct __mavlink_openhd_wifibroadcast_wifi_card_t {
 uint64_t count_p_received; /*<  All received (incoming) packets, not suported by all cards*/
 uint64_t count_p_injected; /*<  All injected (outgoing) packets, not suported by all cards*/
 uint64_t dummy0; /*<  dummy0*/
 uint64_t dummy1; /*<  dummy1*/
 uint8_t card_index; /*<  A system / component might have more than one card for diversity*/
 int8_t rx_rssi; /*<  rx_rssi*/
} mavlink_openhd_wifibroadcast_wifi_card_t;

#define MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN 34
#define MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_MIN_LEN 34
#define MAVLINK_MSG_ID_1211_LEN 34
#define MAVLINK_MSG_ID_1211_MIN_LEN 34

#define MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_CRC 185
#define MAVLINK_MSG_ID_1211_CRC 185



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_WIFIBROADCAST_WIFI_CARD { \
    1211, \
    "OPENHD_WIFIBROADCAST_WIFI_CARD", \
    6, \
    {  { "card_index", NULL, MAVLINK_TYPE_UINT8_T, 0, 32, offsetof(mavlink_openhd_wifibroadcast_wifi_card_t, card_index) }, \
         { "rx_rssi", NULL, MAVLINK_TYPE_INT8_T, 0, 33, offsetof(mavlink_openhd_wifibroadcast_wifi_card_t, rx_rssi) }, \
         { "count_p_received", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_wifibroadcast_wifi_card_t, count_p_received) }, \
         { "count_p_injected", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_openhd_wifibroadcast_wifi_card_t, count_p_injected) }, \
         { "dummy0", NULL, MAVLINK_TYPE_UINT64_T, 0, 16, offsetof(mavlink_openhd_wifibroadcast_wifi_card_t, dummy0) }, \
         { "dummy1", NULL, MAVLINK_TYPE_UINT64_T, 0, 24, offsetof(mavlink_openhd_wifibroadcast_wifi_card_t, dummy1) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_WIFIBROADCAST_WIFI_CARD { \
    "OPENHD_WIFIBROADCAST_WIFI_CARD", \
    6, \
    {  { "card_index", NULL, MAVLINK_TYPE_UINT8_T, 0, 32, offsetof(mavlink_openhd_wifibroadcast_wifi_card_t, card_index) }, \
         { "rx_rssi", NULL, MAVLINK_TYPE_INT8_T, 0, 33, offsetof(mavlink_openhd_wifibroadcast_wifi_card_t, rx_rssi) }, \
         { "count_p_received", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_wifibroadcast_wifi_card_t, count_p_received) }, \
         { "count_p_injected", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_openhd_wifibroadcast_wifi_card_t, count_p_injected) }, \
         { "dummy0", NULL, MAVLINK_TYPE_UINT64_T, 0, 16, offsetof(mavlink_openhd_wifibroadcast_wifi_card_t, dummy0) }, \
         { "dummy1", NULL, MAVLINK_TYPE_UINT64_T, 0, 24, offsetof(mavlink_openhd_wifibroadcast_wifi_card_t, dummy1) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_wifibroadcast_wifi_card message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param card_index  A system / component might have more than one card for diversity
 * @param rx_rssi  rx_rssi
 * @param count_p_received  All received (incoming) packets, not suported by all cards
 * @param count_p_injected  All injected (outgoing) packets, not suported by all cards
 * @param dummy0  dummy0
 * @param dummy1  dummy1
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_wifibroadcast_wifi_card_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t card_index, int8_t rx_rssi, uint64_t count_p_received, uint64_t count_p_injected, uint64_t dummy0, uint64_t dummy1)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN];
    _mav_put_uint64_t(buf, 0, count_p_received);
    _mav_put_uint64_t(buf, 8, count_p_injected);
    _mav_put_uint64_t(buf, 16, dummy0);
    _mav_put_uint64_t(buf, 24, dummy1);
    _mav_put_uint8_t(buf, 32, card_index);
    _mav_put_int8_t(buf, 33, rx_rssi);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN);
#else
    mavlink_openhd_wifibroadcast_wifi_card_t packet;
    packet.count_p_received = count_p_received;
    packet.count_p_injected = count_p_injected;
    packet.dummy0 = dummy0;
    packet.dummy1 = dummy1;
    packet.card_index = card_index;
    packet.rx_rssi = rx_rssi;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_CRC);
}

/**
 * @brief Pack a openhd_wifibroadcast_wifi_card message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param card_index  A system / component might have more than one card for diversity
 * @param rx_rssi  rx_rssi
 * @param count_p_received  All received (incoming) packets, not suported by all cards
 * @param count_p_injected  All injected (outgoing) packets, not suported by all cards
 * @param dummy0  dummy0
 * @param dummy1  dummy1
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_wifibroadcast_wifi_card_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t card_index,int8_t rx_rssi,uint64_t count_p_received,uint64_t count_p_injected,uint64_t dummy0,uint64_t dummy1)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN];
    _mav_put_uint64_t(buf, 0, count_p_received);
    _mav_put_uint64_t(buf, 8, count_p_injected);
    _mav_put_uint64_t(buf, 16, dummy0);
    _mav_put_uint64_t(buf, 24, dummy1);
    _mav_put_uint8_t(buf, 32, card_index);
    _mav_put_int8_t(buf, 33, rx_rssi);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN);
#else
    mavlink_openhd_wifibroadcast_wifi_card_t packet;
    packet.count_p_received = count_p_received;
    packet.count_p_injected = count_p_injected;
    packet.dummy0 = dummy0;
    packet.dummy1 = dummy1;
    packet.card_index = card_index;
    packet.rx_rssi = rx_rssi;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_CRC);
}

/**
 * @brief Encode a openhd_wifibroadcast_wifi_card struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_wifibroadcast_wifi_card C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_wifibroadcast_wifi_card_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_wifibroadcast_wifi_card_t* openhd_wifibroadcast_wifi_card)
{
    return mavlink_msg_openhd_wifibroadcast_wifi_card_pack(system_id, component_id, msg, openhd_wifibroadcast_wifi_card->card_index, openhd_wifibroadcast_wifi_card->rx_rssi, openhd_wifibroadcast_wifi_card->count_p_received, openhd_wifibroadcast_wifi_card->count_p_injected, openhd_wifibroadcast_wifi_card->dummy0, openhd_wifibroadcast_wifi_card->dummy1);
}

/**
 * @brief Encode a openhd_wifibroadcast_wifi_card struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_wifibroadcast_wifi_card C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_wifibroadcast_wifi_card_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_wifibroadcast_wifi_card_t* openhd_wifibroadcast_wifi_card)
{
    return mavlink_msg_openhd_wifibroadcast_wifi_card_pack_chan(system_id, component_id, chan, msg, openhd_wifibroadcast_wifi_card->card_index, openhd_wifibroadcast_wifi_card->rx_rssi, openhd_wifibroadcast_wifi_card->count_p_received, openhd_wifibroadcast_wifi_card->count_p_injected, openhd_wifibroadcast_wifi_card->dummy0, openhd_wifibroadcast_wifi_card->dummy1);
}

/**
 * @brief Send a openhd_wifibroadcast_wifi_card message
 * @param chan MAVLink channel to send the message
 *
 * @param card_index  A system / component might have more than one card for diversity
 * @param rx_rssi  rx_rssi
 * @param count_p_received  All received (incoming) packets, not suported by all cards
 * @param count_p_injected  All injected (outgoing) packets, not suported by all cards
 * @param dummy0  dummy0
 * @param dummy1  dummy1
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_wifibroadcast_wifi_card_send(mavlink_channel_t chan, uint8_t card_index, int8_t rx_rssi, uint64_t count_p_received, uint64_t count_p_injected, uint64_t dummy0, uint64_t dummy1)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN];
    _mav_put_uint64_t(buf, 0, count_p_received);
    _mav_put_uint64_t(buf, 8, count_p_injected);
    _mav_put_uint64_t(buf, 16, dummy0);
    _mav_put_uint64_t(buf, 24, dummy1);
    _mav_put_uint8_t(buf, 32, card_index);
    _mav_put_int8_t(buf, 33, rx_rssi);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD, buf, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_CRC);
#else
    mavlink_openhd_wifibroadcast_wifi_card_t packet;
    packet.count_p_received = count_p_received;
    packet.count_p_injected = count_p_injected;
    packet.dummy0 = dummy0;
    packet.dummy1 = dummy1;
    packet.card_index = card_index;
    packet.rx_rssi = rx_rssi;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_CRC);
#endif
}

/**
 * @brief Send a openhd_wifibroadcast_wifi_card message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_wifibroadcast_wifi_card_send_struct(mavlink_channel_t chan, const mavlink_openhd_wifibroadcast_wifi_card_t* openhd_wifibroadcast_wifi_card)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_wifibroadcast_wifi_card_send(chan, openhd_wifibroadcast_wifi_card->card_index, openhd_wifibroadcast_wifi_card->rx_rssi, openhd_wifibroadcast_wifi_card->count_p_received, openhd_wifibroadcast_wifi_card->count_p_injected, openhd_wifibroadcast_wifi_card->dummy0, openhd_wifibroadcast_wifi_card->dummy1);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD, (const char *)openhd_wifibroadcast_wifi_card, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_wifibroadcast_wifi_card_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t card_index, int8_t rx_rssi, uint64_t count_p_received, uint64_t count_p_injected, uint64_t dummy0, uint64_t dummy1)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint64_t(buf, 0, count_p_received);
    _mav_put_uint64_t(buf, 8, count_p_injected);
    _mav_put_uint64_t(buf, 16, dummy0);
    _mav_put_uint64_t(buf, 24, dummy1);
    _mav_put_uint8_t(buf, 32, card_index);
    _mav_put_int8_t(buf, 33, rx_rssi);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD, buf, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_CRC);
#else
    mavlink_openhd_wifibroadcast_wifi_card_t *packet = (mavlink_openhd_wifibroadcast_wifi_card_t *)msgbuf;
    packet->count_p_received = count_p_received;
    packet->count_p_injected = count_p_injected;
    packet->dummy0 = dummy0;
    packet->dummy1 = dummy1;
    packet->card_index = card_index;
    packet->rx_rssi = rx_rssi;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD, (const char *)packet, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_WIFIBROADCAST_WIFI_CARD UNPACKING


/**
 * @brief Get field card_index from openhd_wifibroadcast_wifi_card message
 *
 * @return  A system / component might have more than one card for diversity
 */
static inline uint8_t mavlink_msg_openhd_wifibroadcast_wifi_card_get_card_index(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  32);
}

/**
 * @brief Get field rx_rssi from openhd_wifibroadcast_wifi_card message
 *
 * @return  rx_rssi
 */
static inline int8_t mavlink_msg_openhd_wifibroadcast_wifi_card_get_rx_rssi(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int8_t(msg,  33);
}

/**
 * @brief Get field count_p_received from openhd_wifibroadcast_wifi_card message
 *
 * @return  All received (incoming) packets, not suported by all cards
 */
static inline uint64_t mavlink_msg_openhd_wifibroadcast_wifi_card_get_count_p_received(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  0);
}

/**
 * @brief Get field count_p_injected from openhd_wifibroadcast_wifi_card message
 *
 * @return  All injected (outgoing) packets, not suported by all cards
 */
static inline uint64_t mavlink_msg_openhd_wifibroadcast_wifi_card_get_count_p_injected(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  8);
}

/**
 * @brief Get field dummy0 from openhd_wifibroadcast_wifi_card message
 *
 * @return  dummy0
 */
static inline uint64_t mavlink_msg_openhd_wifibroadcast_wifi_card_get_dummy0(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  16);
}

/**
 * @brief Get field dummy1 from openhd_wifibroadcast_wifi_card message
 *
 * @return  dummy1
 */
static inline uint64_t mavlink_msg_openhd_wifibroadcast_wifi_card_get_dummy1(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  24);
}

/**
 * @brief Decode a openhd_wifibroadcast_wifi_card message into a struct
 *
 * @param msg The message to decode
 * @param openhd_wifibroadcast_wifi_card C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_wifibroadcast_wifi_card_decode(const mavlink_message_t* msg, mavlink_openhd_wifibroadcast_wifi_card_t* openhd_wifibroadcast_wifi_card)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_wifibroadcast_wifi_card->count_p_received = mavlink_msg_openhd_wifibroadcast_wifi_card_get_count_p_received(msg);
    openhd_wifibroadcast_wifi_card->count_p_injected = mavlink_msg_openhd_wifibroadcast_wifi_card_get_count_p_injected(msg);
    openhd_wifibroadcast_wifi_card->dummy0 = mavlink_msg_openhd_wifibroadcast_wifi_card_get_dummy0(msg);
    openhd_wifibroadcast_wifi_card->dummy1 = mavlink_msg_openhd_wifibroadcast_wifi_card_get_dummy1(msg);
    openhd_wifibroadcast_wifi_card->card_index = mavlink_msg_openhd_wifibroadcast_wifi_card_get_card_index(msg);
    openhd_wifibroadcast_wifi_card->rx_rssi = mavlink_msg_openhd_wifibroadcast_wifi_card_get_rx_rssi(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN;
        memset(openhd_wifibroadcast_wifi_card, 0, MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_LEN);
    memcpy(openhd_wifibroadcast_wifi_card, _MAV_PAYLOAD(msg), len);
#endif
}
