#pragma once
// MESSAGE OPENHD_CAMERA_SETTINGS PACKING

#define MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS 1200

MAVPACKED(
typedef struct __mavlink_openhd_camera_settings_t {
 uint8_t target_system; /*<  system id of the requesting system*/
 uint8_t target_component; /*<  component id of the requesting component*/
 uint8_t brightness; /*<  brightness*/
 uint8_t contrast; /*<  contrast*/
 uint8_t saturation; /*<  saturation*/
}) mavlink_openhd_camera_settings_t;

#define MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN 5
#define MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_MIN_LEN 5
#define MAVLINK_MSG_ID_1200_LEN 5
#define MAVLINK_MSG_ID_1200_MIN_LEN 5

#define MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_CRC 14
#define MAVLINK_MSG_ID_1200_CRC 14



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_CAMERA_SETTINGS { \
    1200, \
    "OPENHD_CAMERA_SETTINGS", \
    5, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_openhd_camera_settings_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_openhd_camera_settings_t, target_component) }, \
         { "brightness", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_openhd_camera_settings_t, brightness) }, \
         { "contrast", NULL, MAVLINK_TYPE_UINT8_T, 0, 3, offsetof(mavlink_openhd_camera_settings_t, contrast) }, \
         { "saturation", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_openhd_camera_settings_t, saturation) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_CAMERA_SETTINGS { \
    "OPENHD_CAMERA_SETTINGS", \
    5, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_openhd_camera_settings_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_openhd_camera_settings_t, target_component) }, \
         { "brightness", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_openhd_camera_settings_t, brightness) }, \
         { "contrast", NULL, MAVLINK_TYPE_UINT8_T, 0, 3, offsetof(mavlink_openhd_camera_settings_t, contrast) }, \
         { "saturation", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_openhd_camera_settings_t, saturation) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_camera_settings message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param brightness  brightness
 * @param contrast  contrast
 * @param saturation  saturation
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_camera_settings_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t target_system, uint8_t target_component, uint8_t brightness, uint8_t contrast, uint8_t saturation)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN];
    _mav_put_uint8_t(buf, 0, target_system);
    _mav_put_uint8_t(buf, 1, target_component);
    _mav_put_uint8_t(buf, 2, brightness);
    _mav_put_uint8_t(buf, 3, contrast);
    _mav_put_uint8_t(buf, 4, saturation);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN);
#else
    mavlink_openhd_camera_settings_t packet;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.brightness = brightness;
    packet.contrast = contrast;
    packet.saturation = saturation;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_CRC);
}

/**
 * @brief Pack a openhd_camera_settings message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param brightness  brightness
 * @param contrast  contrast
 * @param saturation  saturation
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_camera_settings_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t target_system,uint8_t target_component,uint8_t brightness,uint8_t contrast,uint8_t saturation)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN];
    _mav_put_uint8_t(buf, 0, target_system);
    _mav_put_uint8_t(buf, 1, target_component);
    _mav_put_uint8_t(buf, 2, brightness);
    _mav_put_uint8_t(buf, 3, contrast);
    _mav_put_uint8_t(buf, 4, saturation);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN);
#else
    mavlink_openhd_camera_settings_t packet;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.brightness = brightness;
    packet.contrast = contrast;
    packet.saturation = saturation;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_CRC);
}

/**
 * @brief Encode a openhd_camera_settings struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_camera_settings C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_camera_settings_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_camera_settings_t* openhd_camera_settings)
{
    return mavlink_msg_openhd_camera_settings_pack(system_id, component_id, msg, openhd_camera_settings->target_system, openhd_camera_settings->target_component, openhd_camera_settings->brightness, openhd_camera_settings->contrast, openhd_camera_settings->saturation);
}

/**
 * @brief Encode a openhd_camera_settings struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_camera_settings C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_camera_settings_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_camera_settings_t* openhd_camera_settings)
{
    return mavlink_msg_openhd_camera_settings_pack_chan(system_id, component_id, chan, msg, openhd_camera_settings->target_system, openhd_camera_settings->target_component, openhd_camera_settings->brightness, openhd_camera_settings->contrast, openhd_camera_settings->saturation);
}

/**
 * @brief Send a openhd_camera_settings message
 * @param chan MAVLink channel to send the message
 *
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param brightness  brightness
 * @param contrast  contrast
 * @param saturation  saturation
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_camera_settings_send(mavlink_channel_t chan, uint8_t target_system, uint8_t target_component, uint8_t brightness, uint8_t contrast, uint8_t saturation)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN];
    _mav_put_uint8_t(buf, 0, target_system);
    _mav_put_uint8_t(buf, 1, target_component);
    _mav_put_uint8_t(buf, 2, brightness);
    _mav_put_uint8_t(buf, 3, contrast);
    _mav_put_uint8_t(buf, 4, saturation);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS, buf, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_CRC);
#else
    mavlink_openhd_camera_settings_t packet;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.brightness = brightness;
    packet.contrast = contrast;
    packet.saturation = saturation;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_CRC);
#endif
}

/**
 * @brief Send a openhd_camera_settings message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_camera_settings_send_struct(mavlink_channel_t chan, const mavlink_openhd_camera_settings_t* openhd_camera_settings)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_camera_settings_send(chan, openhd_camera_settings->target_system, openhd_camera_settings->target_component, openhd_camera_settings->brightness, openhd_camera_settings->contrast, openhd_camera_settings->saturation);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS, (const char *)openhd_camera_settings, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_camera_settings_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t target_system, uint8_t target_component, uint8_t brightness, uint8_t contrast, uint8_t saturation)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint8_t(buf, 0, target_system);
    _mav_put_uint8_t(buf, 1, target_component);
    _mav_put_uint8_t(buf, 2, brightness);
    _mav_put_uint8_t(buf, 3, contrast);
    _mav_put_uint8_t(buf, 4, saturation);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS, buf, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_CRC);
#else
    mavlink_openhd_camera_settings_t *packet = (mavlink_openhd_camera_settings_t *)msgbuf;
    packet->target_system = target_system;
    packet->target_component = target_component;
    packet->brightness = brightness;
    packet->contrast = contrast;
    packet->saturation = saturation;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS, (const char *)packet, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_CAMERA_SETTINGS UNPACKING


/**
 * @brief Get field target_system from openhd_camera_settings message
 *
 * @return  system id of the requesting system
 */
static inline uint8_t mavlink_msg_openhd_camera_settings_get_target_system(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field target_component from openhd_camera_settings message
 *
 * @return  component id of the requesting component
 */
static inline uint8_t mavlink_msg_openhd_camera_settings_get_target_component(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Get field brightness from openhd_camera_settings message
 *
 * @return  brightness
 */
static inline uint8_t mavlink_msg_openhd_camera_settings_get_brightness(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  2);
}

/**
 * @brief Get field contrast from openhd_camera_settings message
 *
 * @return  contrast
 */
static inline uint8_t mavlink_msg_openhd_camera_settings_get_contrast(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  3);
}

/**
 * @brief Get field saturation from openhd_camera_settings message
 *
 * @return  saturation
 */
static inline uint8_t mavlink_msg_openhd_camera_settings_get_saturation(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  4);
}

/**
 * @brief Decode a openhd_camera_settings message into a struct
 *
 * @param msg The message to decode
 * @param openhd_camera_settings C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_camera_settings_decode(const mavlink_message_t* msg, mavlink_openhd_camera_settings_t* openhd_camera_settings)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_camera_settings->target_system = mavlink_msg_openhd_camera_settings_get_target_system(msg);
    openhd_camera_settings->target_component = mavlink_msg_openhd_camera_settings_get_target_component(msg);
    openhd_camera_settings->brightness = mavlink_msg_openhd_camera_settings_get_brightness(msg);
    openhd_camera_settings->contrast = mavlink_msg_openhd_camera_settings_get_contrast(msg);
    openhd_camera_settings->saturation = mavlink_msg_openhd_camera_settings_get_saturation(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN;
        memset(openhd_camera_settings, 0, MAVLINK_MSG_ID_OPENHD_CAMERA_SETTINGS_LEN);
    memcpy(openhd_camera_settings, _MAV_PAYLOAD(msg), len);
#endif
}
