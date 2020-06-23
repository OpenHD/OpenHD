#pragma once
// MESSAGE COMPONENT_INFORMATION PACKING

#define MAVLINK_MSG_ID_COMPONENT_INFORMATION 395

MAVPACKED(
typedef struct __mavlink_component_information_t {
 uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot).*/
 uint32_t firmware_version; /*<  Version of the component firmware (v << 24 & 0xff = Dev, v << 16 & 0xff = Patch, v << 8 & 0xff = Minor, v & 0xff = Major)*/
 uint32_t hardware_version; /*<  Version of the component hardware (v << 24 & 0xff = Dev, v << 16 & 0xff = Patch, v << 8 & 0xff = Minor, v & 0xff = Major)*/
 uint32_t capability_flags; /*<  Bitmap of component capability flags.*/
 uint16_t component_definition_version; /*<  Component definition version (iteration)*/
 uint8_t vendor_name[32]; /*<  Name of the component vendor*/
 uint8_t model_name[32]; /*<  Name of the component model*/
 char component_definition_uri[140]; /*<  Component definition URI (if any, otherwise only basic functions will be available). The XML format is not yet specified and work in progress. */
}) mavlink_component_information_t;

#define MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN 222
#define MAVLINK_MSG_ID_COMPONENT_INFORMATION_MIN_LEN 222
#define MAVLINK_MSG_ID_395_LEN 222
#define MAVLINK_MSG_ID_395_MIN_LEN 222

#define MAVLINK_MSG_ID_COMPONENT_INFORMATION_CRC 231
#define MAVLINK_MSG_ID_395_CRC 231

#define MAVLINK_MSG_COMPONENT_INFORMATION_FIELD_VENDOR_NAME_LEN 32
#define MAVLINK_MSG_COMPONENT_INFORMATION_FIELD_MODEL_NAME_LEN 32
#define MAVLINK_MSG_COMPONENT_INFORMATION_FIELD_COMPONENT_DEFINITION_URI_LEN 140

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_COMPONENT_INFORMATION { \
    395, \
    "COMPONENT_INFORMATION", \
    8, \
    {  { "time_boot_ms", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_component_information_t, time_boot_ms) }, \
         { "vendor_name", NULL, MAVLINK_TYPE_UINT8_T, 32, 18, offsetof(mavlink_component_information_t, vendor_name) }, \
         { "model_name", NULL, MAVLINK_TYPE_UINT8_T, 32, 50, offsetof(mavlink_component_information_t, model_name) }, \
         { "firmware_version", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_component_information_t, firmware_version) }, \
         { "hardware_version", NULL, MAVLINK_TYPE_UINT32_T, 0, 8, offsetof(mavlink_component_information_t, hardware_version) }, \
         { "capability_flags", NULL, MAVLINK_TYPE_UINT32_T, 0, 12, offsetof(mavlink_component_information_t, capability_flags) }, \
         { "component_definition_version", NULL, MAVLINK_TYPE_UINT16_T, 0, 16, offsetof(mavlink_component_information_t, component_definition_version) }, \
         { "component_definition_uri", NULL, MAVLINK_TYPE_CHAR, 140, 82, offsetof(mavlink_component_information_t, component_definition_uri) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_COMPONENT_INFORMATION { \
    "COMPONENT_INFORMATION", \
    8, \
    {  { "time_boot_ms", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_component_information_t, time_boot_ms) }, \
         { "vendor_name", NULL, MAVLINK_TYPE_UINT8_T, 32, 18, offsetof(mavlink_component_information_t, vendor_name) }, \
         { "model_name", NULL, MAVLINK_TYPE_UINT8_T, 32, 50, offsetof(mavlink_component_information_t, model_name) }, \
         { "firmware_version", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_component_information_t, firmware_version) }, \
         { "hardware_version", NULL, MAVLINK_TYPE_UINT32_T, 0, 8, offsetof(mavlink_component_information_t, hardware_version) }, \
         { "capability_flags", NULL, MAVLINK_TYPE_UINT32_T, 0, 12, offsetof(mavlink_component_information_t, capability_flags) }, \
         { "component_definition_version", NULL, MAVLINK_TYPE_UINT16_T, 0, 16, offsetof(mavlink_component_information_t, component_definition_version) }, \
         { "component_definition_uri", NULL, MAVLINK_TYPE_CHAR, 140, 82, offsetof(mavlink_component_information_t, component_definition_uri) }, \
         } \
}
#endif

/**
 * @brief Pack a component_information message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param time_boot_ms [ms] Timestamp (time since system boot).
 * @param vendor_name  Name of the component vendor
 * @param model_name  Name of the component model
 * @param firmware_version  Version of the component firmware (v << 24 & 0xff = Dev, v << 16 & 0xff = Patch, v << 8 & 0xff = Minor, v & 0xff = Major)
 * @param hardware_version  Version of the component hardware (v << 24 & 0xff = Dev, v << 16 & 0xff = Patch, v << 8 & 0xff = Minor, v & 0xff = Major)
 * @param capability_flags  Bitmap of component capability flags.
 * @param component_definition_version  Component definition version (iteration)
 * @param component_definition_uri  Component definition URI (if any, otherwise only basic functions will be available). The XML format is not yet specified and work in progress. 
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_component_information_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint32_t time_boot_ms, const uint8_t *vendor_name, const uint8_t *model_name, uint32_t firmware_version, uint32_t hardware_version, uint32_t capability_flags, uint16_t component_definition_version, const char *component_definition_uri)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN];
    _mav_put_uint32_t(buf, 0, time_boot_ms);
    _mav_put_uint32_t(buf, 4, firmware_version);
    _mav_put_uint32_t(buf, 8, hardware_version);
    _mav_put_uint32_t(buf, 12, capability_flags);
    _mav_put_uint16_t(buf, 16, component_definition_version);
    _mav_put_uint8_t_array(buf, 18, vendor_name, 32);
    _mav_put_uint8_t_array(buf, 50, model_name, 32);
    _mav_put_char_array(buf, 82, component_definition_uri, 140);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN);
#else
    mavlink_component_information_t packet;
    packet.time_boot_ms = time_boot_ms;
    packet.firmware_version = firmware_version;
    packet.hardware_version = hardware_version;
    packet.capability_flags = capability_flags;
    packet.component_definition_version = component_definition_version;
    mav_array_memcpy(packet.vendor_name, vendor_name, sizeof(uint8_t)*32);
    mav_array_memcpy(packet.model_name, model_name, sizeof(uint8_t)*32);
    mav_array_memcpy(packet.component_definition_uri, component_definition_uri, sizeof(char)*140);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_COMPONENT_INFORMATION;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_COMPONENT_INFORMATION_MIN_LEN, MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN, MAVLINK_MSG_ID_COMPONENT_INFORMATION_CRC);
}

/**
 * @brief Pack a component_information message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param time_boot_ms [ms] Timestamp (time since system boot).
 * @param vendor_name  Name of the component vendor
 * @param model_name  Name of the component model
 * @param firmware_version  Version of the component firmware (v << 24 & 0xff = Dev, v << 16 & 0xff = Patch, v << 8 & 0xff = Minor, v & 0xff = Major)
 * @param hardware_version  Version of the component hardware (v << 24 & 0xff = Dev, v << 16 & 0xff = Patch, v << 8 & 0xff = Minor, v & 0xff = Major)
 * @param capability_flags  Bitmap of component capability flags.
 * @param component_definition_version  Component definition version (iteration)
 * @param component_definition_uri  Component definition URI (if any, otherwise only basic functions will be available). The XML format is not yet specified and work in progress. 
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_component_information_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint32_t time_boot_ms,const uint8_t *vendor_name,const uint8_t *model_name,uint32_t firmware_version,uint32_t hardware_version,uint32_t capability_flags,uint16_t component_definition_version,const char *component_definition_uri)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN];
    _mav_put_uint32_t(buf, 0, time_boot_ms);
    _mav_put_uint32_t(buf, 4, firmware_version);
    _mav_put_uint32_t(buf, 8, hardware_version);
    _mav_put_uint32_t(buf, 12, capability_flags);
    _mav_put_uint16_t(buf, 16, component_definition_version);
    _mav_put_uint8_t_array(buf, 18, vendor_name, 32);
    _mav_put_uint8_t_array(buf, 50, model_name, 32);
    _mav_put_char_array(buf, 82, component_definition_uri, 140);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN);
#else
    mavlink_component_information_t packet;
    packet.time_boot_ms = time_boot_ms;
    packet.firmware_version = firmware_version;
    packet.hardware_version = hardware_version;
    packet.capability_flags = capability_flags;
    packet.component_definition_version = component_definition_version;
    mav_array_memcpy(packet.vendor_name, vendor_name, sizeof(uint8_t)*32);
    mav_array_memcpy(packet.model_name, model_name, sizeof(uint8_t)*32);
    mav_array_memcpy(packet.component_definition_uri, component_definition_uri, sizeof(char)*140);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_COMPONENT_INFORMATION;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_COMPONENT_INFORMATION_MIN_LEN, MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN, MAVLINK_MSG_ID_COMPONENT_INFORMATION_CRC);
}

/**
 * @brief Encode a component_information struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param component_information C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_component_information_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_component_information_t* component_information)
{
    return mavlink_msg_component_information_pack(system_id, component_id, msg, component_information->time_boot_ms, component_information->vendor_name, component_information->model_name, component_information->firmware_version, component_information->hardware_version, component_information->capability_flags, component_information->component_definition_version, component_information->component_definition_uri);
}

/**
 * @brief Encode a component_information struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param component_information C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_component_information_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_component_information_t* component_information)
{
    return mavlink_msg_component_information_pack_chan(system_id, component_id, chan, msg, component_information->time_boot_ms, component_information->vendor_name, component_information->model_name, component_information->firmware_version, component_information->hardware_version, component_information->capability_flags, component_information->component_definition_version, component_information->component_definition_uri);
}

/**
 * @brief Send a component_information message
 * @param chan MAVLink channel to send the message
 *
 * @param time_boot_ms [ms] Timestamp (time since system boot).
 * @param vendor_name  Name of the component vendor
 * @param model_name  Name of the component model
 * @param firmware_version  Version of the component firmware (v << 24 & 0xff = Dev, v << 16 & 0xff = Patch, v << 8 & 0xff = Minor, v & 0xff = Major)
 * @param hardware_version  Version of the component hardware (v << 24 & 0xff = Dev, v << 16 & 0xff = Patch, v << 8 & 0xff = Minor, v & 0xff = Major)
 * @param capability_flags  Bitmap of component capability flags.
 * @param component_definition_version  Component definition version (iteration)
 * @param component_definition_uri  Component definition URI (if any, otherwise only basic functions will be available). The XML format is not yet specified and work in progress. 
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_component_information_send(mavlink_channel_t chan, uint32_t time_boot_ms, const uint8_t *vendor_name, const uint8_t *model_name, uint32_t firmware_version, uint32_t hardware_version, uint32_t capability_flags, uint16_t component_definition_version, const char *component_definition_uri)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN];
    _mav_put_uint32_t(buf, 0, time_boot_ms);
    _mav_put_uint32_t(buf, 4, firmware_version);
    _mav_put_uint32_t(buf, 8, hardware_version);
    _mav_put_uint32_t(buf, 12, capability_flags);
    _mav_put_uint16_t(buf, 16, component_definition_version);
    _mav_put_uint8_t_array(buf, 18, vendor_name, 32);
    _mav_put_uint8_t_array(buf, 50, model_name, 32);
    _mav_put_char_array(buf, 82, component_definition_uri, 140);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMPONENT_INFORMATION, buf, MAVLINK_MSG_ID_COMPONENT_INFORMATION_MIN_LEN, MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN, MAVLINK_MSG_ID_COMPONENT_INFORMATION_CRC);
#else
    mavlink_component_information_t packet;
    packet.time_boot_ms = time_boot_ms;
    packet.firmware_version = firmware_version;
    packet.hardware_version = hardware_version;
    packet.capability_flags = capability_flags;
    packet.component_definition_version = component_definition_version;
    mav_array_memcpy(packet.vendor_name, vendor_name, sizeof(uint8_t)*32);
    mav_array_memcpy(packet.model_name, model_name, sizeof(uint8_t)*32);
    mav_array_memcpy(packet.component_definition_uri, component_definition_uri, sizeof(char)*140);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMPONENT_INFORMATION, (const char *)&packet, MAVLINK_MSG_ID_COMPONENT_INFORMATION_MIN_LEN, MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN, MAVLINK_MSG_ID_COMPONENT_INFORMATION_CRC);
#endif
}

/**
 * @brief Send a component_information message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_component_information_send_struct(mavlink_channel_t chan, const mavlink_component_information_t* component_information)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_component_information_send(chan, component_information->time_boot_ms, component_information->vendor_name, component_information->model_name, component_information->firmware_version, component_information->hardware_version, component_information->capability_flags, component_information->component_definition_version, component_information->component_definition_uri);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMPONENT_INFORMATION, (const char *)component_information, MAVLINK_MSG_ID_COMPONENT_INFORMATION_MIN_LEN, MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN, MAVLINK_MSG_ID_COMPONENT_INFORMATION_CRC);
#endif
}

#if MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_component_information_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint32_t time_boot_ms, const uint8_t *vendor_name, const uint8_t *model_name, uint32_t firmware_version, uint32_t hardware_version, uint32_t capability_flags, uint16_t component_definition_version, const char *component_definition_uri)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint32_t(buf, 0, time_boot_ms);
    _mav_put_uint32_t(buf, 4, firmware_version);
    _mav_put_uint32_t(buf, 8, hardware_version);
    _mav_put_uint32_t(buf, 12, capability_flags);
    _mav_put_uint16_t(buf, 16, component_definition_version);
    _mav_put_uint8_t_array(buf, 18, vendor_name, 32);
    _mav_put_uint8_t_array(buf, 50, model_name, 32);
    _mav_put_char_array(buf, 82, component_definition_uri, 140);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMPONENT_INFORMATION, buf, MAVLINK_MSG_ID_COMPONENT_INFORMATION_MIN_LEN, MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN, MAVLINK_MSG_ID_COMPONENT_INFORMATION_CRC);
#else
    mavlink_component_information_t *packet = (mavlink_component_information_t *)msgbuf;
    packet->time_boot_ms = time_boot_ms;
    packet->firmware_version = firmware_version;
    packet->hardware_version = hardware_version;
    packet->capability_flags = capability_flags;
    packet->component_definition_version = component_definition_version;
    mav_array_memcpy(packet->vendor_name, vendor_name, sizeof(uint8_t)*32);
    mav_array_memcpy(packet->model_name, model_name, sizeof(uint8_t)*32);
    mav_array_memcpy(packet->component_definition_uri, component_definition_uri, sizeof(char)*140);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMPONENT_INFORMATION, (const char *)packet, MAVLINK_MSG_ID_COMPONENT_INFORMATION_MIN_LEN, MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN, MAVLINK_MSG_ID_COMPONENT_INFORMATION_CRC);
#endif
}
#endif

#endif

// MESSAGE COMPONENT_INFORMATION UNPACKING


/**
 * @brief Get field time_boot_ms from component_information message
 *
 * @return [ms] Timestamp (time since system boot).
 */
static inline uint32_t mavlink_msg_component_information_get_time_boot_ms(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Get field vendor_name from component_information message
 *
 * @return  Name of the component vendor
 */
static inline uint16_t mavlink_msg_component_information_get_vendor_name(const mavlink_message_t* msg, uint8_t *vendor_name)
{
    return _MAV_RETURN_uint8_t_array(msg, vendor_name, 32,  18);
}

/**
 * @brief Get field model_name from component_information message
 *
 * @return  Name of the component model
 */
static inline uint16_t mavlink_msg_component_information_get_model_name(const mavlink_message_t* msg, uint8_t *model_name)
{
    return _MAV_RETURN_uint8_t_array(msg, model_name, 32,  50);
}

/**
 * @brief Get field firmware_version from component_information message
 *
 * @return  Version of the component firmware (v << 24 & 0xff = Dev, v << 16 & 0xff = Patch, v << 8 & 0xff = Minor, v & 0xff = Major)
 */
static inline uint32_t mavlink_msg_component_information_get_firmware_version(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  4);
}

/**
 * @brief Get field hardware_version from component_information message
 *
 * @return  Version of the component hardware (v << 24 & 0xff = Dev, v << 16 & 0xff = Patch, v << 8 & 0xff = Minor, v & 0xff = Major)
 */
static inline uint32_t mavlink_msg_component_information_get_hardware_version(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  8);
}

/**
 * @brief Get field capability_flags from component_information message
 *
 * @return  Bitmap of component capability flags.
 */
static inline uint32_t mavlink_msg_component_information_get_capability_flags(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  12);
}

/**
 * @brief Get field component_definition_version from component_information message
 *
 * @return  Component definition version (iteration)
 */
static inline uint16_t mavlink_msg_component_information_get_component_definition_version(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  16);
}

/**
 * @brief Get field component_definition_uri from component_information message
 *
 * @return  Component definition URI (if any, otherwise only basic functions will be available). The XML format is not yet specified and work in progress. 
 */
static inline uint16_t mavlink_msg_component_information_get_component_definition_uri(const mavlink_message_t* msg, char *component_definition_uri)
{
    return _MAV_RETURN_char_array(msg, component_definition_uri, 140,  82);
}

/**
 * @brief Decode a component_information message into a struct
 *
 * @param msg The message to decode
 * @param component_information C-struct to decode the message contents into
 */
static inline void mavlink_msg_component_information_decode(const mavlink_message_t* msg, mavlink_component_information_t* component_information)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    component_information->time_boot_ms = mavlink_msg_component_information_get_time_boot_ms(msg);
    component_information->firmware_version = mavlink_msg_component_information_get_firmware_version(msg);
    component_information->hardware_version = mavlink_msg_component_information_get_hardware_version(msg);
    component_information->capability_flags = mavlink_msg_component_information_get_capability_flags(msg);
    component_information->component_definition_version = mavlink_msg_component_information_get_component_definition_version(msg);
    mavlink_msg_component_information_get_vendor_name(msg, component_information->vendor_name);
    mavlink_msg_component_information_get_model_name(msg, component_information->model_name);
    mavlink_msg_component_information_get_component_definition_uri(msg, component_information->component_definition_uri);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN? msg->len : MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN;
        memset(component_information, 0, MAVLINK_MSG_ID_COMPONENT_INFORMATION_LEN);
    memcpy(component_information, _MAV_PAYLOAD(msg), len);
#endif
}
