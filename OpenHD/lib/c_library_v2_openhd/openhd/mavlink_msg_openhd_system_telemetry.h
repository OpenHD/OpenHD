#pragma once
// MESSAGE OPENHD_SYSTEM_TELEMETRY PACKING

#define MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY 1210


typedef struct __mavlink_openhd_system_telemetry_t {
 uint8_t cpuload; /*<  cpuload in percent*/
 uint8_t temperature; /*<  temperature in degree*/
 uint8_t reserved; /*<  reserved*/
} mavlink_openhd_system_telemetry_t;

#define MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN 3
#define MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_MIN_LEN 3
#define MAVLINK_MSG_ID_1210_LEN 3
#define MAVLINK_MSG_ID_1210_MIN_LEN 3

#define MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_CRC 159
#define MAVLINK_MSG_ID_1210_CRC 159



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_SYSTEM_TELEMETRY { \
    1210, \
    "OPENHD_SYSTEM_TELEMETRY", \
    3, \
    {  { "cpuload", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_openhd_system_telemetry_t, cpuload) }, \
         { "temperature", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_openhd_system_telemetry_t, temperature) }, \
         { "reserved", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_openhd_system_telemetry_t, reserved) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_SYSTEM_TELEMETRY { \
    "OPENHD_SYSTEM_TELEMETRY", \
    3, \
    {  { "cpuload", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_openhd_system_telemetry_t, cpuload) }, \
         { "temperature", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_openhd_system_telemetry_t, temperature) }, \
         { "reserved", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_openhd_system_telemetry_t, reserved) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_system_telemetry message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param cpuload  cpuload in percent
 * @param temperature  temperature in degree
 * @param reserved  reserved
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_system_telemetry_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t cpuload, uint8_t temperature, uint8_t reserved)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN];
    _mav_put_uint8_t(buf, 0, cpuload);
    _mav_put_uint8_t(buf, 1, temperature);
    _mav_put_uint8_t(buf, 2, reserved);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN);
#else
    mavlink_openhd_system_telemetry_t packet;
    packet.cpuload = cpuload;
    packet.temperature = temperature;
    packet.reserved = reserved;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_CRC);
}

/**
 * @brief Pack a openhd_system_telemetry message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param cpuload  cpuload in percent
 * @param temperature  temperature in degree
 * @param reserved  reserved
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_system_telemetry_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t cpuload,uint8_t temperature,uint8_t reserved)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN];
    _mav_put_uint8_t(buf, 0, cpuload);
    _mav_put_uint8_t(buf, 1, temperature);
    _mav_put_uint8_t(buf, 2, reserved);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN);
#else
    mavlink_openhd_system_telemetry_t packet;
    packet.cpuload = cpuload;
    packet.temperature = temperature;
    packet.reserved = reserved;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_CRC);
}

/**
 * @brief Encode a openhd_system_telemetry struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_system_telemetry C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_system_telemetry_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_system_telemetry_t* openhd_system_telemetry)
{
    return mavlink_msg_openhd_system_telemetry_pack(system_id, component_id, msg, openhd_system_telemetry->cpuload, openhd_system_telemetry->temperature, openhd_system_telemetry->reserved);
}

/**
 * @brief Encode a openhd_system_telemetry struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_system_telemetry C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_system_telemetry_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_system_telemetry_t* openhd_system_telemetry)
{
    return mavlink_msg_openhd_system_telemetry_pack_chan(system_id, component_id, chan, msg, openhd_system_telemetry->cpuload, openhd_system_telemetry->temperature, openhd_system_telemetry->reserved);
}

/**
 * @brief Send a openhd_system_telemetry message
 * @param chan MAVLink channel to send the message
 *
 * @param cpuload  cpuload in percent
 * @param temperature  temperature in degree
 * @param reserved  reserved
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_system_telemetry_send(mavlink_channel_t chan, uint8_t cpuload, uint8_t temperature, uint8_t reserved)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN];
    _mav_put_uint8_t(buf, 0, cpuload);
    _mav_put_uint8_t(buf, 1, temperature);
    _mav_put_uint8_t(buf, 2, reserved);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY, buf, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_CRC);
#else
    mavlink_openhd_system_telemetry_t packet;
    packet.cpuload = cpuload;
    packet.temperature = temperature;
    packet.reserved = reserved;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_CRC);
#endif
}

/**
 * @brief Send a openhd_system_telemetry message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_system_telemetry_send_struct(mavlink_channel_t chan, const mavlink_openhd_system_telemetry_t* openhd_system_telemetry)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_system_telemetry_send(chan, openhd_system_telemetry->cpuload, openhd_system_telemetry->temperature, openhd_system_telemetry->reserved);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY, (const char *)openhd_system_telemetry, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_system_telemetry_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t cpuload, uint8_t temperature, uint8_t reserved)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint8_t(buf, 0, cpuload);
    _mav_put_uint8_t(buf, 1, temperature);
    _mav_put_uint8_t(buf, 2, reserved);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY, buf, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_CRC);
#else
    mavlink_openhd_system_telemetry_t *packet = (mavlink_openhd_system_telemetry_t *)msgbuf;
    packet->cpuload = cpuload;
    packet->temperature = temperature;
    packet->reserved = reserved;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY, (const char *)packet, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_SYSTEM_TELEMETRY UNPACKING


/**
 * @brief Get field cpuload from openhd_system_telemetry message
 *
 * @return  cpuload in percent
 */
static inline uint8_t mavlink_msg_openhd_system_telemetry_get_cpuload(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field temperature from openhd_system_telemetry message
 *
 * @return  temperature in degree
 */
static inline uint8_t mavlink_msg_openhd_system_telemetry_get_temperature(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Get field reserved from openhd_system_telemetry message
 *
 * @return  reserved
 */
static inline uint8_t mavlink_msg_openhd_system_telemetry_get_reserved(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  2);
}

/**
 * @brief Decode a openhd_system_telemetry message into a struct
 *
 * @param msg The message to decode
 * @param openhd_system_telemetry C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_system_telemetry_decode(const mavlink_message_t* msg, mavlink_openhd_system_telemetry_t* openhd_system_telemetry)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_system_telemetry->cpuload = mavlink_msg_openhd_system_telemetry_get_cpuload(msg);
    openhd_system_telemetry->temperature = mavlink_msg_openhd_system_telemetry_get_temperature(msg);
    openhd_system_telemetry->reserved = mavlink_msg_openhd_system_telemetry_get_reserved(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN;
        memset(openhd_system_telemetry, 0, MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY_LEN);
    memcpy(openhd_system_telemetry, _MAV_PAYLOAD(msg), len);
#endif
}
