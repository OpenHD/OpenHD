#pragma once
// MESSAGE OPENHD_STATUS_MESSAGE PACKING

#define MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE 1260

MAVPACKED(
typedef struct __mavlink_openhd_status_message_t {
 uint64_t timestamp; /*<  timestamp when message was originally generated*/
 uint8_t target_system; /*<  system id of the requesting system*/
 uint8_t target_component; /*<  component id of the requesting component*/
 uint8_t severity; /*<  severity level, relies on the definitions within RFC-5424.*/
 char text[50]; /*<  status message, 49 character max length since it *must* be null-terminated*/
}) mavlink_openhd_status_message_t;

#define MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN 61
#define MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_MIN_LEN 61
#define MAVLINK_MSG_ID_1260_LEN 61
#define MAVLINK_MSG_ID_1260_MIN_LEN 61

#define MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_CRC 90
#define MAVLINK_MSG_ID_1260_CRC 90

#define MAVLINK_MSG_OPENHD_STATUS_MESSAGE_FIELD_TEXT_LEN 50

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_STATUS_MESSAGE { \
    1260, \
    "OPENHD_STATUS_MESSAGE", \
    5, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_openhd_status_message_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 9, offsetof(mavlink_openhd_status_message_t, target_component) }, \
         { "severity", NULL, MAVLINK_TYPE_UINT8_T, 0, 10, offsetof(mavlink_openhd_status_message_t, severity) }, \
         { "text", NULL, MAVLINK_TYPE_CHAR, 50, 11, offsetof(mavlink_openhd_status_message_t, text) }, \
         { "timestamp", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_status_message_t, timestamp) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_STATUS_MESSAGE { \
    "OPENHD_STATUS_MESSAGE", \
    5, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_openhd_status_message_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 9, offsetof(mavlink_openhd_status_message_t, target_component) }, \
         { "severity", NULL, MAVLINK_TYPE_UINT8_T, 0, 10, offsetof(mavlink_openhd_status_message_t, severity) }, \
         { "text", NULL, MAVLINK_TYPE_CHAR, 50, 11, offsetof(mavlink_openhd_status_message_t, text) }, \
         { "timestamp", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_status_message_t, timestamp) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_status_message message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param severity  severity level, relies on the definitions within RFC-5424.
 * @param text  status message, 49 character max length since it *must* be null-terminated
 * @param timestamp  timestamp when message was originally generated
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_status_message_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t target_system, uint8_t target_component, uint8_t severity, const char *text, uint64_t timestamp)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN];
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_uint8_t(buf, 8, target_system);
    _mav_put_uint8_t(buf, 9, target_component);
    _mav_put_uint8_t(buf, 10, severity);
    _mav_put_char_array(buf, 11, text, 50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN);
#else
    mavlink_openhd_status_message_t packet;
    packet.timestamp = timestamp;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.severity = severity;
    mav_array_memcpy(packet.text, text, sizeof(char)*50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_CRC);
}

/**
 * @brief Pack a openhd_status_message message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param severity  severity level, relies on the definitions within RFC-5424.
 * @param text  status message, 49 character max length since it *must* be null-terminated
 * @param timestamp  timestamp when message was originally generated
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_status_message_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t target_system,uint8_t target_component,uint8_t severity,const char *text,uint64_t timestamp)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN];
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_uint8_t(buf, 8, target_system);
    _mav_put_uint8_t(buf, 9, target_component);
    _mav_put_uint8_t(buf, 10, severity);
    _mav_put_char_array(buf, 11, text, 50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN);
#else
    mavlink_openhd_status_message_t packet;
    packet.timestamp = timestamp;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.severity = severity;
    mav_array_memcpy(packet.text, text, sizeof(char)*50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_CRC);
}

/**
 * @brief Encode a openhd_status_message struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_status_message C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_status_message_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_status_message_t* openhd_status_message)
{
    return mavlink_msg_openhd_status_message_pack(system_id, component_id, msg, openhd_status_message->target_system, openhd_status_message->target_component, openhd_status_message->severity, openhd_status_message->text, openhd_status_message->timestamp);
}

/**
 * @brief Encode a openhd_status_message struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_status_message C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_status_message_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_status_message_t* openhd_status_message)
{
    return mavlink_msg_openhd_status_message_pack_chan(system_id, component_id, chan, msg, openhd_status_message->target_system, openhd_status_message->target_component, openhd_status_message->severity, openhd_status_message->text, openhd_status_message->timestamp);
}

/**
 * @brief Send a openhd_status_message message
 * @param chan MAVLink channel to send the message
 *
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param severity  severity level, relies on the definitions within RFC-5424.
 * @param text  status message, 49 character max length since it *must* be null-terminated
 * @param timestamp  timestamp when message was originally generated
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_status_message_send(mavlink_channel_t chan, uint8_t target_system, uint8_t target_component, uint8_t severity, const char *text, uint64_t timestamp)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN];
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_uint8_t(buf, 8, target_system);
    _mav_put_uint8_t(buf, 9, target_component);
    _mav_put_uint8_t(buf, 10, severity);
    _mav_put_char_array(buf, 11, text, 50);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE, buf, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_CRC);
#else
    mavlink_openhd_status_message_t packet;
    packet.timestamp = timestamp;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.severity = severity;
    mav_array_memcpy(packet.text, text, sizeof(char)*50);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_CRC);
#endif
}

/**
 * @brief Send a openhd_status_message message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_status_message_send_struct(mavlink_channel_t chan, const mavlink_openhd_status_message_t* openhd_status_message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_status_message_send(chan, openhd_status_message->target_system, openhd_status_message->target_component, openhd_status_message->severity, openhd_status_message->text, openhd_status_message->timestamp);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE, (const char *)openhd_status_message, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_status_message_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t target_system, uint8_t target_component, uint8_t severity, const char *text, uint64_t timestamp)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_uint8_t(buf, 8, target_system);
    _mav_put_uint8_t(buf, 9, target_component);
    _mav_put_uint8_t(buf, 10, severity);
    _mav_put_char_array(buf, 11, text, 50);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE, buf, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_CRC);
#else
    mavlink_openhd_status_message_t *packet = (mavlink_openhd_status_message_t *)msgbuf;
    packet->timestamp = timestamp;
    packet->target_system = target_system;
    packet->target_component = target_component;
    packet->severity = severity;
    mav_array_memcpy(packet->text, text, sizeof(char)*50);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE, (const char *)packet, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_STATUS_MESSAGE UNPACKING


/**
 * @brief Get field target_system from openhd_status_message message
 *
 * @return  system id of the requesting system
 */
static inline uint8_t mavlink_msg_openhd_status_message_get_target_system(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  8);
}

/**
 * @brief Get field target_component from openhd_status_message message
 *
 * @return  component id of the requesting component
 */
static inline uint8_t mavlink_msg_openhd_status_message_get_target_component(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  9);
}

/**
 * @brief Get field severity from openhd_status_message message
 *
 * @return  severity level, relies on the definitions within RFC-5424.
 */
static inline uint8_t mavlink_msg_openhd_status_message_get_severity(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  10);
}

/**
 * @brief Get field text from openhd_status_message message
 *
 * @return  status message, 49 character max length since it *must* be null-terminated
 */
static inline uint16_t mavlink_msg_openhd_status_message_get_text(const mavlink_message_t* msg, char *text)
{
    return _MAV_RETURN_char_array(msg, text, 50,  11);
}

/**
 * @brief Get field timestamp from openhd_status_message message
 *
 * @return  timestamp when message was originally generated
 */
static inline uint64_t mavlink_msg_openhd_status_message_get_timestamp(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  0);
}

/**
 * @brief Decode a openhd_status_message message into a struct
 *
 * @param msg The message to decode
 * @param openhd_status_message C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_status_message_decode(const mavlink_message_t* msg, mavlink_openhd_status_message_t* openhd_status_message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_status_message->timestamp = mavlink_msg_openhd_status_message_get_timestamp(msg);
    openhd_status_message->target_system = mavlink_msg_openhd_status_message_get_target_system(msg);
    openhd_status_message->target_component = mavlink_msg_openhd_status_message_get_target_component(msg);
    openhd_status_message->severity = mavlink_msg_openhd_status_message_get_severity(msg);
    mavlink_msg_openhd_status_message_get_text(msg, openhd_status_message->text);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN;
        memset(openhd_status_message, 0, MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE_LEN);
    memcpy(openhd_status_message, _MAV_PAYLOAD(msg), len);
#endif
}
