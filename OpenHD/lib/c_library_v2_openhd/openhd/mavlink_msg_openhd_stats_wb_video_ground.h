#pragma once
// MESSAGE OPENHD_STATS_WB_VIDEO_GROUND PACKING

#define MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND 1215


typedef struct __mavlink_openhd_stats_wb_video_ground_t {
 uint64_t count_blocks_total; /*<  count_blocks_total*/
 uint64_t count_blocks_lost; /*<  count_blocks_lost*/
 uint64_t count_blocks_recovered; /*<  count_blocks_recovered*/
 uint64_t count_fragments_recovered; /*<  count_fragments_recovered*/
 int32_t curr_incoming_bitrate; /*<  todo*/
 int32_t curr_fec_decode_time_avg_ms; /*<  todo*/
 int32_t curr_fec_decode_time_min_ms; /*<  todo*/
 int32_t curr_fec_decode_time_max_ms; /*<  todo*/
 int32_t unused0; /*<  unused0*/
 int32_t unused1; /*<  unused1*/
 uint8_t link_index; /*<  link_index*/
} mavlink_openhd_stats_wb_video_ground_t;

#define MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN 57
#define MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_MIN_LEN 57
#define MAVLINK_MSG_ID_1215_LEN 57
#define MAVLINK_MSG_ID_1215_MIN_LEN 57

#define MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_CRC 61
#define MAVLINK_MSG_ID_1215_CRC 61



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_STATS_WB_VIDEO_GROUND { \
    1215, \
    "OPENHD_STATS_WB_VIDEO_GROUND", \
    11, \
    {  { "link_index", NULL, MAVLINK_TYPE_UINT8_T, 0, 56, offsetof(mavlink_openhd_stats_wb_video_ground_t, link_index) }, \
         { "curr_incoming_bitrate", NULL, MAVLINK_TYPE_INT32_T, 0, 32, offsetof(mavlink_openhd_stats_wb_video_ground_t, curr_incoming_bitrate) }, \
         { "count_blocks_total", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_stats_wb_video_ground_t, count_blocks_total) }, \
         { "count_blocks_lost", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_openhd_stats_wb_video_ground_t, count_blocks_lost) }, \
         { "count_blocks_recovered", NULL, MAVLINK_TYPE_UINT64_T, 0, 16, offsetof(mavlink_openhd_stats_wb_video_ground_t, count_blocks_recovered) }, \
         { "count_fragments_recovered", NULL, MAVLINK_TYPE_UINT64_T, 0, 24, offsetof(mavlink_openhd_stats_wb_video_ground_t, count_fragments_recovered) }, \
         { "curr_fec_decode_time_avg_ms", NULL, MAVLINK_TYPE_INT32_T, 0, 36, offsetof(mavlink_openhd_stats_wb_video_ground_t, curr_fec_decode_time_avg_ms) }, \
         { "curr_fec_decode_time_min_ms", NULL, MAVLINK_TYPE_INT32_T, 0, 40, offsetof(mavlink_openhd_stats_wb_video_ground_t, curr_fec_decode_time_min_ms) }, \
         { "curr_fec_decode_time_max_ms", NULL, MAVLINK_TYPE_INT32_T, 0, 44, offsetof(mavlink_openhd_stats_wb_video_ground_t, curr_fec_decode_time_max_ms) }, \
         { "unused0", NULL, MAVLINK_TYPE_INT32_T, 0, 48, offsetof(mavlink_openhd_stats_wb_video_ground_t, unused0) }, \
         { "unused1", NULL, MAVLINK_TYPE_INT32_T, 0, 52, offsetof(mavlink_openhd_stats_wb_video_ground_t, unused1) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_STATS_WB_VIDEO_GROUND { \
    "OPENHD_STATS_WB_VIDEO_GROUND", \
    11, \
    {  { "link_index", NULL, MAVLINK_TYPE_UINT8_T, 0, 56, offsetof(mavlink_openhd_stats_wb_video_ground_t, link_index) }, \
         { "curr_incoming_bitrate", NULL, MAVLINK_TYPE_INT32_T, 0, 32, offsetof(mavlink_openhd_stats_wb_video_ground_t, curr_incoming_bitrate) }, \
         { "count_blocks_total", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_stats_wb_video_ground_t, count_blocks_total) }, \
         { "count_blocks_lost", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_openhd_stats_wb_video_ground_t, count_blocks_lost) }, \
         { "count_blocks_recovered", NULL, MAVLINK_TYPE_UINT64_T, 0, 16, offsetof(mavlink_openhd_stats_wb_video_ground_t, count_blocks_recovered) }, \
         { "count_fragments_recovered", NULL, MAVLINK_TYPE_UINT64_T, 0, 24, offsetof(mavlink_openhd_stats_wb_video_ground_t, count_fragments_recovered) }, \
         { "curr_fec_decode_time_avg_ms", NULL, MAVLINK_TYPE_INT32_T, 0, 36, offsetof(mavlink_openhd_stats_wb_video_ground_t, curr_fec_decode_time_avg_ms) }, \
         { "curr_fec_decode_time_min_ms", NULL, MAVLINK_TYPE_INT32_T, 0, 40, offsetof(mavlink_openhd_stats_wb_video_ground_t, curr_fec_decode_time_min_ms) }, \
         { "curr_fec_decode_time_max_ms", NULL, MAVLINK_TYPE_INT32_T, 0, 44, offsetof(mavlink_openhd_stats_wb_video_ground_t, curr_fec_decode_time_max_ms) }, \
         { "unused0", NULL, MAVLINK_TYPE_INT32_T, 0, 48, offsetof(mavlink_openhd_stats_wb_video_ground_t, unused0) }, \
         { "unused1", NULL, MAVLINK_TYPE_INT32_T, 0, 52, offsetof(mavlink_openhd_stats_wb_video_ground_t, unused1) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_stats_wb_video_ground message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param link_index  link_index
 * @param curr_incoming_bitrate  todo
 * @param count_blocks_total  count_blocks_total
 * @param count_blocks_lost  count_blocks_lost
 * @param count_blocks_recovered  count_blocks_recovered
 * @param count_fragments_recovered  count_fragments_recovered
 * @param curr_fec_decode_time_avg_ms  todo
 * @param curr_fec_decode_time_min_ms  todo
 * @param curr_fec_decode_time_max_ms  todo
 * @param unused0  unused0
 * @param unused1  unused1
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_stats_wb_video_ground_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t link_index, int32_t curr_incoming_bitrate, uint64_t count_blocks_total, uint64_t count_blocks_lost, uint64_t count_blocks_recovered, uint64_t count_fragments_recovered, int32_t curr_fec_decode_time_avg_ms, int32_t curr_fec_decode_time_min_ms, int32_t curr_fec_decode_time_max_ms, int32_t unused0, int32_t unused1)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN];
    _mav_put_uint64_t(buf, 0, count_blocks_total);
    _mav_put_uint64_t(buf, 8, count_blocks_lost);
    _mav_put_uint64_t(buf, 16, count_blocks_recovered);
    _mav_put_uint64_t(buf, 24, count_fragments_recovered);
    _mav_put_int32_t(buf, 32, curr_incoming_bitrate);
    _mav_put_int32_t(buf, 36, curr_fec_decode_time_avg_ms);
    _mav_put_int32_t(buf, 40, curr_fec_decode_time_min_ms);
    _mav_put_int32_t(buf, 44, curr_fec_decode_time_max_ms);
    _mav_put_int32_t(buf, 48, unused0);
    _mav_put_int32_t(buf, 52, unused1);
    _mav_put_uint8_t(buf, 56, link_index);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN);
#else
    mavlink_openhd_stats_wb_video_ground_t packet;
    packet.count_blocks_total = count_blocks_total;
    packet.count_blocks_lost = count_blocks_lost;
    packet.count_blocks_recovered = count_blocks_recovered;
    packet.count_fragments_recovered = count_fragments_recovered;
    packet.curr_incoming_bitrate = curr_incoming_bitrate;
    packet.curr_fec_decode_time_avg_ms = curr_fec_decode_time_avg_ms;
    packet.curr_fec_decode_time_min_ms = curr_fec_decode_time_min_ms;
    packet.curr_fec_decode_time_max_ms = curr_fec_decode_time_max_ms;
    packet.unused0 = unused0;
    packet.unused1 = unused1;
    packet.link_index = link_index;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_CRC);
}

/**
 * @brief Pack a openhd_stats_wb_video_ground message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param link_index  link_index
 * @param curr_incoming_bitrate  todo
 * @param count_blocks_total  count_blocks_total
 * @param count_blocks_lost  count_blocks_lost
 * @param count_blocks_recovered  count_blocks_recovered
 * @param count_fragments_recovered  count_fragments_recovered
 * @param curr_fec_decode_time_avg_ms  todo
 * @param curr_fec_decode_time_min_ms  todo
 * @param curr_fec_decode_time_max_ms  todo
 * @param unused0  unused0
 * @param unused1  unused1
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_stats_wb_video_ground_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t link_index,int32_t curr_incoming_bitrate,uint64_t count_blocks_total,uint64_t count_blocks_lost,uint64_t count_blocks_recovered,uint64_t count_fragments_recovered,int32_t curr_fec_decode_time_avg_ms,int32_t curr_fec_decode_time_min_ms,int32_t curr_fec_decode_time_max_ms,int32_t unused0,int32_t unused1)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN];
    _mav_put_uint64_t(buf, 0, count_blocks_total);
    _mav_put_uint64_t(buf, 8, count_blocks_lost);
    _mav_put_uint64_t(buf, 16, count_blocks_recovered);
    _mav_put_uint64_t(buf, 24, count_fragments_recovered);
    _mav_put_int32_t(buf, 32, curr_incoming_bitrate);
    _mav_put_int32_t(buf, 36, curr_fec_decode_time_avg_ms);
    _mav_put_int32_t(buf, 40, curr_fec_decode_time_min_ms);
    _mav_put_int32_t(buf, 44, curr_fec_decode_time_max_ms);
    _mav_put_int32_t(buf, 48, unused0);
    _mav_put_int32_t(buf, 52, unused1);
    _mav_put_uint8_t(buf, 56, link_index);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN);
#else
    mavlink_openhd_stats_wb_video_ground_t packet;
    packet.count_blocks_total = count_blocks_total;
    packet.count_blocks_lost = count_blocks_lost;
    packet.count_blocks_recovered = count_blocks_recovered;
    packet.count_fragments_recovered = count_fragments_recovered;
    packet.curr_incoming_bitrate = curr_incoming_bitrate;
    packet.curr_fec_decode_time_avg_ms = curr_fec_decode_time_avg_ms;
    packet.curr_fec_decode_time_min_ms = curr_fec_decode_time_min_ms;
    packet.curr_fec_decode_time_max_ms = curr_fec_decode_time_max_ms;
    packet.unused0 = unused0;
    packet.unused1 = unused1;
    packet.link_index = link_index;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_CRC);
}

/**
 * @brief Encode a openhd_stats_wb_video_ground struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_stats_wb_video_ground C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_stats_wb_video_ground_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_stats_wb_video_ground_t* openhd_stats_wb_video_ground)
{
    return mavlink_msg_openhd_stats_wb_video_ground_pack(system_id, component_id, msg, openhd_stats_wb_video_ground->link_index, openhd_stats_wb_video_ground->curr_incoming_bitrate, openhd_stats_wb_video_ground->count_blocks_total, openhd_stats_wb_video_ground->count_blocks_lost, openhd_stats_wb_video_ground->count_blocks_recovered, openhd_stats_wb_video_ground->count_fragments_recovered, openhd_stats_wb_video_ground->curr_fec_decode_time_avg_ms, openhd_stats_wb_video_ground->curr_fec_decode_time_min_ms, openhd_stats_wb_video_ground->curr_fec_decode_time_max_ms, openhd_stats_wb_video_ground->unused0, openhd_stats_wb_video_ground->unused1);
}

/**
 * @brief Encode a openhd_stats_wb_video_ground struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_stats_wb_video_ground C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_stats_wb_video_ground_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_stats_wb_video_ground_t* openhd_stats_wb_video_ground)
{
    return mavlink_msg_openhd_stats_wb_video_ground_pack_chan(system_id, component_id, chan, msg, openhd_stats_wb_video_ground->link_index, openhd_stats_wb_video_ground->curr_incoming_bitrate, openhd_stats_wb_video_ground->count_blocks_total, openhd_stats_wb_video_ground->count_blocks_lost, openhd_stats_wb_video_ground->count_blocks_recovered, openhd_stats_wb_video_ground->count_fragments_recovered, openhd_stats_wb_video_ground->curr_fec_decode_time_avg_ms, openhd_stats_wb_video_ground->curr_fec_decode_time_min_ms, openhd_stats_wb_video_ground->curr_fec_decode_time_max_ms, openhd_stats_wb_video_ground->unused0, openhd_stats_wb_video_ground->unused1);
}

/**
 * @brief Send a openhd_stats_wb_video_ground message
 * @param chan MAVLink channel to send the message
 *
 * @param link_index  link_index
 * @param curr_incoming_bitrate  todo
 * @param count_blocks_total  count_blocks_total
 * @param count_blocks_lost  count_blocks_lost
 * @param count_blocks_recovered  count_blocks_recovered
 * @param count_fragments_recovered  count_fragments_recovered
 * @param curr_fec_decode_time_avg_ms  todo
 * @param curr_fec_decode_time_min_ms  todo
 * @param curr_fec_decode_time_max_ms  todo
 * @param unused0  unused0
 * @param unused1  unused1
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_stats_wb_video_ground_send(mavlink_channel_t chan, uint8_t link_index, int32_t curr_incoming_bitrate, uint64_t count_blocks_total, uint64_t count_blocks_lost, uint64_t count_blocks_recovered, uint64_t count_fragments_recovered, int32_t curr_fec_decode_time_avg_ms, int32_t curr_fec_decode_time_min_ms, int32_t curr_fec_decode_time_max_ms, int32_t unused0, int32_t unused1)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN];
    _mav_put_uint64_t(buf, 0, count_blocks_total);
    _mav_put_uint64_t(buf, 8, count_blocks_lost);
    _mav_put_uint64_t(buf, 16, count_blocks_recovered);
    _mav_put_uint64_t(buf, 24, count_fragments_recovered);
    _mav_put_int32_t(buf, 32, curr_incoming_bitrate);
    _mav_put_int32_t(buf, 36, curr_fec_decode_time_avg_ms);
    _mav_put_int32_t(buf, 40, curr_fec_decode_time_min_ms);
    _mav_put_int32_t(buf, 44, curr_fec_decode_time_max_ms);
    _mav_put_int32_t(buf, 48, unused0);
    _mav_put_int32_t(buf, 52, unused1);
    _mav_put_uint8_t(buf, 56, link_index);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND, buf, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_CRC);
#else
    mavlink_openhd_stats_wb_video_ground_t packet;
    packet.count_blocks_total = count_blocks_total;
    packet.count_blocks_lost = count_blocks_lost;
    packet.count_blocks_recovered = count_blocks_recovered;
    packet.count_fragments_recovered = count_fragments_recovered;
    packet.curr_incoming_bitrate = curr_incoming_bitrate;
    packet.curr_fec_decode_time_avg_ms = curr_fec_decode_time_avg_ms;
    packet.curr_fec_decode_time_min_ms = curr_fec_decode_time_min_ms;
    packet.curr_fec_decode_time_max_ms = curr_fec_decode_time_max_ms;
    packet.unused0 = unused0;
    packet.unused1 = unused1;
    packet.link_index = link_index;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_CRC);
#endif
}

/**
 * @brief Send a openhd_stats_wb_video_ground message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_stats_wb_video_ground_send_struct(mavlink_channel_t chan, const mavlink_openhd_stats_wb_video_ground_t* openhd_stats_wb_video_ground)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_stats_wb_video_ground_send(chan, openhd_stats_wb_video_ground->link_index, openhd_stats_wb_video_ground->curr_incoming_bitrate, openhd_stats_wb_video_ground->count_blocks_total, openhd_stats_wb_video_ground->count_blocks_lost, openhd_stats_wb_video_ground->count_blocks_recovered, openhd_stats_wb_video_ground->count_fragments_recovered, openhd_stats_wb_video_ground->curr_fec_decode_time_avg_ms, openhd_stats_wb_video_ground->curr_fec_decode_time_min_ms, openhd_stats_wb_video_ground->curr_fec_decode_time_max_ms, openhd_stats_wb_video_ground->unused0, openhd_stats_wb_video_ground->unused1);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND, (const char *)openhd_stats_wb_video_ground, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_stats_wb_video_ground_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t link_index, int32_t curr_incoming_bitrate, uint64_t count_blocks_total, uint64_t count_blocks_lost, uint64_t count_blocks_recovered, uint64_t count_fragments_recovered, int32_t curr_fec_decode_time_avg_ms, int32_t curr_fec_decode_time_min_ms, int32_t curr_fec_decode_time_max_ms, int32_t unused0, int32_t unused1)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint64_t(buf, 0, count_blocks_total);
    _mav_put_uint64_t(buf, 8, count_blocks_lost);
    _mav_put_uint64_t(buf, 16, count_blocks_recovered);
    _mav_put_uint64_t(buf, 24, count_fragments_recovered);
    _mav_put_int32_t(buf, 32, curr_incoming_bitrate);
    _mav_put_int32_t(buf, 36, curr_fec_decode_time_avg_ms);
    _mav_put_int32_t(buf, 40, curr_fec_decode_time_min_ms);
    _mav_put_int32_t(buf, 44, curr_fec_decode_time_max_ms);
    _mav_put_int32_t(buf, 48, unused0);
    _mav_put_int32_t(buf, 52, unused1);
    _mav_put_uint8_t(buf, 56, link_index);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND, buf, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_CRC);
#else
    mavlink_openhd_stats_wb_video_ground_t *packet = (mavlink_openhd_stats_wb_video_ground_t *)msgbuf;
    packet->count_blocks_total = count_blocks_total;
    packet->count_blocks_lost = count_blocks_lost;
    packet->count_blocks_recovered = count_blocks_recovered;
    packet->count_fragments_recovered = count_fragments_recovered;
    packet->curr_incoming_bitrate = curr_incoming_bitrate;
    packet->curr_fec_decode_time_avg_ms = curr_fec_decode_time_avg_ms;
    packet->curr_fec_decode_time_min_ms = curr_fec_decode_time_min_ms;
    packet->curr_fec_decode_time_max_ms = curr_fec_decode_time_max_ms;
    packet->unused0 = unused0;
    packet->unused1 = unused1;
    packet->link_index = link_index;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND, (const char *)packet, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_STATS_WB_VIDEO_GROUND UNPACKING


/**
 * @brief Get field link_index from openhd_stats_wb_video_ground message
 *
 * @return  link_index
 */
static inline uint8_t mavlink_msg_openhd_stats_wb_video_ground_get_link_index(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  56);
}

/**
 * @brief Get field curr_incoming_bitrate from openhd_stats_wb_video_ground message
 *
 * @return  todo
 */
static inline int32_t mavlink_msg_openhd_stats_wb_video_ground_get_curr_incoming_bitrate(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  32);
}

/**
 * @brief Get field count_blocks_total from openhd_stats_wb_video_ground message
 *
 * @return  count_blocks_total
 */
static inline uint64_t mavlink_msg_openhd_stats_wb_video_ground_get_count_blocks_total(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  0);
}

/**
 * @brief Get field count_blocks_lost from openhd_stats_wb_video_ground message
 *
 * @return  count_blocks_lost
 */
static inline uint64_t mavlink_msg_openhd_stats_wb_video_ground_get_count_blocks_lost(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  8);
}

/**
 * @brief Get field count_blocks_recovered from openhd_stats_wb_video_ground message
 *
 * @return  count_blocks_recovered
 */
static inline uint64_t mavlink_msg_openhd_stats_wb_video_ground_get_count_blocks_recovered(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  16);
}

/**
 * @brief Get field count_fragments_recovered from openhd_stats_wb_video_ground message
 *
 * @return  count_fragments_recovered
 */
static inline uint64_t mavlink_msg_openhd_stats_wb_video_ground_get_count_fragments_recovered(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  24);
}

/**
 * @brief Get field curr_fec_decode_time_avg_ms from openhd_stats_wb_video_ground message
 *
 * @return  todo
 */
static inline int32_t mavlink_msg_openhd_stats_wb_video_ground_get_curr_fec_decode_time_avg_ms(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  36);
}

/**
 * @brief Get field curr_fec_decode_time_min_ms from openhd_stats_wb_video_ground message
 *
 * @return  todo
 */
static inline int32_t mavlink_msg_openhd_stats_wb_video_ground_get_curr_fec_decode_time_min_ms(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  40);
}

/**
 * @brief Get field curr_fec_decode_time_max_ms from openhd_stats_wb_video_ground message
 *
 * @return  todo
 */
static inline int32_t mavlink_msg_openhd_stats_wb_video_ground_get_curr_fec_decode_time_max_ms(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  44);
}

/**
 * @brief Get field unused0 from openhd_stats_wb_video_ground message
 *
 * @return  unused0
 */
static inline int32_t mavlink_msg_openhd_stats_wb_video_ground_get_unused0(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  48);
}

/**
 * @brief Get field unused1 from openhd_stats_wb_video_ground message
 *
 * @return  unused1
 */
static inline int32_t mavlink_msg_openhd_stats_wb_video_ground_get_unused1(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  52);
}

/**
 * @brief Decode a openhd_stats_wb_video_ground message into a struct
 *
 * @param msg The message to decode
 * @param openhd_stats_wb_video_ground C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_stats_wb_video_ground_decode(const mavlink_message_t* msg, mavlink_openhd_stats_wb_video_ground_t* openhd_stats_wb_video_ground)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_stats_wb_video_ground->count_blocks_total = mavlink_msg_openhd_stats_wb_video_ground_get_count_blocks_total(msg);
    openhd_stats_wb_video_ground->count_blocks_lost = mavlink_msg_openhd_stats_wb_video_ground_get_count_blocks_lost(msg);
    openhd_stats_wb_video_ground->count_blocks_recovered = mavlink_msg_openhd_stats_wb_video_ground_get_count_blocks_recovered(msg);
    openhd_stats_wb_video_ground->count_fragments_recovered = mavlink_msg_openhd_stats_wb_video_ground_get_count_fragments_recovered(msg);
    openhd_stats_wb_video_ground->curr_incoming_bitrate = mavlink_msg_openhd_stats_wb_video_ground_get_curr_incoming_bitrate(msg);
    openhd_stats_wb_video_ground->curr_fec_decode_time_avg_ms = mavlink_msg_openhd_stats_wb_video_ground_get_curr_fec_decode_time_avg_ms(msg);
    openhd_stats_wb_video_ground->curr_fec_decode_time_min_ms = mavlink_msg_openhd_stats_wb_video_ground_get_curr_fec_decode_time_min_ms(msg);
    openhd_stats_wb_video_ground->curr_fec_decode_time_max_ms = mavlink_msg_openhd_stats_wb_video_ground_get_curr_fec_decode_time_max_ms(msg);
    openhd_stats_wb_video_ground->unused0 = mavlink_msg_openhd_stats_wb_video_ground_get_unused0(msg);
    openhd_stats_wb_video_ground->unused1 = mavlink_msg_openhd_stats_wb_video_ground_get_unused1(msg);
    openhd_stats_wb_video_ground->link_index = mavlink_msg_openhd_stats_wb_video_ground_get_link_index(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN;
        memset(openhd_stats_wb_video_ground, 0, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_LEN);
    memcpy(openhd_stats_wb_video_ground, _MAV_PAYLOAD(msg), len);
#endif
}
