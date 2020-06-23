#pragma once
// MESSAGE OPENHD_GROUND_POWER PACKING

#define MAVLINK_MSG_ID_OPENHD_GROUND_POWER 1240

MAVPACKED(
typedef struct __mavlink_openhd_ground_power_t {
 float vin; /*<  vin*/
 float vout; /*<  vout*/
 float vbat; /*<  vbat*/
 float iout; /*<  iout*/
 uint8_t target_system; /*<  system id of the requesting system*/
 uint8_t target_component; /*<  component id of the requesting component*/
 uint8_t type; /*<  Type (chemistry) of the battery*/
}) mavlink_openhd_ground_power_t;

#define MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN 19
#define MAVLINK_MSG_ID_OPENHD_GROUND_POWER_MIN_LEN 19
#define MAVLINK_MSG_ID_1240_LEN 19
#define MAVLINK_MSG_ID_1240_MIN_LEN 19

#define MAVLINK_MSG_ID_OPENHD_GROUND_POWER_CRC 166
#define MAVLINK_MSG_ID_1240_CRC 166



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_GROUND_POWER { \
    1240, \
    "OPENHD_GROUND_POWER", \
    7, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 16, offsetof(mavlink_openhd_ground_power_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 17, offsetof(mavlink_openhd_ground_power_t, target_component) }, \
         { "vin", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_openhd_ground_power_t, vin) }, \
         { "vout", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_openhd_ground_power_t, vout) }, \
         { "vbat", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_openhd_ground_power_t, vbat) }, \
         { "iout", NULL, MAVLINK_TYPE_FLOAT, 0, 12, offsetof(mavlink_openhd_ground_power_t, iout) }, \
         { "type", NULL, MAVLINK_TYPE_UINT8_T, 0, 18, offsetof(mavlink_openhd_ground_power_t, type) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_GROUND_POWER { \
    "OPENHD_GROUND_POWER", \
    7, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 16, offsetof(mavlink_openhd_ground_power_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 17, offsetof(mavlink_openhd_ground_power_t, target_component) }, \
         { "vin", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_openhd_ground_power_t, vin) }, \
         { "vout", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_openhd_ground_power_t, vout) }, \
         { "vbat", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_openhd_ground_power_t, vbat) }, \
         { "iout", NULL, MAVLINK_TYPE_FLOAT, 0, 12, offsetof(mavlink_openhd_ground_power_t, iout) }, \
         { "type", NULL, MAVLINK_TYPE_UINT8_T, 0, 18, offsetof(mavlink_openhd_ground_power_t, type) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_ground_power message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param vin  vin
 * @param vout  vout
 * @param vbat  vbat
 * @param iout  iout
 * @param type  Type (chemistry) of the battery
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_ground_power_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t target_system, uint8_t target_component, float vin, float vout, float vbat, float iout, uint8_t type)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN];
    _mav_put_float(buf, 0, vin);
    _mav_put_float(buf, 4, vout);
    _mav_put_float(buf, 8, vbat);
    _mav_put_float(buf, 12, iout);
    _mav_put_uint8_t(buf, 16, target_system);
    _mav_put_uint8_t(buf, 17, target_component);
    _mav_put_uint8_t(buf, 18, type);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN);
#else
    mavlink_openhd_ground_power_t packet;
    packet.vin = vin;
    packet.vout = vout;
    packet.vbat = vbat;
    packet.iout = iout;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.type = type;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_GROUND_POWER;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_CRC);
}

/**
 * @brief Pack a openhd_ground_power message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param vin  vin
 * @param vout  vout
 * @param vbat  vbat
 * @param iout  iout
 * @param type  Type (chemistry) of the battery
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_ground_power_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t target_system,uint8_t target_component,float vin,float vout,float vbat,float iout,uint8_t type)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN];
    _mav_put_float(buf, 0, vin);
    _mav_put_float(buf, 4, vout);
    _mav_put_float(buf, 8, vbat);
    _mav_put_float(buf, 12, iout);
    _mav_put_uint8_t(buf, 16, target_system);
    _mav_put_uint8_t(buf, 17, target_component);
    _mav_put_uint8_t(buf, 18, type);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN);
#else
    mavlink_openhd_ground_power_t packet;
    packet.vin = vin;
    packet.vout = vout;
    packet.vbat = vbat;
    packet.iout = iout;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.type = type;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_GROUND_POWER;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_CRC);
}

/**
 * @brief Encode a openhd_ground_power struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_ground_power C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_ground_power_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_ground_power_t* openhd_ground_power)
{
    return mavlink_msg_openhd_ground_power_pack(system_id, component_id, msg, openhd_ground_power->target_system, openhd_ground_power->target_component, openhd_ground_power->vin, openhd_ground_power->vout, openhd_ground_power->vbat, openhd_ground_power->iout, openhd_ground_power->type);
}

/**
 * @brief Encode a openhd_ground_power struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_ground_power C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_ground_power_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_ground_power_t* openhd_ground_power)
{
    return mavlink_msg_openhd_ground_power_pack_chan(system_id, component_id, chan, msg, openhd_ground_power->target_system, openhd_ground_power->target_component, openhd_ground_power->vin, openhd_ground_power->vout, openhd_ground_power->vbat, openhd_ground_power->iout, openhd_ground_power->type);
}

/**
 * @brief Send a openhd_ground_power message
 * @param chan MAVLink channel to send the message
 *
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param vin  vin
 * @param vout  vout
 * @param vbat  vbat
 * @param iout  iout
 * @param type  Type (chemistry) of the battery
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_ground_power_send(mavlink_channel_t chan, uint8_t target_system, uint8_t target_component, float vin, float vout, float vbat, float iout, uint8_t type)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN];
    _mav_put_float(buf, 0, vin);
    _mav_put_float(buf, 4, vout);
    _mav_put_float(buf, 8, vbat);
    _mav_put_float(buf, 12, iout);
    _mav_put_uint8_t(buf, 16, target_system);
    _mav_put_uint8_t(buf, 17, target_component);
    _mav_put_uint8_t(buf, 18, type);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_GROUND_POWER, buf, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_CRC);
#else
    mavlink_openhd_ground_power_t packet;
    packet.vin = vin;
    packet.vout = vout;
    packet.vbat = vbat;
    packet.iout = iout;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.type = type;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_GROUND_POWER, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_CRC);
#endif
}

/**
 * @brief Send a openhd_ground_power message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_ground_power_send_struct(mavlink_channel_t chan, const mavlink_openhd_ground_power_t* openhd_ground_power)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_ground_power_send(chan, openhd_ground_power->target_system, openhd_ground_power->target_component, openhd_ground_power->vin, openhd_ground_power->vout, openhd_ground_power->vbat, openhd_ground_power->iout, openhd_ground_power->type);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_GROUND_POWER, (const char *)openhd_ground_power, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_ground_power_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t target_system, uint8_t target_component, float vin, float vout, float vbat, float iout, uint8_t type)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_float(buf, 0, vin);
    _mav_put_float(buf, 4, vout);
    _mav_put_float(buf, 8, vbat);
    _mav_put_float(buf, 12, iout);
    _mav_put_uint8_t(buf, 16, target_system);
    _mav_put_uint8_t(buf, 17, target_component);
    _mav_put_uint8_t(buf, 18, type);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_GROUND_POWER, buf, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_CRC);
#else
    mavlink_openhd_ground_power_t *packet = (mavlink_openhd_ground_power_t *)msgbuf;
    packet->vin = vin;
    packet->vout = vout;
    packet->vbat = vbat;
    packet->iout = iout;
    packet->target_system = target_system;
    packet->target_component = target_component;
    packet->type = type;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_GROUND_POWER, (const char *)packet, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_GROUND_POWER UNPACKING


/**
 * @brief Get field target_system from openhd_ground_power message
 *
 * @return  system id of the requesting system
 */
static inline uint8_t mavlink_msg_openhd_ground_power_get_target_system(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  16);
}

/**
 * @brief Get field target_component from openhd_ground_power message
 *
 * @return  component id of the requesting component
 */
static inline uint8_t mavlink_msg_openhd_ground_power_get_target_component(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  17);
}

/**
 * @brief Get field vin from openhd_ground_power message
 *
 * @return  vin
 */
static inline float mavlink_msg_openhd_ground_power_get_vin(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field vout from openhd_ground_power message
 *
 * @return  vout
 */
static inline float mavlink_msg_openhd_ground_power_get_vout(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Get field vbat from openhd_ground_power message
 *
 * @return  vbat
 */
static inline float mavlink_msg_openhd_ground_power_get_vbat(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  8);
}

/**
 * @brief Get field iout from openhd_ground_power message
 *
 * @return  iout
 */
static inline float mavlink_msg_openhd_ground_power_get_iout(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  12);
}

/**
 * @brief Get field type from openhd_ground_power message
 *
 * @return  Type (chemistry) of the battery
 */
static inline uint8_t mavlink_msg_openhd_ground_power_get_type(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  18);
}

/**
 * @brief Decode a openhd_ground_power message into a struct
 *
 * @param msg The message to decode
 * @param openhd_ground_power C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_ground_power_decode(const mavlink_message_t* msg, mavlink_openhd_ground_power_t* openhd_ground_power)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_ground_power->vin = mavlink_msg_openhd_ground_power_get_vin(msg);
    openhd_ground_power->vout = mavlink_msg_openhd_ground_power_get_vout(msg);
    openhd_ground_power->vbat = mavlink_msg_openhd_ground_power_get_vbat(msg);
    openhd_ground_power->iout = mavlink_msg_openhd_ground_power_get_iout(msg);
    openhd_ground_power->target_system = mavlink_msg_openhd_ground_power_get_target_system(msg);
    openhd_ground_power->target_component = mavlink_msg_openhd_ground_power_get_target_component(msg);
    openhd_ground_power->type = mavlink_msg_openhd_ground_power_get_type(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN;
        memset(openhd_ground_power, 0, MAVLINK_MSG_ID_OPENHD_GROUND_POWER_LEN);
    memcpy(openhd_ground_power, _MAV_PAYLOAD(msg), len);
#endif
}
