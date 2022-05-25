#pragma once
// MESSAGE OPEN_DRONE_ID_SYSTEM PACKING

#define MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM 12904

MAVPACKED(
typedef struct __mavlink_open_drone_id_system_t {
 int32_t operator_latitude; /*< [degE7] Latitude of the operator. If unknown: 0 deg (both Lat/Lon).*/
 int32_t operator_longitude; /*< [degE7] Longitude of the operator. If unknown: 0 deg (both Lat/Lon).*/
 float area_ceiling; /*< [m] Area Operations Ceiling relative to WGS84. If unknown: -1000 m.*/
 float area_floor; /*< [m] Area Operations Floor relative to WGS84. If unknown: -1000 m.*/
 uint16_t area_count; /*<  Number of aircraft in the area, group or formation (default 1).*/
 uint16_t area_radius; /*< [m] Radius of the cylindrical area of the group or formation (default 0).*/
 uint8_t flags; /*<  Specifies the location source for the operator location.*/
}) mavlink_open_drone_id_system_t;

#define MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN 21
#define MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_MIN_LEN 21
#define MAVLINK_MSG_ID_12904_LEN 21
#define MAVLINK_MSG_ID_12904_MIN_LEN 21

#define MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_CRC 238
#define MAVLINK_MSG_ID_12904_CRC 238



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPEN_DRONE_ID_SYSTEM { \
    12904, \
    "OPEN_DRONE_ID_SYSTEM", \
    7, \
    {  { "flags", NULL, MAVLINK_TYPE_UINT8_T, 0, 20, offsetof(mavlink_open_drone_id_system_t, flags) }, \
         { "operator_latitude", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_open_drone_id_system_t, operator_latitude) }, \
         { "operator_longitude", NULL, MAVLINK_TYPE_INT32_T, 0, 4, offsetof(mavlink_open_drone_id_system_t, operator_longitude) }, \
         { "area_count", NULL, MAVLINK_TYPE_UINT16_T, 0, 16, offsetof(mavlink_open_drone_id_system_t, area_count) }, \
         { "area_radius", NULL, MAVLINK_TYPE_UINT16_T, 0, 18, offsetof(mavlink_open_drone_id_system_t, area_radius) }, \
         { "area_ceiling", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_open_drone_id_system_t, area_ceiling) }, \
         { "area_floor", NULL, MAVLINK_TYPE_FLOAT, 0, 12, offsetof(mavlink_open_drone_id_system_t, area_floor) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPEN_DRONE_ID_SYSTEM { \
    "OPEN_DRONE_ID_SYSTEM", \
    7, \
    {  { "flags", NULL, MAVLINK_TYPE_UINT8_T, 0, 20, offsetof(mavlink_open_drone_id_system_t, flags) }, \
         { "operator_latitude", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_open_drone_id_system_t, operator_latitude) }, \
         { "operator_longitude", NULL, MAVLINK_TYPE_INT32_T, 0, 4, offsetof(mavlink_open_drone_id_system_t, operator_longitude) }, \
         { "area_count", NULL, MAVLINK_TYPE_UINT16_T, 0, 16, offsetof(mavlink_open_drone_id_system_t, area_count) }, \
         { "area_radius", NULL, MAVLINK_TYPE_UINT16_T, 0, 18, offsetof(mavlink_open_drone_id_system_t, area_radius) }, \
         { "area_ceiling", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_open_drone_id_system_t, area_ceiling) }, \
         { "area_floor", NULL, MAVLINK_TYPE_FLOAT, 0, 12, offsetof(mavlink_open_drone_id_system_t, area_floor) }, \
         } \
}
#endif

/**
 * @brief Pack a open_drone_id_system message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param flags  Specifies the location source for the operator location.
 * @param operator_latitude [degE7] Latitude of the operator. If unknown: 0 deg (both Lat/Lon).
 * @param operator_longitude [degE7] Longitude of the operator. If unknown: 0 deg (both Lat/Lon).
 * @param area_count  Number of aircraft in the area, group or formation (default 1).
 * @param area_radius [m] Radius of the cylindrical area of the group or formation (default 0).
 * @param area_ceiling [m] Area Operations Ceiling relative to WGS84. If unknown: -1000 m.
 * @param area_floor [m] Area Operations Floor relative to WGS84. If unknown: -1000 m.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_open_drone_id_system_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t flags, int32_t operator_latitude, int32_t operator_longitude, uint16_t area_count, uint16_t area_radius, float area_ceiling, float area_floor)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN];
    _mav_put_int32_t(buf, 0, operator_latitude);
    _mav_put_int32_t(buf, 4, operator_longitude);
    _mav_put_float(buf, 8, area_ceiling);
    _mav_put_float(buf, 12, area_floor);
    _mav_put_uint16_t(buf, 16, area_count);
    _mav_put_uint16_t(buf, 18, area_radius);
    _mav_put_uint8_t(buf, 20, flags);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN);
#else
    mavlink_open_drone_id_system_t packet;
    packet.operator_latitude = operator_latitude;
    packet.operator_longitude = operator_longitude;
    packet.area_ceiling = area_ceiling;
    packet.area_floor = area_floor;
    packet.area_count = area_count;
    packet.area_radius = area_radius;
    packet.flags = flags;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_MIN_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_CRC);
}

/**
 * @brief Pack a open_drone_id_system message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param flags  Specifies the location source for the operator location.
 * @param operator_latitude [degE7] Latitude of the operator. If unknown: 0 deg (both Lat/Lon).
 * @param operator_longitude [degE7] Longitude of the operator. If unknown: 0 deg (both Lat/Lon).
 * @param area_count  Number of aircraft in the area, group or formation (default 1).
 * @param area_radius [m] Radius of the cylindrical area of the group or formation (default 0).
 * @param area_ceiling [m] Area Operations Ceiling relative to WGS84. If unknown: -1000 m.
 * @param area_floor [m] Area Operations Floor relative to WGS84. If unknown: -1000 m.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_open_drone_id_system_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t flags,int32_t operator_latitude,int32_t operator_longitude,uint16_t area_count,uint16_t area_radius,float area_ceiling,float area_floor)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN];
    _mav_put_int32_t(buf, 0, operator_latitude);
    _mav_put_int32_t(buf, 4, operator_longitude);
    _mav_put_float(buf, 8, area_ceiling);
    _mav_put_float(buf, 12, area_floor);
    _mav_put_uint16_t(buf, 16, area_count);
    _mav_put_uint16_t(buf, 18, area_radius);
    _mav_put_uint8_t(buf, 20, flags);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN);
#else
    mavlink_open_drone_id_system_t packet;
    packet.operator_latitude = operator_latitude;
    packet.operator_longitude = operator_longitude;
    packet.area_ceiling = area_ceiling;
    packet.area_floor = area_floor;
    packet.area_count = area_count;
    packet.area_radius = area_radius;
    packet.flags = flags;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_MIN_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_CRC);
}

/**
 * @brief Encode a open_drone_id_system struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param open_drone_id_system C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_open_drone_id_system_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_open_drone_id_system_t* open_drone_id_system)
{
    return mavlink_msg_open_drone_id_system_pack(system_id, component_id, msg, open_drone_id_system->flags, open_drone_id_system->operator_latitude, open_drone_id_system->operator_longitude, open_drone_id_system->area_count, open_drone_id_system->area_radius, open_drone_id_system->area_ceiling, open_drone_id_system->area_floor);
}

/**
 * @brief Encode a open_drone_id_system struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param open_drone_id_system C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_open_drone_id_system_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_open_drone_id_system_t* open_drone_id_system)
{
    return mavlink_msg_open_drone_id_system_pack_chan(system_id, component_id, chan, msg, open_drone_id_system->flags, open_drone_id_system->operator_latitude, open_drone_id_system->operator_longitude, open_drone_id_system->area_count, open_drone_id_system->area_radius, open_drone_id_system->area_ceiling, open_drone_id_system->area_floor);
}

/**
 * @brief Send a open_drone_id_system message
 * @param chan MAVLink channel to send the message
 *
 * @param flags  Specifies the location source for the operator location.
 * @param operator_latitude [degE7] Latitude of the operator. If unknown: 0 deg (both Lat/Lon).
 * @param operator_longitude [degE7] Longitude of the operator. If unknown: 0 deg (both Lat/Lon).
 * @param area_count  Number of aircraft in the area, group or formation (default 1).
 * @param area_radius [m] Radius of the cylindrical area of the group or formation (default 0).
 * @param area_ceiling [m] Area Operations Ceiling relative to WGS84. If unknown: -1000 m.
 * @param area_floor [m] Area Operations Floor relative to WGS84. If unknown: -1000 m.
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_open_drone_id_system_send(mavlink_channel_t chan, uint8_t flags, int32_t operator_latitude, int32_t operator_longitude, uint16_t area_count, uint16_t area_radius, float area_ceiling, float area_floor)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN];
    _mav_put_int32_t(buf, 0, operator_latitude);
    _mav_put_int32_t(buf, 4, operator_longitude);
    _mav_put_float(buf, 8, area_ceiling);
    _mav_put_float(buf, 12, area_floor);
    _mav_put_uint16_t(buf, 16, area_count);
    _mav_put_uint16_t(buf, 18, area_radius);
    _mav_put_uint8_t(buf, 20, flags);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM, buf, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_MIN_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_CRC);
#else
    mavlink_open_drone_id_system_t packet;
    packet.operator_latitude = operator_latitude;
    packet.operator_longitude = operator_longitude;
    packet.area_ceiling = area_ceiling;
    packet.area_floor = area_floor;
    packet.area_count = area_count;
    packet.area_radius = area_radius;
    packet.flags = flags;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM, (const char *)&packet, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_MIN_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_CRC);
#endif
}

/**
 * @brief Send a open_drone_id_system message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_open_drone_id_system_send_struct(mavlink_channel_t chan, const mavlink_open_drone_id_system_t* open_drone_id_system)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_open_drone_id_system_send(chan, open_drone_id_system->flags, open_drone_id_system->operator_latitude, open_drone_id_system->operator_longitude, open_drone_id_system->area_count, open_drone_id_system->area_radius, open_drone_id_system->area_ceiling, open_drone_id_system->area_floor);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM, (const char *)open_drone_id_system, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_MIN_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_open_drone_id_system_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t flags, int32_t operator_latitude, int32_t operator_longitude, uint16_t area_count, uint16_t area_radius, float area_ceiling, float area_floor)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_int32_t(buf, 0, operator_latitude);
    _mav_put_int32_t(buf, 4, operator_longitude);
    _mav_put_float(buf, 8, area_ceiling);
    _mav_put_float(buf, 12, area_floor);
    _mav_put_uint16_t(buf, 16, area_count);
    _mav_put_uint16_t(buf, 18, area_radius);
    _mav_put_uint8_t(buf, 20, flags);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM, buf, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_MIN_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_CRC);
#else
    mavlink_open_drone_id_system_t *packet = (mavlink_open_drone_id_system_t *)msgbuf;
    packet->operator_latitude = operator_latitude;
    packet->operator_longitude = operator_longitude;
    packet->area_ceiling = area_ceiling;
    packet->area_floor = area_floor;
    packet->area_count = area_count;
    packet->area_radius = area_radius;
    packet->flags = flags;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM, (const char *)packet, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_MIN_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_CRC);
#endif
}
#endif

#endif

// MESSAGE OPEN_DRONE_ID_SYSTEM UNPACKING


/**
 * @brief Get field flags from open_drone_id_system message
 *
 * @return  Specifies the location source for the operator location.
 */
static inline uint8_t mavlink_msg_open_drone_id_system_get_flags(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  20);
}

/**
 * @brief Get field operator_latitude from open_drone_id_system message
 *
 * @return [degE7] Latitude of the operator. If unknown: 0 deg (both Lat/Lon).
 */
static inline int32_t mavlink_msg_open_drone_id_system_get_operator_latitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  0);
}

/**
 * @brief Get field operator_longitude from open_drone_id_system message
 *
 * @return [degE7] Longitude of the operator. If unknown: 0 deg (both Lat/Lon).
 */
static inline int32_t mavlink_msg_open_drone_id_system_get_operator_longitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  4);
}

/**
 * @brief Get field area_count from open_drone_id_system message
 *
 * @return  Number of aircraft in the area, group or formation (default 1).
 */
static inline uint16_t mavlink_msg_open_drone_id_system_get_area_count(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  16);
}

/**
 * @brief Get field area_radius from open_drone_id_system message
 *
 * @return [m] Radius of the cylindrical area of the group or formation (default 0).
 */
static inline uint16_t mavlink_msg_open_drone_id_system_get_area_radius(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  18);
}

/**
 * @brief Get field area_ceiling from open_drone_id_system message
 *
 * @return [m] Area Operations Ceiling relative to WGS84. If unknown: -1000 m.
 */
static inline float mavlink_msg_open_drone_id_system_get_area_ceiling(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  8);
}

/**
 * @brief Get field area_floor from open_drone_id_system message
 *
 * @return [m] Area Operations Floor relative to WGS84. If unknown: -1000 m.
 */
static inline float mavlink_msg_open_drone_id_system_get_area_floor(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  12);
}

/**
 * @brief Decode a open_drone_id_system message into a struct
 *
 * @param msg The message to decode
 * @param open_drone_id_system C-struct to decode the message contents into
 */
static inline void mavlink_msg_open_drone_id_system_decode(const mavlink_message_t* msg, mavlink_open_drone_id_system_t* open_drone_id_system)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    open_drone_id_system->operator_latitude = mavlink_msg_open_drone_id_system_get_operator_latitude(msg);
    open_drone_id_system->operator_longitude = mavlink_msg_open_drone_id_system_get_operator_longitude(msg);
    open_drone_id_system->area_ceiling = mavlink_msg_open_drone_id_system_get_area_ceiling(msg);
    open_drone_id_system->area_floor = mavlink_msg_open_drone_id_system_get_area_floor(msg);
    open_drone_id_system->area_count = mavlink_msg_open_drone_id_system_get_area_count(msg);
    open_drone_id_system->area_radius = mavlink_msg_open_drone_id_system_get_area_radius(msg);
    open_drone_id_system->flags = mavlink_msg_open_drone_id_system_get_flags(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN? msg->len : MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN;
        memset(open_drone_id_system, 0, MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM_LEN);
    memcpy(open_drone_id_system, _MAV_PAYLOAD(msg), len);
#endif
}
