#pragma once
// MESSAGE OPENHD_GPIO_STATE PACKING

#define MAVLINK_MSG_ID_OPENHD_GPIO_STATE 1250

MAVPACKED(
typedef struct __mavlink_openhd_gpio_state_t {
 uint8_t target_system; /*<  system id of the requesting system*/
 uint8_t target_component; /*<  component id of the requesting component*/
 uint8_t pins; /*<  pins*/
}) mavlink_openhd_gpio_state_t;

#define MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN 3
#define MAVLINK_MSG_ID_OPENHD_GPIO_STATE_MIN_LEN 3
#define MAVLINK_MSG_ID_1250_LEN 3
#define MAVLINK_MSG_ID_1250_MIN_LEN 3

#define MAVLINK_MSG_ID_OPENHD_GPIO_STATE_CRC 123
#define MAVLINK_MSG_ID_1250_CRC 123



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_GPIO_STATE { \
    1250, \
    "OPENHD_GPIO_STATE", \
    3, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_openhd_gpio_state_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_openhd_gpio_state_t, target_component) }, \
         { "pins", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_openhd_gpio_state_t, pins) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_GPIO_STATE { \
    "OPENHD_GPIO_STATE", \
    3, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_openhd_gpio_state_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_openhd_gpio_state_t, target_component) }, \
         { "pins", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_openhd_gpio_state_t, pins) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_gpio_state message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param pins  pins
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_gpio_state_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t target_system, uint8_t target_component, uint8_t pins)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN];
    _mav_put_uint8_t(buf, 0, target_system);
    _mav_put_uint8_t(buf, 1, target_component);
    _mav_put_uint8_t(buf, 2, pins);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN);
#else
    mavlink_openhd_gpio_state_t packet;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.pins = pins;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_GPIO_STATE;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_CRC);
}

/**
 * @brief Pack a openhd_gpio_state message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param pins  pins
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_gpio_state_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t target_system,uint8_t target_component,uint8_t pins)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN];
    _mav_put_uint8_t(buf, 0, target_system);
    _mav_put_uint8_t(buf, 1, target_component);
    _mav_put_uint8_t(buf, 2, pins);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN);
#else
    mavlink_openhd_gpio_state_t packet;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.pins = pins;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_GPIO_STATE;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_CRC);
}

/**
 * @brief Encode a openhd_gpio_state struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_gpio_state C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_gpio_state_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_gpio_state_t* openhd_gpio_state)
{
    return mavlink_msg_openhd_gpio_state_pack(system_id, component_id, msg, openhd_gpio_state->target_system, openhd_gpio_state->target_component, openhd_gpio_state->pins);
}

/**
 * @brief Encode a openhd_gpio_state struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_gpio_state C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_gpio_state_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_gpio_state_t* openhd_gpio_state)
{
    return mavlink_msg_openhd_gpio_state_pack_chan(system_id, component_id, chan, msg, openhd_gpio_state->target_system, openhd_gpio_state->target_component, openhd_gpio_state->pins);
}

/**
 * @brief Send a openhd_gpio_state message
 * @param chan MAVLink channel to send the message
 *
 * @param target_system  system id of the requesting system
 * @param target_component  component id of the requesting component
 * @param pins  pins
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_gpio_state_send(mavlink_channel_t chan, uint8_t target_system, uint8_t target_component, uint8_t pins)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN];
    _mav_put_uint8_t(buf, 0, target_system);
    _mav_put_uint8_t(buf, 1, target_component);
    _mav_put_uint8_t(buf, 2, pins);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_GPIO_STATE, buf, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_CRC);
#else
    mavlink_openhd_gpio_state_t packet;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.pins = pins;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_GPIO_STATE, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_CRC);
#endif
}

/**
 * @brief Send a openhd_gpio_state message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_gpio_state_send_struct(mavlink_channel_t chan, const mavlink_openhd_gpio_state_t* openhd_gpio_state)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_gpio_state_send(chan, openhd_gpio_state->target_system, openhd_gpio_state->target_component, openhd_gpio_state->pins);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_GPIO_STATE, (const char *)openhd_gpio_state, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_gpio_state_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t target_system, uint8_t target_component, uint8_t pins)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint8_t(buf, 0, target_system);
    _mav_put_uint8_t(buf, 1, target_component);
    _mav_put_uint8_t(buf, 2, pins);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_GPIO_STATE, buf, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_CRC);
#else
    mavlink_openhd_gpio_state_t *packet = (mavlink_openhd_gpio_state_t *)msgbuf;
    packet->target_system = target_system;
    packet->target_component = target_component;
    packet->pins = pins;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_GPIO_STATE, (const char *)packet, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_MIN_LEN, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_GPIO_STATE UNPACKING


/**
 * @brief Get field target_system from openhd_gpio_state message
 *
 * @return  system id of the requesting system
 */
static inline uint8_t mavlink_msg_openhd_gpio_state_get_target_system(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field target_component from openhd_gpio_state message
 *
 * @return  component id of the requesting component
 */
static inline uint8_t mavlink_msg_openhd_gpio_state_get_target_component(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Get field pins from openhd_gpio_state message
 *
 * @return  pins
 */
static inline uint8_t mavlink_msg_openhd_gpio_state_get_pins(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  2);
}

/**
 * @brief Decode a openhd_gpio_state message into a struct
 *
 * @param msg The message to decode
 * @param openhd_gpio_state C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_gpio_state_decode(const mavlink_message_t* msg, mavlink_openhd_gpio_state_t* openhd_gpio_state)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_gpio_state->target_system = mavlink_msg_openhd_gpio_state_get_target_system(msg);
    openhd_gpio_state->target_component = mavlink_msg_openhd_gpio_state_get_target_component(msg);
    openhd_gpio_state->pins = mavlink_msg_openhd_gpio_state_get_pins(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN;
        memset(openhd_gpio_state, 0, MAVLINK_MSG_ID_OPENHD_GPIO_STATE_LEN);
    memcpy(openhd_gpio_state, _MAV_PAYLOAD(msg), len);
#endif
}
