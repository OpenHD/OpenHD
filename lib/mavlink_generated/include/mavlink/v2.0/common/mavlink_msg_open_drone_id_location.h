#pragma once
// MESSAGE OPEN_DRONE_ID_LOCATION PACKING

#define MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION 12901

MAVPACKED(
typedef struct __mavlink_open_drone_id_location_t {
 int32_t latitude; /*< [degE7] Current latitude of the UA (Unmanned Aircraft). If unknown: 0 deg (both Lat/Lon).*/
 int32_t longitude; /*< [degE7] Current longitude of the UA (Unmanned Aircraft). If unknown: 0 deg (both Lat/Lon).*/
 float altitude_barometric; /*< [m] The altitude calculated from the barometric pressue. Reference is against 29.92inHg or 1013.2mb. If unknown: -1000 m.*/
 float altitude_geodetic; /*< [m] The geodetic altitude as defined by WGS84. If unknown: -1000 m.*/
 float height; /*< [m] The current height of the UA (Unmanned Aircraft) above the take-off location or the ground as indicated by height_reference. If unknown: -1000 m.*/
 float timestamp; /*< [s] Seconds after the full hour. Typically the GPS outputs a time of week value in milliseconds. That value can be easily converted for this field using ((float) (time_week_ms % (60*60*1000))) / 1000.*/
 uint16_t direction; /*< [cdeg] Direction over ground (not heading, but direction of movement) in degrees * 100: 0.0 - 359.99 degrees. If unknown: 361.00 degrees.*/
 uint16_t speed_horizontal; /*< [cm/s] Ground speed. Positive only. If unknown: 255.00 m/s. If speed is larger than 254.25 m/s, use 254.25 m/s.*/
 int16_t speed_vertical; /*< [cm/s] The vertical speed. Up is positive. If unknown: 63.00 m/s. If speed is larger than 62.00 m/s, use 62.00 m/s.*/
 uint8_t status; /*<  Indicates whether the Unmanned Aircraft is on the ground or in the air.*/
 uint8_t height_reference; /*<  Indicates the reference point for the height field.*/
 uint8_t horizontal_accuracy; /*<  The accuracy of the horizontal position.*/
 uint8_t vertical_accuracy; /*<  The accuracy of the vertical position.*/
 uint8_t barometer_accuracy; /*<  The accuracy of the barometric altitude.*/
 uint8_t speed_accuracy; /*<  The accuracy of the horizontal and vertical speed.*/
 uint8_t timestamp_accuracy; /*<  The accuracy of the timestamps.*/
}) mavlink_open_drone_id_location_t;

#define MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN 37
#define MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_MIN_LEN 37
#define MAVLINK_MSG_ID_12901_LEN 37
#define MAVLINK_MSG_ID_12901_MIN_LEN 37

#define MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_CRC 16
#define MAVLINK_MSG_ID_12901_CRC 16



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPEN_DRONE_ID_LOCATION { \
    12901, \
    "OPEN_DRONE_ID_LOCATION", \
    16, \
    {  { "status", NULL, MAVLINK_TYPE_UINT8_T, 0, 30, offsetof(mavlink_open_drone_id_location_t, status) }, \
         { "direction", NULL, MAVLINK_TYPE_UINT16_T, 0, 24, offsetof(mavlink_open_drone_id_location_t, direction) }, \
         { "speed_horizontal", NULL, MAVLINK_TYPE_UINT16_T, 0, 26, offsetof(mavlink_open_drone_id_location_t, speed_horizontal) }, \
         { "speed_vertical", NULL, MAVLINK_TYPE_INT16_T, 0, 28, offsetof(mavlink_open_drone_id_location_t, speed_vertical) }, \
         { "latitude", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_open_drone_id_location_t, latitude) }, \
         { "longitude", NULL, MAVLINK_TYPE_INT32_T, 0, 4, offsetof(mavlink_open_drone_id_location_t, longitude) }, \
         { "altitude_barometric", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_open_drone_id_location_t, altitude_barometric) }, \
         { "altitude_geodetic", NULL, MAVLINK_TYPE_FLOAT, 0, 12, offsetof(mavlink_open_drone_id_location_t, altitude_geodetic) }, \
         { "height_reference", NULL, MAVLINK_TYPE_UINT8_T, 0, 31, offsetof(mavlink_open_drone_id_location_t, height_reference) }, \
         { "height", NULL, MAVLINK_TYPE_FLOAT, 0, 16, offsetof(mavlink_open_drone_id_location_t, height) }, \
         { "horizontal_accuracy", NULL, MAVLINK_TYPE_UINT8_T, 0, 32, offsetof(mavlink_open_drone_id_location_t, horizontal_accuracy) }, \
         { "vertical_accuracy", NULL, MAVLINK_TYPE_UINT8_T, 0, 33, offsetof(mavlink_open_drone_id_location_t, vertical_accuracy) }, \
         { "barometer_accuracy", NULL, MAVLINK_TYPE_UINT8_T, 0, 34, offsetof(mavlink_open_drone_id_location_t, barometer_accuracy) }, \
         { "speed_accuracy", NULL, MAVLINK_TYPE_UINT8_T, 0, 35, offsetof(mavlink_open_drone_id_location_t, speed_accuracy) }, \
         { "timestamp", NULL, MAVLINK_TYPE_FLOAT, 0, 20, offsetof(mavlink_open_drone_id_location_t, timestamp) }, \
         { "timestamp_accuracy", NULL, MAVLINK_TYPE_UINT8_T, 0, 36, offsetof(mavlink_open_drone_id_location_t, timestamp_accuracy) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPEN_DRONE_ID_LOCATION { \
    "OPEN_DRONE_ID_LOCATION", \
    16, \
    {  { "status", NULL, MAVLINK_TYPE_UINT8_T, 0, 30, offsetof(mavlink_open_drone_id_location_t, status) }, \
         { "direction", NULL, MAVLINK_TYPE_UINT16_T, 0, 24, offsetof(mavlink_open_drone_id_location_t, direction) }, \
         { "speed_horizontal", NULL, MAVLINK_TYPE_UINT16_T, 0, 26, offsetof(mavlink_open_drone_id_location_t, speed_horizontal) }, \
         { "speed_vertical", NULL, MAVLINK_TYPE_INT16_T, 0, 28, offsetof(mavlink_open_drone_id_location_t, speed_vertical) }, \
         { "latitude", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_open_drone_id_location_t, latitude) }, \
         { "longitude", NULL, MAVLINK_TYPE_INT32_T, 0, 4, offsetof(mavlink_open_drone_id_location_t, longitude) }, \
         { "altitude_barometric", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_open_drone_id_location_t, altitude_barometric) }, \
         { "altitude_geodetic", NULL, MAVLINK_TYPE_FLOAT, 0, 12, offsetof(mavlink_open_drone_id_location_t, altitude_geodetic) }, \
         { "height_reference", NULL, MAVLINK_TYPE_UINT8_T, 0, 31, offsetof(mavlink_open_drone_id_location_t, height_reference) }, \
         { "height", NULL, MAVLINK_TYPE_FLOAT, 0, 16, offsetof(mavlink_open_drone_id_location_t, height) }, \
         { "horizontal_accuracy", NULL, MAVLINK_TYPE_UINT8_T, 0, 32, offsetof(mavlink_open_drone_id_location_t, horizontal_accuracy) }, \
         { "vertical_accuracy", NULL, MAVLINK_TYPE_UINT8_T, 0, 33, offsetof(mavlink_open_drone_id_location_t, vertical_accuracy) }, \
         { "barometer_accuracy", NULL, MAVLINK_TYPE_UINT8_T, 0, 34, offsetof(mavlink_open_drone_id_location_t, barometer_accuracy) }, \
         { "speed_accuracy", NULL, MAVLINK_TYPE_UINT8_T, 0, 35, offsetof(mavlink_open_drone_id_location_t, speed_accuracy) }, \
         { "timestamp", NULL, MAVLINK_TYPE_FLOAT, 0, 20, offsetof(mavlink_open_drone_id_location_t, timestamp) }, \
         { "timestamp_accuracy", NULL, MAVLINK_TYPE_UINT8_T, 0, 36, offsetof(mavlink_open_drone_id_location_t, timestamp_accuracy) }, \
         } \
}
#endif

/**
 * @brief Pack a open_drone_id_location message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param status  Indicates whether the Unmanned Aircraft is on the ground or in the air.
 * @param direction [cdeg] Direction over ground (not heading, but direction of movement) in degrees * 100: 0.0 - 359.99 degrees. If unknown: 361.00 degrees.
 * @param speed_horizontal [cm/s] Ground speed. Positive only. If unknown: 255.00 m/s. If speed is larger than 254.25 m/s, use 254.25 m/s.
 * @param speed_vertical [cm/s] The vertical speed. Up is positive. If unknown: 63.00 m/s. If speed is larger than 62.00 m/s, use 62.00 m/s.
 * @param latitude [degE7] Current latitude of the UA (Unmanned Aircraft). If unknown: 0 deg (both Lat/Lon).
 * @param longitude [degE7] Current longitude of the UA (Unmanned Aircraft). If unknown: 0 deg (both Lat/Lon).
 * @param altitude_barometric [m] The altitude calculated from the barometric pressue. Reference is against 29.92inHg or 1013.2mb. If unknown: -1000 m.
 * @param altitude_geodetic [m] The geodetic altitude as defined by WGS84. If unknown: -1000 m.
 * @param height_reference  Indicates the reference point for the height field.
 * @param height [m] The current height of the UA (Unmanned Aircraft) above the take-off location or the ground as indicated by height_reference. If unknown: -1000 m.
 * @param horizontal_accuracy  The accuracy of the horizontal position.
 * @param vertical_accuracy  The accuracy of the vertical position.
 * @param barometer_accuracy  The accuracy of the barometric altitude.
 * @param speed_accuracy  The accuracy of the horizontal and vertical speed.
 * @param timestamp [s] Seconds after the full hour. Typically the GPS outputs a time of week value in milliseconds. That value can be easily converted for this field using ((float) (time_week_ms % (60*60*1000))) / 1000.
 * @param timestamp_accuracy  The accuracy of the timestamps.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_open_drone_id_location_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t status, uint16_t direction, uint16_t speed_horizontal, int16_t speed_vertical, int32_t latitude, int32_t longitude, float altitude_barometric, float altitude_geodetic, uint8_t height_reference, float height, uint8_t horizontal_accuracy, uint8_t vertical_accuracy, uint8_t barometer_accuracy, uint8_t speed_accuracy, float timestamp, uint8_t timestamp_accuracy)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN];
    _mav_put_int32_t(buf, 0, latitude);
    _mav_put_int32_t(buf, 4, longitude);
    _mav_put_float(buf, 8, altitude_barometric);
    _mav_put_float(buf, 12, altitude_geodetic);
    _mav_put_float(buf, 16, height);
    _mav_put_float(buf, 20, timestamp);
    _mav_put_uint16_t(buf, 24, direction);
    _mav_put_uint16_t(buf, 26, speed_horizontal);
    _mav_put_int16_t(buf, 28, speed_vertical);
    _mav_put_uint8_t(buf, 30, status);
    _mav_put_uint8_t(buf, 31, height_reference);
    _mav_put_uint8_t(buf, 32, horizontal_accuracy);
    _mav_put_uint8_t(buf, 33, vertical_accuracy);
    _mav_put_uint8_t(buf, 34, barometer_accuracy);
    _mav_put_uint8_t(buf, 35, speed_accuracy);
    _mav_put_uint8_t(buf, 36, timestamp_accuracy);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN);
#else
    mavlink_open_drone_id_location_t packet;
    packet.latitude = latitude;
    packet.longitude = longitude;
    packet.altitude_barometric = altitude_barometric;
    packet.altitude_geodetic = altitude_geodetic;
    packet.height = height;
    packet.timestamp = timestamp;
    packet.direction = direction;
    packet.speed_horizontal = speed_horizontal;
    packet.speed_vertical = speed_vertical;
    packet.status = status;
    packet.height_reference = height_reference;
    packet.horizontal_accuracy = horizontal_accuracy;
    packet.vertical_accuracy = vertical_accuracy;
    packet.barometer_accuracy = barometer_accuracy;
    packet.speed_accuracy = speed_accuracy;
    packet.timestamp_accuracy = timestamp_accuracy;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_MIN_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_CRC);
}

/**
 * @brief Pack a open_drone_id_location message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param status  Indicates whether the Unmanned Aircraft is on the ground or in the air.
 * @param direction [cdeg] Direction over ground (not heading, but direction of movement) in degrees * 100: 0.0 - 359.99 degrees. If unknown: 361.00 degrees.
 * @param speed_horizontal [cm/s] Ground speed. Positive only. If unknown: 255.00 m/s. If speed is larger than 254.25 m/s, use 254.25 m/s.
 * @param speed_vertical [cm/s] The vertical speed. Up is positive. If unknown: 63.00 m/s. If speed is larger than 62.00 m/s, use 62.00 m/s.
 * @param latitude [degE7] Current latitude of the UA (Unmanned Aircraft). If unknown: 0 deg (both Lat/Lon).
 * @param longitude [degE7] Current longitude of the UA (Unmanned Aircraft). If unknown: 0 deg (both Lat/Lon).
 * @param altitude_barometric [m] The altitude calculated from the barometric pressue. Reference is against 29.92inHg or 1013.2mb. If unknown: -1000 m.
 * @param altitude_geodetic [m] The geodetic altitude as defined by WGS84. If unknown: -1000 m.
 * @param height_reference  Indicates the reference point for the height field.
 * @param height [m] The current height of the UA (Unmanned Aircraft) above the take-off location or the ground as indicated by height_reference. If unknown: -1000 m.
 * @param horizontal_accuracy  The accuracy of the horizontal position.
 * @param vertical_accuracy  The accuracy of the vertical position.
 * @param barometer_accuracy  The accuracy of the barometric altitude.
 * @param speed_accuracy  The accuracy of the horizontal and vertical speed.
 * @param timestamp [s] Seconds after the full hour. Typically the GPS outputs a time of week value in milliseconds. That value can be easily converted for this field using ((float) (time_week_ms % (60*60*1000))) / 1000.
 * @param timestamp_accuracy  The accuracy of the timestamps.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_open_drone_id_location_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t status,uint16_t direction,uint16_t speed_horizontal,int16_t speed_vertical,int32_t latitude,int32_t longitude,float altitude_barometric,float altitude_geodetic,uint8_t height_reference,float height,uint8_t horizontal_accuracy,uint8_t vertical_accuracy,uint8_t barometer_accuracy,uint8_t speed_accuracy,float timestamp,uint8_t timestamp_accuracy)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN];
    _mav_put_int32_t(buf, 0, latitude);
    _mav_put_int32_t(buf, 4, longitude);
    _mav_put_float(buf, 8, altitude_barometric);
    _mav_put_float(buf, 12, altitude_geodetic);
    _mav_put_float(buf, 16, height);
    _mav_put_float(buf, 20, timestamp);
    _mav_put_uint16_t(buf, 24, direction);
    _mav_put_uint16_t(buf, 26, speed_horizontal);
    _mav_put_int16_t(buf, 28, speed_vertical);
    _mav_put_uint8_t(buf, 30, status);
    _mav_put_uint8_t(buf, 31, height_reference);
    _mav_put_uint8_t(buf, 32, horizontal_accuracy);
    _mav_put_uint8_t(buf, 33, vertical_accuracy);
    _mav_put_uint8_t(buf, 34, barometer_accuracy);
    _mav_put_uint8_t(buf, 35, speed_accuracy);
    _mav_put_uint8_t(buf, 36, timestamp_accuracy);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN);
#else
    mavlink_open_drone_id_location_t packet;
    packet.latitude = latitude;
    packet.longitude = longitude;
    packet.altitude_barometric = altitude_barometric;
    packet.altitude_geodetic = altitude_geodetic;
    packet.height = height;
    packet.timestamp = timestamp;
    packet.direction = direction;
    packet.speed_horizontal = speed_horizontal;
    packet.speed_vertical = speed_vertical;
    packet.status = status;
    packet.height_reference = height_reference;
    packet.horizontal_accuracy = horizontal_accuracy;
    packet.vertical_accuracy = vertical_accuracy;
    packet.barometer_accuracy = barometer_accuracy;
    packet.speed_accuracy = speed_accuracy;
    packet.timestamp_accuracy = timestamp_accuracy;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_MIN_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_CRC);
}

/**
 * @brief Encode a open_drone_id_location struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param open_drone_id_location C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_open_drone_id_location_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_open_drone_id_location_t* open_drone_id_location)
{
    return mavlink_msg_open_drone_id_location_pack(system_id, component_id, msg, open_drone_id_location->status, open_drone_id_location->direction, open_drone_id_location->speed_horizontal, open_drone_id_location->speed_vertical, open_drone_id_location->latitude, open_drone_id_location->longitude, open_drone_id_location->altitude_barometric, open_drone_id_location->altitude_geodetic, open_drone_id_location->height_reference, open_drone_id_location->height, open_drone_id_location->horizontal_accuracy, open_drone_id_location->vertical_accuracy, open_drone_id_location->barometer_accuracy, open_drone_id_location->speed_accuracy, open_drone_id_location->timestamp, open_drone_id_location->timestamp_accuracy);
}

/**
 * @brief Encode a open_drone_id_location struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param open_drone_id_location C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_open_drone_id_location_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_open_drone_id_location_t* open_drone_id_location)
{
    return mavlink_msg_open_drone_id_location_pack_chan(system_id, component_id, chan, msg, open_drone_id_location->status, open_drone_id_location->direction, open_drone_id_location->speed_horizontal, open_drone_id_location->speed_vertical, open_drone_id_location->latitude, open_drone_id_location->longitude, open_drone_id_location->altitude_barometric, open_drone_id_location->altitude_geodetic, open_drone_id_location->height_reference, open_drone_id_location->height, open_drone_id_location->horizontal_accuracy, open_drone_id_location->vertical_accuracy, open_drone_id_location->barometer_accuracy, open_drone_id_location->speed_accuracy, open_drone_id_location->timestamp, open_drone_id_location->timestamp_accuracy);
}

/**
 * @brief Send a open_drone_id_location message
 * @param chan MAVLink channel to send the message
 *
 * @param status  Indicates whether the Unmanned Aircraft is on the ground or in the air.
 * @param direction [cdeg] Direction over ground (not heading, but direction of movement) in degrees * 100: 0.0 - 359.99 degrees. If unknown: 361.00 degrees.
 * @param speed_horizontal [cm/s] Ground speed. Positive only. If unknown: 255.00 m/s. If speed is larger than 254.25 m/s, use 254.25 m/s.
 * @param speed_vertical [cm/s] The vertical speed. Up is positive. If unknown: 63.00 m/s. If speed is larger than 62.00 m/s, use 62.00 m/s.
 * @param latitude [degE7] Current latitude of the UA (Unmanned Aircraft). If unknown: 0 deg (both Lat/Lon).
 * @param longitude [degE7] Current longitude of the UA (Unmanned Aircraft). If unknown: 0 deg (both Lat/Lon).
 * @param altitude_barometric [m] The altitude calculated from the barometric pressue. Reference is against 29.92inHg or 1013.2mb. If unknown: -1000 m.
 * @param altitude_geodetic [m] The geodetic altitude as defined by WGS84. If unknown: -1000 m.
 * @param height_reference  Indicates the reference point for the height field.
 * @param height [m] The current height of the UA (Unmanned Aircraft) above the take-off location or the ground as indicated by height_reference. If unknown: -1000 m.
 * @param horizontal_accuracy  The accuracy of the horizontal position.
 * @param vertical_accuracy  The accuracy of the vertical position.
 * @param barometer_accuracy  The accuracy of the barometric altitude.
 * @param speed_accuracy  The accuracy of the horizontal and vertical speed.
 * @param timestamp [s] Seconds after the full hour. Typically the GPS outputs a time of week value in milliseconds. That value can be easily converted for this field using ((float) (time_week_ms % (60*60*1000))) / 1000.
 * @param timestamp_accuracy  The accuracy of the timestamps.
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_open_drone_id_location_send(mavlink_channel_t chan, uint8_t status, uint16_t direction, uint16_t speed_horizontal, int16_t speed_vertical, int32_t latitude, int32_t longitude, float altitude_barometric, float altitude_geodetic, uint8_t height_reference, float height, uint8_t horizontal_accuracy, uint8_t vertical_accuracy, uint8_t barometer_accuracy, uint8_t speed_accuracy, float timestamp, uint8_t timestamp_accuracy)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN];
    _mav_put_int32_t(buf, 0, latitude);
    _mav_put_int32_t(buf, 4, longitude);
    _mav_put_float(buf, 8, altitude_barometric);
    _mav_put_float(buf, 12, altitude_geodetic);
    _mav_put_float(buf, 16, height);
    _mav_put_float(buf, 20, timestamp);
    _mav_put_uint16_t(buf, 24, direction);
    _mav_put_uint16_t(buf, 26, speed_horizontal);
    _mav_put_int16_t(buf, 28, speed_vertical);
    _mav_put_uint8_t(buf, 30, status);
    _mav_put_uint8_t(buf, 31, height_reference);
    _mav_put_uint8_t(buf, 32, horizontal_accuracy);
    _mav_put_uint8_t(buf, 33, vertical_accuracy);
    _mav_put_uint8_t(buf, 34, barometer_accuracy);
    _mav_put_uint8_t(buf, 35, speed_accuracy);
    _mav_put_uint8_t(buf, 36, timestamp_accuracy);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION, buf, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_MIN_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_CRC);
#else
    mavlink_open_drone_id_location_t packet;
    packet.latitude = latitude;
    packet.longitude = longitude;
    packet.altitude_barometric = altitude_barometric;
    packet.altitude_geodetic = altitude_geodetic;
    packet.height = height;
    packet.timestamp = timestamp;
    packet.direction = direction;
    packet.speed_horizontal = speed_horizontal;
    packet.speed_vertical = speed_vertical;
    packet.status = status;
    packet.height_reference = height_reference;
    packet.horizontal_accuracy = horizontal_accuracy;
    packet.vertical_accuracy = vertical_accuracy;
    packet.barometer_accuracy = barometer_accuracy;
    packet.speed_accuracy = speed_accuracy;
    packet.timestamp_accuracy = timestamp_accuracy;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION, (const char *)&packet, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_MIN_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_CRC);
#endif
}

/**
 * @brief Send a open_drone_id_location message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_open_drone_id_location_send_struct(mavlink_channel_t chan, const mavlink_open_drone_id_location_t* open_drone_id_location)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_open_drone_id_location_send(chan, open_drone_id_location->status, open_drone_id_location->direction, open_drone_id_location->speed_horizontal, open_drone_id_location->speed_vertical, open_drone_id_location->latitude, open_drone_id_location->longitude, open_drone_id_location->altitude_barometric, open_drone_id_location->altitude_geodetic, open_drone_id_location->height_reference, open_drone_id_location->height, open_drone_id_location->horizontal_accuracy, open_drone_id_location->vertical_accuracy, open_drone_id_location->barometer_accuracy, open_drone_id_location->speed_accuracy, open_drone_id_location->timestamp, open_drone_id_location->timestamp_accuracy);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION, (const char *)open_drone_id_location, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_MIN_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_open_drone_id_location_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t status, uint16_t direction, uint16_t speed_horizontal, int16_t speed_vertical, int32_t latitude, int32_t longitude, float altitude_barometric, float altitude_geodetic, uint8_t height_reference, float height, uint8_t horizontal_accuracy, uint8_t vertical_accuracy, uint8_t barometer_accuracy, uint8_t speed_accuracy, float timestamp, uint8_t timestamp_accuracy)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_int32_t(buf, 0, latitude);
    _mav_put_int32_t(buf, 4, longitude);
    _mav_put_float(buf, 8, altitude_barometric);
    _mav_put_float(buf, 12, altitude_geodetic);
    _mav_put_float(buf, 16, height);
    _mav_put_float(buf, 20, timestamp);
    _mav_put_uint16_t(buf, 24, direction);
    _mav_put_uint16_t(buf, 26, speed_horizontal);
    _mav_put_int16_t(buf, 28, speed_vertical);
    _mav_put_uint8_t(buf, 30, status);
    _mav_put_uint8_t(buf, 31, height_reference);
    _mav_put_uint8_t(buf, 32, horizontal_accuracy);
    _mav_put_uint8_t(buf, 33, vertical_accuracy);
    _mav_put_uint8_t(buf, 34, barometer_accuracy);
    _mav_put_uint8_t(buf, 35, speed_accuracy);
    _mav_put_uint8_t(buf, 36, timestamp_accuracy);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION, buf, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_MIN_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_CRC);
#else
    mavlink_open_drone_id_location_t *packet = (mavlink_open_drone_id_location_t *)msgbuf;
    packet->latitude = latitude;
    packet->longitude = longitude;
    packet->altitude_barometric = altitude_barometric;
    packet->altitude_geodetic = altitude_geodetic;
    packet->height = height;
    packet->timestamp = timestamp;
    packet->direction = direction;
    packet->speed_horizontal = speed_horizontal;
    packet->speed_vertical = speed_vertical;
    packet->status = status;
    packet->height_reference = height_reference;
    packet->horizontal_accuracy = horizontal_accuracy;
    packet->vertical_accuracy = vertical_accuracy;
    packet->barometer_accuracy = barometer_accuracy;
    packet->speed_accuracy = speed_accuracy;
    packet->timestamp_accuracy = timestamp_accuracy;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION, (const char *)packet, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_MIN_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_CRC);
#endif
}
#endif

#endif

// MESSAGE OPEN_DRONE_ID_LOCATION UNPACKING


/**
 * @brief Get field status from open_drone_id_location message
 *
 * @return  Indicates whether the Unmanned Aircraft is on the ground or in the air.
 */
static inline uint8_t mavlink_msg_open_drone_id_location_get_status(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  30);
}

/**
 * @brief Get field direction from open_drone_id_location message
 *
 * @return [cdeg] Direction over ground (not heading, but direction of movement) in degrees * 100: 0.0 - 359.99 degrees. If unknown: 361.00 degrees.
 */
static inline uint16_t mavlink_msg_open_drone_id_location_get_direction(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  24);
}

/**
 * @brief Get field speed_horizontal from open_drone_id_location message
 *
 * @return [cm/s] Ground speed. Positive only. If unknown: 255.00 m/s. If speed is larger than 254.25 m/s, use 254.25 m/s.
 */
static inline uint16_t mavlink_msg_open_drone_id_location_get_speed_horizontal(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  26);
}

/**
 * @brief Get field speed_vertical from open_drone_id_location message
 *
 * @return [cm/s] The vertical speed. Up is positive. If unknown: 63.00 m/s. If speed is larger than 62.00 m/s, use 62.00 m/s.
 */
static inline int16_t mavlink_msg_open_drone_id_location_get_speed_vertical(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int16_t(msg,  28);
}

/**
 * @brief Get field latitude from open_drone_id_location message
 *
 * @return [degE7] Current latitude of the UA (Unmanned Aircraft). If unknown: 0 deg (both Lat/Lon).
 */
static inline int32_t mavlink_msg_open_drone_id_location_get_latitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  0);
}

/**
 * @brief Get field longitude from open_drone_id_location message
 *
 * @return [degE7] Current longitude of the UA (Unmanned Aircraft). If unknown: 0 deg (both Lat/Lon).
 */
static inline int32_t mavlink_msg_open_drone_id_location_get_longitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  4);
}

/**
 * @brief Get field altitude_barometric from open_drone_id_location message
 *
 * @return [m] The altitude calculated from the barometric pressue. Reference is against 29.92inHg or 1013.2mb. If unknown: -1000 m.
 */
static inline float mavlink_msg_open_drone_id_location_get_altitude_barometric(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  8);
}

/**
 * @brief Get field altitude_geodetic from open_drone_id_location message
 *
 * @return [m] The geodetic altitude as defined by WGS84. If unknown: -1000 m.
 */
static inline float mavlink_msg_open_drone_id_location_get_altitude_geodetic(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  12);
}

/**
 * @brief Get field height_reference from open_drone_id_location message
 *
 * @return  Indicates the reference point for the height field.
 */
static inline uint8_t mavlink_msg_open_drone_id_location_get_height_reference(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  31);
}

/**
 * @brief Get field height from open_drone_id_location message
 *
 * @return [m] The current height of the UA (Unmanned Aircraft) above the take-off location or the ground as indicated by height_reference. If unknown: -1000 m.
 */
static inline float mavlink_msg_open_drone_id_location_get_height(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  16);
}

/**
 * @brief Get field horizontal_accuracy from open_drone_id_location message
 *
 * @return  The accuracy of the horizontal position.
 */
static inline uint8_t mavlink_msg_open_drone_id_location_get_horizontal_accuracy(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  32);
}

/**
 * @brief Get field vertical_accuracy from open_drone_id_location message
 *
 * @return  The accuracy of the vertical position.
 */
static inline uint8_t mavlink_msg_open_drone_id_location_get_vertical_accuracy(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  33);
}

/**
 * @brief Get field barometer_accuracy from open_drone_id_location message
 *
 * @return  The accuracy of the barometric altitude.
 */
static inline uint8_t mavlink_msg_open_drone_id_location_get_barometer_accuracy(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  34);
}

/**
 * @brief Get field speed_accuracy from open_drone_id_location message
 *
 * @return  The accuracy of the horizontal and vertical speed.
 */
static inline uint8_t mavlink_msg_open_drone_id_location_get_speed_accuracy(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  35);
}

/**
 * @brief Get field timestamp from open_drone_id_location message
 *
 * @return [s] Seconds after the full hour. Typically the GPS outputs a time of week value in milliseconds. That value can be easily converted for this field using ((float) (time_week_ms % (60*60*1000))) / 1000.
 */
static inline float mavlink_msg_open_drone_id_location_get_timestamp(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  20);
}

/**
 * @brief Get field timestamp_accuracy from open_drone_id_location message
 *
 * @return  The accuracy of the timestamps.
 */
static inline uint8_t mavlink_msg_open_drone_id_location_get_timestamp_accuracy(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  36);
}

/**
 * @brief Decode a open_drone_id_location message into a struct
 *
 * @param msg The message to decode
 * @param open_drone_id_location C-struct to decode the message contents into
 */
static inline void mavlink_msg_open_drone_id_location_decode(const mavlink_message_t* msg, mavlink_open_drone_id_location_t* open_drone_id_location)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    open_drone_id_location->latitude = mavlink_msg_open_drone_id_location_get_latitude(msg);
    open_drone_id_location->longitude = mavlink_msg_open_drone_id_location_get_longitude(msg);
    open_drone_id_location->altitude_barometric = mavlink_msg_open_drone_id_location_get_altitude_barometric(msg);
    open_drone_id_location->altitude_geodetic = mavlink_msg_open_drone_id_location_get_altitude_geodetic(msg);
    open_drone_id_location->height = mavlink_msg_open_drone_id_location_get_height(msg);
    open_drone_id_location->timestamp = mavlink_msg_open_drone_id_location_get_timestamp(msg);
    open_drone_id_location->direction = mavlink_msg_open_drone_id_location_get_direction(msg);
    open_drone_id_location->speed_horizontal = mavlink_msg_open_drone_id_location_get_speed_horizontal(msg);
    open_drone_id_location->speed_vertical = mavlink_msg_open_drone_id_location_get_speed_vertical(msg);
    open_drone_id_location->status = mavlink_msg_open_drone_id_location_get_status(msg);
    open_drone_id_location->height_reference = mavlink_msg_open_drone_id_location_get_height_reference(msg);
    open_drone_id_location->horizontal_accuracy = mavlink_msg_open_drone_id_location_get_horizontal_accuracy(msg);
    open_drone_id_location->vertical_accuracy = mavlink_msg_open_drone_id_location_get_vertical_accuracy(msg);
    open_drone_id_location->barometer_accuracy = mavlink_msg_open_drone_id_location_get_barometer_accuracy(msg);
    open_drone_id_location->speed_accuracy = mavlink_msg_open_drone_id_location_get_speed_accuracy(msg);
    open_drone_id_location->timestamp_accuracy = mavlink_msg_open_drone_id_location_get_timestamp_accuracy(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN? msg->len : MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN;
        memset(open_drone_id_location, 0, MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION_LEN);
    memcpy(open_drone_id_location, _MAV_PAYLOAD(msg), len);
#endif
}
