#pragma once
// MESSAGE OPENHD_WIFI_STATUS PACKING

#define MAVLINK_MSG_ID_OPENHD_WIFI_STATUS 1212

MAVPACKED(
typedef struct __mavlink_openhd_wifi_status_t {
 uint32_t received_packet_cnt[4]; /*<  received_packet_cnt*/
 uint8_t target_system; /*<  system id of the requesting system*/
 uint8_t target_component; /*<  component id of the requesting component*/
 uint8_t wifi_adapter_cnt; /*<  wifi_adapter_cnt*/
 int8_t current_signal_dbm[4]; /*<  current_signal_dbm*/
 uint8_t wifi_card_type[4]; /*<  wifi_card_type*/
}) mavlink_openhd_wifi_status_t;

#define MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN 27
#define MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_MIN_LEN 27
#define MAVLINK_MSG_ID_1212_LEN 27
#define MAVLINK_MSG_ID_1212_MIN_LEN 27

#define MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_CRC 124
#define MAVLINK_MSG_ID_1212_CRC 124

#define MAVLINK_MSG_OPENHD_WIFI_STATUS_FIELD_RECEIVED_PACKET_CNT_LEN 4
#define MAVLINK_MSG_OPENHD_WIFI_STATUS_FIELD_CURRENT_SIGNAL_DBM_LEN 4
#define MAVLINK_MSG_OPENHD_WIFI_STATUS_FIELD_WIFI_CARD_TYPE_LEN 4

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_WIFI_STATUS { \
    1212, \
    "OPENHD_WIFI_STATUS", \
    6, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 16, offsetof(mavlink_openhd_wifi_status_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 17, offsetof(mavlink_openhd_wifi_status_t, target_component) }, \
         { "wifi_adapter_cnt", NULL, MAVLINK_TYPE_UINT8_T, 0, 18, offsetof(mavlink_openhd_wifi_status_t, wifi_adapter_cnt) }, \
         { "current_signal_dbm", NULL, MAVLINK_TYPE_INT8_T, 4, 19, offsetof(mavlink_openhd_wifi_status_t, current_signal_dbm) }, \
         { "wifi_card_type", NULL, MAVLINK_TYPE_UINT8_T, 4, 23, offsetof(mavlink_openhd_wifi_status_t, wifi_card_type) }, \
         { "received_packet_cnt", NULL, MAVLINK_TYPE_UINT32_T, 4, 0, offsetof(mavlink_openhd_wifi_status_t, received_packet_cnt) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_WIFI_STATUS { \
    "OPENHD_WIFI_STATUS", \
    6, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 16, offsetof(mavlink_openhd_wifi_status_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 17, offsetof(mavlink_openhd_wifi_status_t, target_component) }, \
         { "wifi_adapter_cnt", NULL, MAVLINK_TYPE_UINT8_T, 0, 18, offsetof(mavlink_openhd_wifi_status_t, wifi_adapter_cnt) }, \
         { "current_signal_dbm", NULL, MAVLINK_TYPE_INT8_T, 4, 19, offsetof(mavlink_openhd_wifi_status_t, current_signal_dbm) }, \
         { "wifi_card_type", NULL, MAVLINK_TYPE_UINT8_T, 4, 23, offsetof(mavlink_openhd_wifi_status_t, wifi_card_type) }, \
         { "received_packet_cnt", NULL, MAVLINK_TYPE_UINT32_T, 4, 0, offsetof(mavlink_openhd_wifi_status_t, received_packet_cnt) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_wifi_status message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param wifi_adapter_cnt  wifi_adapter_cnt
 * @param current_signal_dbm  current_signal_dbm
 * @param wifi_card_type  wifi_card_type
 * @param received_packet_cnt  received_packet_cnt
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_wifi_status_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t target_system, uint8_t target_component, uint8_t wifi_adapter_cnt, const int8_t *current_signal_dbm, const uint8_t *wifi_card_type, const uint32_t *received_packet_cnt)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN];
    _mav_put_uint8_t(buf, 16, target_system);
    _mav_put_uint8_t(buf, 17, target_component);
    _mav_put_uint8_t(buf, 18, wifi_adapter_cnt);
    _mav_put_uint32_t_array(buf, 0, received_packet_cnt, 4);
    _mav_put_int8_t_array(buf, 19, current_signal_dbm, 4);
    _mav_put_uint8_t_array(buf, 23, wifi_card_type, 4);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN);
#else
    mavlink_openhd_wifi_status_t packet;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.wifi_adapter_cnt = wifi_adapter_cnt;
    mav_array_memcpy(packet.received_packet_cnt, received_packet_cnt, sizeof(uint32_t)*4);
    mav_array_memcpy(packet.current_signal_dbm, current_signal_dbm, sizeof(int8_t)*4);
    mav_array_memcpy(packet.wifi_card_type, wifi_card_type, sizeof(uint8_t)*4);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_WIFI_STATUS;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_CRC);
}

/**
 * @brief Pack a openhd_wifi_status message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param wifi_adapter_cnt  wifi_adapter_cnt
 * @param current_signal_dbm  current_signal_dbm
 * @param wifi_card_type  wifi_card_type
 * @param received_packet_cnt  received_packet_cnt
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_wifi_status_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t target_system,uint8_t target_component,uint8_t wifi_adapter_cnt,const int8_t *current_signal_dbm,const uint8_t *wifi_card_type,const uint32_t *received_packet_cnt)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN];
    _mav_put_uint8_t(buf, 16, target_system);
    _mav_put_uint8_t(buf, 17, target_component);
    _mav_put_uint8_t(buf, 18, wifi_adapter_cnt);
    _mav_put_uint32_t_array(buf, 0, received_packet_cnt, 4);
    _mav_put_int8_t_array(buf, 19, current_signal_dbm, 4);
    _mav_put_uint8_t_array(buf, 23, wifi_card_type, 4);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN);
#else
    mavlink_openhd_wifi_status_t packet;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.wifi_adapter_cnt = wifi_adapter_cnt;
    mav_array_memcpy(packet.received_packet_cnt, received_packet_cnt, sizeof(uint32_t)*4);
    mav_array_memcpy(packet.current_signal_dbm, current_signal_dbm, sizeof(int8_t)*4);
    mav_array_memcpy(packet.wifi_card_type, wifi_card_type, sizeof(uint8_t)*4);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_WIFI_STATUS;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_CRC);
}

/**
 * @brief Encode a openhd_wifi_status struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_wifi_status C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_wifi_status_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_wifi_status_t* openhd_wifi_status)
{
    return mavlink_msg_openhd_wifi_status_pack(system_id, component_id, msg, openhd_wifi_status->target_system, openhd_wifi_status->target_component, openhd_wifi_status->wifi_adapter_cnt, openhd_wifi_status->current_signal_dbm, openhd_wifi_status->wifi_card_type, openhd_wifi_status->received_packet_cnt);
}

/**
 * @brief Encode a openhd_wifi_status struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_wifi_status C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_wifi_status_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_wifi_status_t* openhd_wifi_status)
{
    return mavlink_msg_openhd_wifi_status_pack_chan(system_id, component_id, chan, msg, openhd_wifi_status->target_system, openhd_wifi_status->target_component, openhd_wifi_status->wifi_adapter_cnt, openhd_wifi_status->current_signal_dbm, openhd_wifi_status->wifi_card_type, openhd_wifi_status->received_packet_cnt);
}

/**
 * @brief Send a openhd_wifi_status message
 * @param chan MAVLink channel to send the message
 *
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param wifi_adapter_cnt  wifi_adapter_cnt
 * @param current_signal_dbm  current_signal_dbm
 * @param wifi_card_type  wifi_card_type
 * @param received_packet_cnt  received_packet_cnt
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_wifi_status_send(mavlink_channel_t chan, uint8_t target_system, uint8_t target_component, uint8_t wifi_adapter_cnt, const int8_t *current_signal_dbm, const uint8_t *wifi_card_type, const uint32_t *received_packet_cnt)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN];
    _mav_put_uint8_t(buf, 16, target_system);
    _mav_put_uint8_t(buf, 17, target_component);
    _mav_put_uint8_t(buf, 18, wifi_adapter_cnt);
    _mav_put_uint32_t_array(buf, 0, received_packet_cnt, 4);
    _mav_put_int8_t_array(buf, 19, current_signal_dbm, 4);
    _mav_put_uint8_t_array(buf, 23, wifi_card_type, 4);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS, buf, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_CRC);
#else
    mavlink_openhd_wifi_status_t packet;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.wifi_adapter_cnt = wifi_adapter_cnt;
    mav_array_memcpy(packet.received_packet_cnt, received_packet_cnt, sizeof(uint32_t)*4);
    mav_array_memcpy(packet.current_signal_dbm, current_signal_dbm, sizeof(int8_t)*4);
    mav_array_memcpy(packet.wifi_card_type, wifi_card_type, sizeof(uint8_t)*4);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_CRC);
#endif
}

/**
 * @brief Send a openhd_wifi_status message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_wifi_status_send_struct(mavlink_channel_t chan, const mavlink_openhd_wifi_status_t* openhd_wifi_status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_wifi_status_send(chan, openhd_wifi_status->target_system, openhd_wifi_status->target_component, openhd_wifi_status->wifi_adapter_cnt, openhd_wifi_status->current_signal_dbm, openhd_wifi_status->wifi_card_type, openhd_wifi_status->received_packet_cnt);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS, (const char *)openhd_wifi_status, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_wifi_status_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t target_system, uint8_t target_component, uint8_t wifi_adapter_cnt, const int8_t *current_signal_dbm, const uint8_t *wifi_card_type, const uint32_t *received_packet_cnt)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint8_t(buf, 16, target_system);
    _mav_put_uint8_t(buf, 17, target_component);
    _mav_put_uint8_t(buf, 18, wifi_adapter_cnt);
    _mav_put_uint32_t_array(buf, 0, received_packet_cnt, 4);
    _mav_put_int8_t_array(buf, 19, current_signal_dbm, 4);
    _mav_put_uint8_t_array(buf, 23, wifi_card_type, 4);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS, buf, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_CRC);
#else
    mavlink_openhd_wifi_status_t *packet = (mavlink_openhd_wifi_status_t *)msgbuf;
    packet->target_system = target_system;
    packet->target_component = target_component;
    packet->wifi_adapter_cnt = wifi_adapter_cnt;
    mav_array_memcpy(packet->received_packet_cnt, received_packet_cnt, sizeof(uint32_t)*4);
    mav_array_memcpy(packet->current_signal_dbm, current_signal_dbm, sizeof(int8_t)*4);
    mav_array_memcpy(packet->wifi_card_type, wifi_card_type, sizeof(uint8_t)*4);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS, (const char *)packet, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_WIFI_STATUS UNPACKING


/**
 * @brief Get field target_system from openhd_wifi_status message
 *
 * @return  system id of the requesting system
 */
static inline uint8_t mavlink_msg_openhd_wifi_status_get_target_system(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  16);
}

/**
 * @brief Get field target_component from openhd_wifi_status message
 *
 * @return  component id of the requesting component
 */
static inline uint8_t mavlink_msg_openhd_wifi_status_get_target_component(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  17);
}

/**
 * @brief Get field wifi_adapter_cnt from openhd_wifi_status message
 *
 * @return  wifi_adapter_cnt
 */
static inline uint8_t mavlink_msg_openhd_wifi_status_get_wifi_adapter_cnt(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  18);
}

/**
 * @brief Get field current_signal_dbm from openhd_wifi_status message
 *
 * @return  current_signal_dbm
 */
static inline uint16_t mavlink_msg_openhd_wifi_status_get_current_signal_dbm(const mavlink_message_t* msg, int8_t *current_signal_dbm)
{
    return _MAV_RETURN_int8_t_array(msg, current_signal_dbm, 4,  19);
}

/**
 * @brief Get field wifi_card_type from openhd_wifi_status message
 *
 * @return  wifi_card_type
 */
static inline uint16_t mavlink_msg_openhd_wifi_status_get_wifi_card_type(const mavlink_message_t* msg, uint8_t *wifi_card_type)
{
    return _MAV_RETURN_uint8_t_array(msg, wifi_card_type, 4,  23);
}

/**
 * @brief Get field received_packet_cnt from openhd_wifi_status message
 *
 * @return  received_packet_cnt
 */
static inline uint16_t mavlink_msg_openhd_wifi_status_get_received_packet_cnt(const mavlink_message_t* msg, uint32_t *received_packet_cnt)
{
    return _MAV_RETURN_uint32_t_array(msg, received_packet_cnt, 4,  0);
}

/**
 * @brief Decode a openhd_wifi_status message into a struct
 *
 * @param msg The message to decode
 * @param openhd_wifi_status C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_wifi_status_decode(const mavlink_message_t* msg, mavlink_openhd_wifi_status_t* openhd_wifi_status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_wifi_status_get_received_packet_cnt(msg, openhd_wifi_status->received_packet_cnt);
    openhd_wifi_status->target_system = mavlink_msg_openhd_wifi_status_get_target_system(msg);
    openhd_wifi_status->target_component = mavlink_msg_openhd_wifi_status_get_target_component(msg);
    openhd_wifi_status->wifi_adapter_cnt = mavlink_msg_openhd_wifi_status_get_wifi_adapter_cnt(msg);
    mavlink_msg_openhd_wifi_status_get_current_signal_dbm(msg, openhd_wifi_status->current_signal_dbm);
    mavlink_msg_openhd_wifi_status_get_wifi_card_type(msg, openhd_wifi_status->wifi_card_type);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN;
        memset(openhd_wifi_status, 0, MAVLINK_MSG_ID_OPENHD_WIFI_STATUS_LEN);
    memcpy(openhd_wifi_status, _MAV_PAYLOAD(msg), len);
#endif
}
