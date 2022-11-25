#pragma once
// MESSAGE OPENHD_LOG_MESSAGE PACKING

#define MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE 1260


typedef struct __mavlink_openhd_log_message_t {
 uint64_t timestamp; /*<  timestamp when message was originally generated*/
 uint8_t severity; /*<  severity level, relies on the definitions within RFC-5424.*/
 char tag[10]; /*<  log tag,must not be null-terminated*/
 char message[50]; /*<  log message,must not be null-terminated*/
} mavlink_openhd_log_message_t;

#define MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN 69
#define MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_MIN_LEN 69
#define MAVLINK_MSG_ID_1260_LEN 69
#define MAVLINK_MSG_ID_1260_MIN_LEN 69

#define MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_CRC 84
#define MAVLINK_MSG_ID_1260_CRC 84

#define MAVLINK_MSG_OPENHD_LOG_MESSAGE_FIELD_TAG_LEN 10
#define MAVLINK_MSG_OPENHD_LOG_MESSAGE_FIELD_MESSAGE_LEN 50

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_LOG_MESSAGE { \
    1260, \
    "OPENHD_LOG_MESSAGE", \
    4, \
    {  { "severity", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_openhd_log_message_t, severity) }, \
         { "tag", NULL, MAVLINK_TYPE_CHAR, 10, 9, offsetof(mavlink_openhd_log_message_t, tag) }, \
         { "message", NULL, MAVLINK_TYPE_CHAR, 50, 19, offsetof(mavlink_openhd_log_message_t, message) }, \
         { "timestamp", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_log_message_t, timestamp) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_LOG_MESSAGE { \
    "OPENHD_LOG_MESSAGE", \
    4, \
    {  { "severity", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_openhd_log_message_t, severity) }, \
         { "tag", NULL, MAVLINK_TYPE_CHAR, 10, 9, offsetof(mavlink_openhd_log_message_t, tag) }, \
         { "message", NULL, MAVLINK_TYPE_CHAR, 50, 19, offsetof(mavlink_openhd_log_message_t, message) }, \
         { "timestamp", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_log_message_t, timestamp) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_log_message message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param severity  severity level, relies on the definitions within RFC-5424.
 * @param tag  log tag,must not be null-terminated
 * @param message  log message,must not be null-terminated
 * @param timestamp  timestamp when message was originally generated
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_log_message_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t severity, const char *tag, const char *message, uint64_t timestamp)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN];
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_uint8_t(buf, 8, severity);
    _mav_put_char_array(buf, 9, tag, 10);
    _mav_put_char_array(buf, 19, message, 50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN);
#else
    mavlink_openhd_log_message_t packet;
    packet.timestamp = timestamp;
    packet.severity = severity;
    mav_array_memcpy(packet.tag, tag, sizeof(char)*10);
    mav_array_memcpy(packet.message, message, sizeof(char)*50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_CRC);
}

/**
 * @brief Pack a openhd_log_message message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param severity  severity level, relies on the definitions within RFC-5424.
 * @param tag  log tag,must not be null-terminated
 * @param message  log message,must not be null-terminated
 * @param timestamp  timestamp when message was originally generated
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_log_message_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t severity,const char *tag,const char *message,uint64_t timestamp)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN];
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_uint8_t(buf, 8, severity);
    _mav_put_char_array(buf, 9, tag, 10);
    _mav_put_char_array(buf, 19, message, 50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN);
#else
    mavlink_openhd_log_message_t packet;
    packet.timestamp = timestamp;
    packet.severity = severity;
    mav_array_memcpy(packet.tag, tag, sizeof(char)*10);
    mav_array_memcpy(packet.message, message, sizeof(char)*50);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_CRC);
}

/**
 * @brief Encode a openhd_log_message struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_log_message C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_log_message_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_log_message_t* openhd_log_message)
{
    return mavlink_msg_openhd_log_message_pack(system_id, component_id, msg, openhd_log_message->severity, openhd_log_message->tag, openhd_log_message->message, openhd_log_message->timestamp);
}

/**
 * @brief Encode a openhd_log_message struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_log_message C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_log_message_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_log_message_t* openhd_log_message)
{
    return mavlink_msg_openhd_log_message_pack_chan(system_id, component_id, chan, msg, openhd_log_message->severity, openhd_log_message->tag, openhd_log_message->message, openhd_log_message->timestamp);
}

/**
 * @brief Send a openhd_log_message message
 * @param chan MAVLink channel to send the message
 *
 * @param severity  severity level, relies on the definitions within RFC-5424.
 * @param tag  log tag,must not be null-terminated
 * @param message  log message,must not be null-terminated
 * @param timestamp  timestamp when message was originally generated
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_log_message_send(mavlink_channel_t chan, uint8_t severity, const char *tag, const char *message, uint64_t timestamp)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN];
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_uint8_t(buf, 8, severity);
    _mav_put_char_array(buf, 9, tag, 10);
    _mav_put_char_array(buf, 19, message, 50);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE, buf, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_CRC);
#else
    mavlink_openhd_log_message_t packet;
    packet.timestamp = timestamp;
    packet.severity = severity;
    mav_array_memcpy(packet.tag, tag, sizeof(char)*10);
    mav_array_memcpy(packet.message, message, sizeof(char)*50);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_CRC);
#endif
}

/**
 * @brief Send a openhd_log_message message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_log_message_send_struct(mavlink_channel_t chan, const mavlink_openhd_log_message_t* openhd_log_message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_log_message_send(chan, openhd_log_message->severity, openhd_log_message->tag, openhd_log_message->message, openhd_log_message->timestamp);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE, (const char *)openhd_log_message, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_log_message_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t severity, const char *tag, const char *message, uint64_t timestamp)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_uint8_t(buf, 8, severity);
    _mav_put_char_array(buf, 9, tag, 10);
    _mav_put_char_array(buf, 19, message, 50);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE, buf, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_CRC);
#else
    mavlink_openhd_log_message_t *packet = (mavlink_openhd_log_message_t *)msgbuf;
    packet->timestamp = timestamp;
    packet->severity = severity;
    mav_array_memcpy(packet->tag, tag, sizeof(char)*10);
    mav_array_memcpy(packet->message, message, sizeof(char)*50);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE, (const char *)packet, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_LOG_MESSAGE UNPACKING


/**
 * @brief Get field severity from openhd_log_message message
 *
 * @return  severity level, relies on the definitions within RFC-5424.
 */
static inline uint8_t mavlink_msg_openhd_log_message_get_severity(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  8);
}

/**
 * @brief Get field tag from openhd_log_message message
 *
 * @return  log tag,must not be null-terminated
 */
static inline uint16_t mavlink_msg_openhd_log_message_get_tag(const mavlink_message_t* msg, char *tag)
{
    return _MAV_RETURN_char_array(msg, tag, 10,  9);
}

/**
 * @brief Get field message from openhd_log_message message
 *
 * @return  log message,must not be null-terminated
 */
static inline uint16_t mavlink_msg_openhd_log_message_get_message(const mavlink_message_t* msg, char *message)
{
    return _MAV_RETURN_char_array(msg, message, 50,  19);
}

/**
 * @brief Get field timestamp from openhd_log_message message
 *
 * @return  timestamp when message was originally generated
 */
static inline uint64_t mavlink_msg_openhd_log_message_get_timestamp(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  0);
}

/**
 * @brief Decode a openhd_log_message message into a struct
 *
 * @param msg The message to decode
 * @param openhd_log_message C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_log_message_decode(const mavlink_message_t* msg, mavlink_openhd_log_message_t* openhd_log_message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_log_message->timestamp = mavlink_msg_openhd_log_message_get_timestamp(msg);
    openhd_log_message->severity = mavlink_msg_openhd_log_message_get_severity(msg);
    mavlink_msg_openhd_log_message_get_tag(msg, openhd_log_message->tag);
    mavlink_msg_openhd_log_message_get_message(msg, openhd_log_message->message);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN;
        memset(openhd_log_message, 0, MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_LEN);
    memcpy(openhd_log_message, _MAV_PAYLOAD(msg), len);
#endif
}
