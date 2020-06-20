#pragma once
// MESSAGE OPENHD_GROUND_TELEMETRY PACKING

#define MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY 1210

MAVPACKED(
typedef struct __mavlink_openhd_ground_telemetry_t {
 uint32_t damaged_block_cnt; /*<  damaged_block_cnt*/
 uint32_t lost_packet_cnt; /*<  lost_packet_cnt*/
 uint32_t received_packet_cnt; /*<  received_packet_cnt*/
 uint32_t kbitrate; /*<  kbitrate*/
 uint32_t kbitrate_measured; /*<  kbitrate_measured*/
 uint32_t kbitrate_set; /*<  kbitrate_set*/
 uint8_t target_system; /*<  system id of the requesting system*/
 uint8_t target_component; /*<  component id of the requesting component*/
}) mavlink_openhd_ground_telemetry_t;

#define MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN 26
#define MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_MIN_LEN 26
#define MAVLINK_MSG_ID_1210_LEN 26
#define MAVLINK_MSG_ID_1210_MIN_LEN 26

#define MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_CRC 234
#define MAVLINK_MSG_ID_1210_CRC 234



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_GROUND_TELEMETRY { \
    1210, \
    "OPENHD_GROUND_TELEMETRY", \
    8, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 24, offsetof(mavlink_openhd_ground_telemetry_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 25, offsetof(mavlink_openhd_ground_telemetry_t, target_component) }, \
         { "damaged_block_cnt", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_openhd_ground_telemetry_t, damaged_block_cnt) }, \
         { "lost_packet_cnt", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_openhd_ground_telemetry_t, lost_packet_cnt) }, \
         { "received_packet_cnt", NULL, MAVLINK_TYPE_UINT32_T, 0, 8, offsetof(mavlink_openhd_ground_telemetry_t, received_packet_cnt) }, \
         { "kbitrate", NULL, MAVLINK_TYPE_UINT32_T, 0, 12, offsetof(mavlink_openhd_ground_telemetry_t, kbitrate) }, \
         { "kbitrate_measured", NULL, MAVLINK_TYPE_UINT32_T, 0, 16, offsetof(mavlink_openhd_ground_telemetry_t, kbitrate_measured) }, \
         { "kbitrate_set", NULL, MAVLINK_TYPE_UINT32_T, 0, 20, offsetof(mavlink_openhd_ground_telemetry_t, kbitrate_set) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_GROUND_TELEMETRY { \
    "OPENHD_GROUND_TELEMETRY", \
    8, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 24, offsetof(mavlink_openhd_ground_telemetry_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 25, offsetof(mavlink_openhd_ground_telemetry_t, target_component) }, \
         { "damaged_block_cnt", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_openhd_ground_telemetry_t, damaged_block_cnt) }, \
         { "lost_packet_cnt", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_openhd_ground_telemetry_t, lost_packet_cnt) }, \
         { "received_packet_cnt", NULL, MAVLINK_TYPE_UINT32_T, 0, 8, offsetof(mavlink_openhd_ground_telemetry_t, received_packet_cnt) }, \
         { "kbitrate", NULL, MAVLINK_TYPE_UINT32_T, 0, 12, offsetof(mavlink_openhd_ground_telemetry_t, kbitrate) }, \
         { "kbitrate_measured", NULL, MAVLINK_TYPE_UINT32_T, 0, 16, offsetof(mavlink_openhd_ground_telemetry_t, kbitrate_measured) }, \
         { "kbitrate_set", NULL, MAVLINK_TYPE_UINT32_T, 0, 20, offsetof(mavlink_openhd_ground_telemetry_t, kbitrate_set) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_ground_telemetry message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param damaged_block_cnt  damaged_block_cnt
 * @param lost_packet_cnt  lost_packet_cnt
 * @param received_packet_cnt  received_packet_cnt
 * @param kbitrate  kbitrate
 * @param kbitrate_measured  kbitrate_measured
 * @param kbitrate_set  kbitrate_set
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_ground_telemetry_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t target_system, uint8_t target_component, uint32_t damaged_block_cnt, uint32_t lost_packet_cnt, uint32_t received_packet_cnt, uint32_t kbitrate, uint32_t kbitrate_measured, uint32_t kbitrate_set)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN];
    _mav_put_uint32_t(buf, 0, damaged_block_cnt);
    _mav_put_uint32_t(buf, 4, lost_packet_cnt);
    _mav_put_uint32_t(buf, 8, received_packet_cnt);
    _mav_put_uint32_t(buf, 12, kbitrate);
    _mav_put_uint32_t(buf, 16, kbitrate_measured);
    _mav_put_uint32_t(buf, 20, kbitrate_set);
    _mav_put_uint8_t(buf, 24, target_system);
    _mav_put_uint8_t(buf, 25, target_component);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN);
#else
    mavlink_openhd_ground_telemetry_t packet;
    packet.damaged_block_cnt = damaged_block_cnt;
    packet.lost_packet_cnt = lost_packet_cnt;
    packet.received_packet_cnt = received_packet_cnt;
    packet.kbitrate = kbitrate;
    packet.kbitrate_measured = kbitrate_measured;
    packet.kbitrate_set = kbitrate_set;
    packet.target_system = target_system;
    packet.target_component = target_component;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_CRC);
}

/**
 * @brief Pack a openhd_ground_telemetry message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param damaged_block_cnt  damaged_block_cnt
 * @param lost_packet_cnt  lost_packet_cnt
 * @param received_packet_cnt  received_packet_cnt
 * @param kbitrate  kbitrate
 * @param kbitrate_measured  kbitrate_measured
 * @param kbitrate_set  kbitrate_set
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_ground_telemetry_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t target_system,uint8_t target_component,uint32_t damaged_block_cnt,uint32_t lost_packet_cnt,uint32_t received_packet_cnt,uint32_t kbitrate,uint32_t kbitrate_measured,uint32_t kbitrate_set)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN];
    _mav_put_uint32_t(buf, 0, damaged_block_cnt);
    _mav_put_uint32_t(buf, 4, lost_packet_cnt);
    _mav_put_uint32_t(buf, 8, received_packet_cnt);
    _mav_put_uint32_t(buf, 12, kbitrate);
    _mav_put_uint32_t(buf, 16, kbitrate_measured);
    _mav_put_uint32_t(buf, 20, kbitrate_set);
    _mav_put_uint8_t(buf, 24, target_system);
    _mav_put_uint8_t(buf, 25, target_component);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN);
#else
    mavlink_openhd_ground_telemetry_t packet;
    packet.damaged_block_cnt = damaged_block_cnt;
    packet.lost_packet_cnt = lost_packet_cnt;
    packet.received_packet_cnt = received_packet_cnt;
    packet.kbitrate = kbitrate;
    packet.kbitrate_measured = kbitrate_measured;
    packet.kbitrate_set = kbitrate_set;
    packet.target_system = target_system;
    packet.target_component = target_component;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_CRC);
}

/**
 * @brief Encode a openhd_ground_telemetry struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_ground_telemetry C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_ground_telemetry_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_ground_telemetry_t* openhd_ground_telemetry)
{
    return mavlink_msg_openhd_ground_telemetry_pack(system_id, component_id, msg, openhd_ground_telemetry->target_system, openhd_ground_telemetry->target_component, openhd_ground_telemetry->damaged_block_cnt, openhd_ground_telemetry->lost_packet_cnt, openhd_ground_telemetry->received_packet_cnt, openhd_ground_telemetry->kbitrate, openhd_ground_telemetry->kbitrate_measured, openhd_ground_telemetry->kbitrate_set);
}

/**
 * @brief Encode a openhd_ground_telemetry struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_ground_telemetry C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_ground_telemetry_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_ground_telemetry_t* openhd_ground_telemetry)
{
    return mavlink_msg_openhd_ground_telemetry_pack_chan(system_id, component_id, chan, msg, openhd_ground_telemetry->target_system, openhd_ground_telemetry->target_component, openhd_ground_telemetry->damaged_block_cnt, openhd_ground_telemetry->lost_packet_cnt, openhd_ground_telemetry->received_packet_cnt, openhd_ground_telemetry->kbitrate, openhd_ground_telemetry->kbitrate_measured, openhd_ground_telemetry->kbitrate_set);
}

/**
 * @brief Send a openhd_ground_telemetry message
 * @param chan MAVLink channel to send the message
 *
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param damaged_block_cnt  damaged_block_cnt
 * @param lost_packet_cnt  lost_packet_cnt
 * @param received_packet_cnt  received_packet_cnt
 * @param kbitrate  kbitrate
 * @param kbitrate_measured  kbitrate_measured
 * @param kbitrate_set  kbitrate_set
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_ground_telemetry_send(mavlink_channel_t chan, uint8_t target_system, uint8_t target_component, uint32_t damaged_block_cnt, uint32_t lost_packet_cnt, uint32_t received_packet_cnt, uint32_t kbitrate, uint32_t kbitrate_measured, uint32_t kbitrate_set)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN];
    _mav_put_uint32_t(buf, 0, damaged_block_cnt);
    _mav_put_uint32_t(buf, 4, lost_packet_cnt);
    _mav_put_uint32_t(buf, 8, received_packet_cnt);
    _mav_put_uint32_t(buf, 12, kbitrate);
    _mav_put_uint32_t(buf, 16, kbitrate_measured);
    _mav_put_uint32_t(buf, 20, kbitrate_set);
    _mav_put_uint8_t(buf, 24, target_system);
    _mav_put_uint8_t(buf, 25, target_component);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY, buf, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_CRC);
#else
    mavlink_openhd_ground_telemetry_t packet;
    packet.damaged_block_cnt = damaged_block_cnt;
    packet.lost_packet_cnt = lost_packet_cnt;
    packet.received_packet_cnt = received_packet_cnt;
    packet.kbitrate = kbitrate;
    packet.kbitrate_measured = kbitrate_measured;
    packet.kbitrate_set = kbitrate_set;
    packet.target_system = target_system;
    packet.target_component = target_component;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_CRC);
#endif
}

/**
 * @brief Send a openhd_ground_telemetry message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_ground_telemetry_send_struct(mavlink_channel_t chan, const mavlink_openhd_ground_telemetry_t* openhd_ground_telemetry)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_ground_telemetry_send(chan, openhd_ground_telemetry->target_system, openhd_ground_telemetry->target_component, openhd_ground_telemetry->damaged_block_cnt, openhd_ground_telemetry->lost_packet_cnt, openhd_ground_telemetry->received_packet_cnt, openhd_ground_telemetry->kbitrate, openhd_ground_telemetry->kbitrate_measured, openhd_ground_telemetry->kbitrate_set);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY, (const char *)openhd_ground_telemetry, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_ground_telemetry_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t target_system, uint8_t target_component, uint32_t damaged_block_cnt, uint32_t lost_packet_cnt, uint32_t received_packet_cnt, uint32_t kbitrate, uint32_t kbitrate_measured, uint32_t kbitrate_set)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint32_t(buf, 0, damaged_block_cnt);
    _mav_put_uint32_t(buf, 4, lost_packet_cnt);
    _mav_put_uint32_t(buf, 8, received_packet_cnt);
    _mav_put_uint32_t(buf, 12, kbitrate);
    _mav_put_uint32_t(buf, 16, kbitrate_measured);
    _mav_put_uint32_t(buf, 20, kbitrate_set);
    _mav_put_uint8_t(buf, 24, target_system);
    _mav_put_uint8_t(buf, 25, target_component);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY, buf, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_CRC);
#else
    mavlink_openhd_ground_telemetry_t *packet = (mavlink_openhd_ground_telemetry_t *)msgbuf;
    packet->damaged_block_cnt = damaged_block_cnt;
    packet->lost_packet_cnt = lost_packet_cnt;
    packet->received_packet_cnt = received_packet_cnt;
    packet->kbitrate = kbitrate;
    packet->kbitrate_measured = kbitrate_measured;
    packet->kbitrate_set = kbitrate_set;
    packet->target_system = target_system;
    packet->target_component = target_component;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY, (const char *)packet, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_GROUND_TELEMETRY UNPACKING


/**
 * @brief Get field target_system from openhd_ground_telemetry message
 *
 * @return  system id of the requesting system
 */
static inline uint8_t mavlink_msg_openhd_ground_telemetry_get_target_system(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  24);
}

/**
 * @brief Get field target_component from openhd_ground_telemetry message
 *
 * @return  component id of the requesting component
 */
static inline uint8_t mavlink_msg_openhd_ground_telemetry_get_target_component(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  25);
}

/**
 * @brief Get field damaged_block_cnt from openhd_ground_telemetry message
 *
 * @return  damaged_block_cnt
 */
static inline uint32_t mavlink_msg_openhd_ground_telemetry_get_damaged_block_cnt(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Get field lost_packet_cnt from openhd_ground_telemetry message
 *
 * @return  lost_packet_cnt
 */
static inline uint32_t mavlink_msg_openhd_ground_telemetry_get_lost_packet_cnt(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  4);
}

/**
 * @brief Get field received_packet_cnt from openhd_ground_telemetry message
 *
 * @return  received_packet_cnt
 */
static inline uint32_t mavlink_msg_openhd_ground_telemetry_get_received_packet_cnt(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  8);
}

/**
 * @brief Get field kbitrate from openhd_ground_telemetry message
 *
 * @return  kbitrate
 */
static inline uint32_t mavlink_msg_openhd_ground_telemetry_get_kbitrate(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  12);
}

/**
 * @brief Get field kbitrate_measured from openhd_ground_telemetry message
 *
 * @return  kbitrate_measured
 */
static inline uint32_t mavlink_msg_openhd_ground_telemetry_get_kbitrate_measured(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  16);
}

/**
 * @brief Get field kbitrate_set from openhd_ground_telemetry message
 *
 * @return  kbitrate_set
 */
static inline uint32_t mavlink_msg_openhd_ground_telemetry_get_kbitrate_set(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  20);
}

/**
 * @brief Decode a openhd_ground_telemetry message into a struct
 *
 * @param msg The message to decode
 * @param openhd_ground_telemetry C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_ground_telemetry_decode(const mavlink_message_t* msg, mavlink_openhd_ground_telemetry_t* openhd_ground_telemetry)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_ground_telemetry->damaged_block_cnt = mavlink_msg_openhd_ground_telemetry_get_damaged_block_cnt(msg);
    openhd_ground_telemetry->lost_packet_cnt = mavlink_msg_openhd_ground_telemetry_get_lost_packet_cnt(msg);
    openhd_ground_telemetry->received_packet_cnt = mavlink_msg_openhd_ground_telemetry_get_received_packet_cnt(msg);
    openhd_ground_telemetry->kbitrate = mavlink_msg_openhd_ground_telemetry_get_kbitrate(msg);
    openhd_ground_telemetry->kbitrate_measured = mavlink_msg_openhd_ground_telemetry_get_kbitrate_measured(msg);
    openhd_ground_telemetry->kbitrate_set = mavlink_msg_openhd_ground_telemetry_get_kbitrate_set(msg);
    openhd_ground_telemetry->target_system = mavlink_msg_openhd_ground_telemetry_get_target_system(msg);
    openhd_ground_telemetry->target_component = mavlink_msg_openhd_ground_telemetry_get_target_component(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN;
        memset(openhd_ground_telemetry, 0, MAVLINK_MSG_ID_OPENHD_GROUND_TELEMETRY_LEN);
    memcpy(openhd_ground_telemetry, _MAV_PAYLOAD(msg), len);
#endif
}
