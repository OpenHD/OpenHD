#pragma once
// MESSAGE OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION PACKING

#define MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION 1216


typedef struct __mavlink_openhd_onboard_computer_status_extension_t {
 uint16_t cpu_core_voltage_milliV; /*<  cpu_core_voltage_milliV*/
 uint16_t reserved1; /*<  reserved1*/
 uint16_t reserved2; /*<  reserved1*/
 uint16_t reserved3; /*<  reserved1*/
 uint16_t reserved4; /*<  reserved1*/
 uint8_t over_current; /*<  bool over_current*/
} mavlink_openhd_onboard_computer_status_extension_t;

#define MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN 11
#define MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_MIN_LEN 11
#define MAVLINK_MSG_ID_1216_LEN 11
#define MAVLINK_MSG_ID_1216_MIN_LEN 11

#define MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_CRC 239
#define MAVLINK_MSG_ID_1216_CRC 239



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION { \
    1216, \
    "OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION", \
    6, \
    {  { "cpu_core_voltage_milliV", NULL, MAVLINK_TYPE_UINT16_T, 0, 0, offsetof(mavlink_openhd_onboard_computer_status_extension_t, cpu_core_voltage_milliV) }, \
         { "over_current", NULL, MAVLINK_TYPE_UINT8_T, 0, 10, offsetof(mavlink_openhd_onboard_computer_status_extension_t, over_current) }, \
         { "reserved1", NULL, MAVLINK_TYPE_UINT16_T, 0, 2, offsetof(mavlink_openhd_onboard_computer_status_extension_t, reserved1) }, \
         { "reserved2", NULL, MAVLINK_TYPE_UINT16_T, 0, 4, offsetof(mavlink_openhd_onboard_computer_status_extension_t, reserved2) }, \
         { "reserved3", NULL, MAVLINK_TYPE_UINT16_T, 0, 6, offsetof(mavlink_openhd_onboard_computer_status_extension_t, reserved3) }, \
         { "reserved4", NULL, MAVLINK_TYPE_UINT16_T, 0, 8, offsetof(mavlink_openhd_onboard_computer_status_extension_t, reserved4) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION { \
    "OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION", \
    6, \
    {  { "cpu_core_voltage_milliV", NULL, MAVLINK_TYPE_UINT16_T, 0, 0, offsetof(mavlink_openhd_onboard_computer_status_extension_t, cpu_core_voltage_milliV) }, \
         { "over_current", NULL, MAVLINK_TYPE_UINT8_T, 0, 10, offsetof(mavlink_openhd_onboard_computer_status_extension_t, over_current) }, \
         { "reserved1", NULL, MAVLINK_TYPE_UINT16_T, 0, 2, offsetof(mavlink_openhd_onboard_computer_status_extension_t, reserved1) }, \
         { "reserved2", NULL, MAVLINK_TYPE_UINT16_T, 0, 4, offsetof(mavlink_openhd_onboard_computer_status_extension_t, reserved2) }, \
         { "reserved3", NULL, MAVLINK_TYPE_UINT16_T, 0, 6, offsetof(mavlink_openhd_onboard_computer_status_extension_t, reserved3) }, \
         { "reserved4", NULL, MAVLINK_TYPE_UINT16_T, 0, 8, offsetof(mavlink_openhd_onboard_computer_status_extension_t, reserved4) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_onboard_computer_status_extension message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param cpu_core_voltage_milliV  cpu_core_voltage_milliV
 * @param over_current  bool over_current
 * @param reserved1  reserved1
 * @param reserved2  reserved1
 * @param reserved3  reserved1
 * @param reserved4  reserved1
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_onboard_computer_status_extension_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint16_t cpu_core_voltage_milliV, uint8_t over_current, uint16_t reserved1, uint16_t reserved2, uint16_t reserved3, uint16_t reserved4)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN];
    _mav_put_uint16_t(buf, 0, cpu_core_voltage_milliV);
    _mav_put_uint16_t(buf, 2, reserved1);
    _mav_put_uint16_t(buf, 4, reserved2);
    _mav_put_uint16_t(buf, 6, reserved3);
    _mav_put_uint16_t(buf, 8, reserved4);
    _mav_put_uint8_t(buf, 10, over_current);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN);
#else
    mavlink_openhd_onboard_computer_status_extension_t packet;
    packet.cpu_core_voltage_milliV = cpu_core_voltage_milliV;
    packet.reserved1 = reserved1;
    packet.reserved2 = reserved2;
    packet.reserved3 = reserved3;
    packet.reserved4 = reserved4;
    packet.over_current = over_current;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_MIN_LEN, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_CRC);
}

/**
 * @brief Pack a openhd_onboard_computer_status_extension message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param cpu_core_voltage_milliV  cpu_core_voltage_milliV
 * @param over_current  bool over_current
 * @param reserved1  reserved1
 * @param reserved2  reserved1
 * @param reserved3  reserved1
 * @param reserved4  reserved1
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_onboard_computer_status_extension_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint16_t cpu_core_voltage_milliV,uint8_t over_current,uint16_t reserved1,uint16_t reserved2,uint16_t reserved3,uint16_t reserved4)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN];
    _mav_put_uint16_t(buf, 0, cpu_core_voltage_milliV);
    _mav_put_uint16_t(buf, 2, reserved1);
    _mav_put_uint16_t(buf, 4, reserved2);
    _mav_put_uint16_t(buf, 6, reserved3);
    _mav_put_uint16_t(buf, 8, reserved4);
    _mav_put_uint8_t(buf, 10, over_current);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN);
#else
    mavlink_openhd_onboard_computer_status_extension_t packet;
    packet.cpu_core_voltage_milliV = cpu_core_voltage_milliV;
    packet.reserved1 = reserved1;
    packet.reserved2 = reserved2;
    packet.reserved3 = reserved3;
    packet.reserved4 = reserved4;
    packet.over_current = over_current;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_MIN_LEN, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_CRC);
}

/**
 * @brief Encode a openhd_onboard_computer_status_extension struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_onboard_computer_status_extension C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_onboard_computer_status_extension_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_onboard_computer_status_extension_t* openhd_onboard_computer_status_extension)
{
    return mavlink_msg_openhd_onboard_computer_status_extension_pack(system_id, component_id, msg, openhd_onboard_computer_status_extension->cpu_core_voltage_milliV, openhd_onboard_computer_status_extension->over_current, openhd_onboard_computer_status_extension->reserved1, openhd_onboard_computer_status_extension->reserved2, openhd_onboard_computer_status_extension->reserved3, openhd_onboard_computer_status_extension->reserved4);
}

/**
 * @brief Encode a openhd_onboard_computer_status_extension struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_onboard_computer_status_extension C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_onboard_computer_status_extension_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_onboard_computer_status_extension_t* openhd_onboard_computer_status_extension)
{
    return mavlink_msg_openhd_onboard_computer_status_extension_pack_chan(system_id, component_id, chan, msg, openhd_onboard_computer_status_extension->cpu_core_voltage_milliV, openhd_onboard_computer_status_extension->over_current, openhd_onboard_computer_status_extension->reserved1, openhd_onboard_computer_status_extension->reserved2, openhd_onboard_computer_status_extension->reserved3, openhd_onboard_computer_status_extension->reserved4);
}

/**
 * @brief Send a openhd_onboard_computer_status_extension message
 * @param chan MAVLink channel to send the message
 *
 * @param cpu_core_voltage_milliV  cpu_core_voltage_milliV
 * @param over_current  bool over_current
 * @param reserved1  reserved1
 * @param reserved2  reserved1
 * @param reserved3  reserved1
 * @param reserved4  reserved1
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_onboard_computer_status_extension_send(mavlink_channel_t chan, uint16_t cpu_core_voltage_milliV, uint8_t over_current, uint16_t reserved1, uint16_t reserved2, uint16_t reserved3, uint16_t reserved4)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN];
    _mav_put_uint16_t(buf, 0, cpu_core_voltage_milliV);
    _mav_put_uint16_t(buf, 2, reserved1);
    _mav_put_uint16_t(buf, 4, reserved2);
    _mav_put_uint16_t(buf, 6, reserved3);
    _mav_put_uint16_t(buf, 8, reserved4);
    _mav_put_uint8_t(buf, 10, over_current);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION, buf, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_MIN_LEN, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_CRC);
#else
    mavlink_openhd_onboard_computer_status_extension_t packet;
    packet.cpu_core_voltage_milliV = cpu_core_voltage_milliV;
    packet.reserved1 = reserved1;
    packet.reserved2 = reserved2;
    packet.reserved3 = reserved3;
    packet.reserved4 = reserved4;
    packet.over_current = over_current;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_MIN_LEN, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_CRC);
#endif
}

/**
 * @brief Send a openhd_onboard_computer_status_extension message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_onboard_computer_status_extension_send_struct(mavlink_channel_t chan, const mavlink_openhd_onboard_computer_status_extension_t* openhd_onboard_computer_status_extension)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_onboard_computer_status_extension_send(chan, openhd_onboard_computer_status_extension->cpu_core_voltage_milliV, openhd_onboard_computer_status_extension->over_current, openhd_onboard_computer_status_extension->reserved1, openhd_onboard_computer_status_extension->reserved2, openhd_onboard_computer_status_extension->reserved3, openhd_onboard_computer_status_extension->reserved4);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION, (const char *)openhd_onboard_computer_status_extension, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_MIN_LEN, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_onboard_computer_status_extension_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint16_t cpu_core_voltage_milliV, uint8_t over_current, uint16_t reserved1, uint16_t reserved2, uint16_t reserved3, uint16_t reserved4)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint16_t(buf, 0, cpu_core_voltage_milliV);
    _mav_put_uint16_t(buf, 2, reserved1);
    _mav_put_uint16_t(buf, 4, reserved2);
    _mav_put_uint16_t(buf, 6, reserved3);
    _mav_put_uint16_t(buf, 8, reserved4);
    _mav_put_uint8_t(buf, 10, over_current);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION, buf, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_MIN_LEN, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_CRC);
#else
    mavlink_openhd_onboard_computer_status_extension_t *packet = (mavlink_openhd_onboard_computer_status_extension_t *)msgbuf;
    packet->cpu_core_voltage_milliV = cpu_core_voltage_milliV;
    packet->reserved1 = reserved1;
    packet->reserved2 = reserved2;
    packet->reserved3 = reserved3;
    packet->reserved4 = reserved4;
    packet->over_current = over_current;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION, (const char *)packet, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_MIN_LEN, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION UNPACKING


/**
 * @brief Get field cpu_core_voltage_milliV from openhd_onboard_computer_status_extension message
 *
 * @return  cpu_core_voltage_milliV
 */
static inline uint16_t mavlink_msg_openhd_onboard_computer_status_extension_get_cpu_core_voltage_milliV(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  0);
}

/**
 * @brief Get field over_current from openhd_onboard_computer_status_extension message
 *
 * @return  bool over_current
 */
static inline uint8_t mavlink_msg_openhd_onboard_computer_status_extension_get_over_current(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  10);
}

/**
 * @brief Get field reserved1 from openhd_onboard_computer_status_extension message
 *
 * @return  reserved1
 */
static inline uint16_t mavlink_msg_openhd_onboard_computer_status_extension_get_reserved1(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  2);
}

/**
 * @brief Get field reserved2 from openhd_onboard_computer_status_extension message
 *
 * @return  reserved1
 */
static inline uint16_t mavlink_msg_openhd_onboard_computer_status_extension_get_reserved2(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  4);
}

/**
 * @brief Get field reserved3 from openhd_onboard_computer_status_extension message
 *
 * @return  reserved1
 */
static inline uint16_t mavlink_msg_openhd_onboard_computer_status_extension_get_reserved3(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  6);
}

/**
 * @brief Get field reserved4 from openhd_onboard_computer_status_extension message
 *
 * @return  reserved1
 */
static inline uint16_t mavlink_msg_openhd_onboard_computer_status_extension_get_reserved4(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  8);
}

/**
 * @brief Decode a openhd_onboard_computer_status_extension message into a struct
 *
 * @param msg The message to decode
 * @param openhd_onboard_computer_status_extension C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_onboard_computer_status_extension_decode(const mavlink_message_t* msg, mavlink_openhd_onboard_computer_status_extension_t* openhd_onboard_computer_status_extension)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_onboard_computer_status_extension->cpu_core_voltage_milliV = mavlink_msg_openhd_onboard_computer_status_extension_get_cpu_core_voltage_milliV(msg);
    openhd_onboard_computer_status_extension->reserved1 = mavlink_msg_openhd_onboard_computer_status_extension_get_reserved1(msg);
    openhd_onboard_computer_status_extension->reserved2 = mavlink_msg_openhd_onboard_computer_status_extension_get_reserved2(msg);
    openhd_onboard_computer_status_extension->reserved3 = mavlink_msg_openhd_onboard_computer_status_extension_get_reserved3(msg);
    openhd_onboard_computer_status_extension->reserved4 = mavlink_msg_openhd_onboard_computer_status_extension_get_reserved4(msg);
    openhd_onboard_computer_status_extension->over_current = mavlink_msg_openhd_onboard_computer_status_extension_get_over_current(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN;
        memset(openhd_onboard_computer_status_extension, 0, MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_LEN);
    memcpy(openhd_onboard_computer_status_extension, _MAV_PAYLOAD(msg), len);
#endif
}
