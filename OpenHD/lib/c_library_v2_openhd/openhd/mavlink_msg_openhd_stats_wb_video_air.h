#pragma once
// MESSAGE OPENHD_STATS_WB_VIDEO_AIR PACKING

#define MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR 1214


typedef struct __mavlink_openhd_stats_wb_video_air_t {
 int32_t curr_recommended_bitrate; /*<  curr_recommended_bitrate*/
 int32_t curr_measured_encoder_bitrate; /*<  curr_measured_encoder_bitrate*/
 int32_t curr_injected_bitrate; /*<  curr_injected_bitrate (+FEC overhead)*/
 int32_t curr_injected_pps; /*<  curr_injected_pps*/
 int32_t curr_dropped_packets; /*<  curr_dropped_packets*/
 int32_t curr_fec_encode_time_avg_ms; /*<  curr_fec_encode_time_avg_ms*/
 int32_t curr_fec_encode_time_min_ms; /*<  curr_fec_encode_time_min_ms*/
 int32_t curr_fec_encode_time_max_ms; /*<  curr_fec_encode_time_max_ms*/
 int32_t unused0; /*<  unused0*/
 int32_t unused1; /*<  unused1*/
 int16_t curr_fec_block_size_avg; /*<  curr_fec_block_size_avg*/
 int16_t curr_fec_block_size_min; /*<  curr_fec_block_size_min*/
 int16_t curr_fec_block_size_max; /*<  curr_fec_block_size_max*/
 uint8_t link_index; /*<  link_index*/
 uint8_t curr_video_codec; /*<  curr_video_codec*/
} mavlink_openhd_stats_wb_video_air_t;

#define MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN 48
#define MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_MIN_LEN 48
#define MAVLINK_MSG_ID_1214_LEN 48
#define MAVLINK_MSG_ID_1214_MIN_LEN 48

#define MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_CRC 213
#define MAVLINK_MSG_ID_1214_CRC 213



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_STATS_WB_VIDEO_AIR { \
    1214, \
    "OPENHD_STATS_WB_VIDEO_AIR", \
    15, \
    {  { "link_index", NULL, MAVLINK_TYPE_UINT8_T, 0, 46, offsetof(mavlink_openhd_stats_wb_video_air_t, link_index) }, \
         { "curr_video_codec", NULL, MAVLINK_TYPE_UINT8_T, 0, 47, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_video_codec) }, \
         { "curr_recommended_bitrate", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_recommended_bitrate) }, \
         { "curr_measured_encoder_bitrate", NULL, MAVLINK_TYPE_INT32_T, 0, 4, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_measured_encoder_bitrate) }, \
         { "curr_injected_bitrate", NULL, MAVLINK_TYPE_INT32_T, 0, 8, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_injected_bitrate) }, \
         { "curr_injected_pps", NULL, MAVLINK_TYPE_INT32_T, 0, 12, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_injected_pps) }, \
         { "curr_dropped_packets", NULL, MAVLINK_TYPE_INT32_T, 0, 16, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_dropped_packets) }, \
         { "curr_fec_encode_time_avg_ms", NULL, MAVLINK_TYPE_INT32_T, 0, 20, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_fec_encode_time_avg_ms) }, \
         { "curr_fec_encode_time_min_ms", NULL, MAVLINK_TYPE_INT32_T, 0, 24, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_fec_encode_time_min_ms) }, \
         { "curr_fec_encode_time_max_ms", NULL, MAVLINK_TYPE_INT32_T, 0, 28, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_fec_encode_time_max_ms) }, \
         { "curr_fec_block_size_avg", NULL, MAVLINK_TYPE_INT16_T, 0, 40, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_fec_block_size_avg) }, \
         { "curr_fec_block_size_min", NULL, MAVLINK_TYPE_INT16_T, 0, 42, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_fec_block_size_min) }, \
         { "curr_fec_block_size_max", NULL, MAVLINK_TYPE_INT16_T, 0, 44, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_fec_block_size_max) }, \
         { "unused0", NULL, MAVLINK_TYPE_INT32_T, 0, 32, offsetof(mavlink_openhd_stats_wb_video_air_t, unused0) }, \
         { "unused1", NULL, MAVLINK_TYPE_INT32_T, 0, 36, offsetof(mavlink_openhd_stats_wb_video_air_t, unused1) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_STATS_WB_VIDEO_AIR { \
    "OPENHD_STATS_WB_VIDEO_AIR", \
    15, \
    {  { "link_index", NULL, MAVLINK_TYPE_UINT8_T, 0, 46, offsetof(mavlink_openhd_stats_wb_video_air_t, link_index) }, \
         { "curr_video_codec", NULL, MAVLINK_TYPE_UINT8_T, 0, 47, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_video_codec) }, \
         { "curr_recommended_bitrate", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_recommended_bitrate) }, \
         { "curr_measured_encoder_bitrate", NULL, MAVLINK_TYPE_INT32_T, 0, 4, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_measured_encoder_bitrate) }, \
         { "curr_injected_bitrate", NULL, MAVLINK_TYPE_INT32_T, 0, 8, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_injected_bitrate) }, \
         { "curr_injected_pps", NULL, MAVLINK_TYPE_INT32_T, 0, 12, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_injected_pps) }, \
         { "curr_dropped_packets", NULL, MAVLINK_TYPE_INT32_T, 0, 16, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_dropped_packets) }, \
         { "curr_fec_encode_time_avg_ms", NULL, MAVLINK_TYPE_INT32_T, 0, 20, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_fec_encode_time_avg_ms) }, \
         { "curr_fec_encode_time_min_ms", NULL, MAVLINK_TYPE_INT32_T, 0, 24, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_fec_encode_time_min_ms) }, \
         { "curr_fec_encode_time_max_ms", NULL, MAVLINK_TYPE_INT32_T, 0, 28, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_fec_encode_time_max_ms) }, \
         { "curr_fec_block_size_avg", NULL, MAVLINK_TYPE_INT16_T, 0, 40, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_fec_block_size_avg) }, \
         { "curr_fec_block_size_min", NULL, MAVLINK_TYPE_INT16_T, 0, 42, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_fec_block_size_min) }, \
         { "curr_fec_block_size_max", NULL, MAVLINK_TYPE_INT16_T, 0, 44, offsetof(mavlink_openhd_stats_wb_video_air_t, curr_fec_block_size_max) }, \
         { "unused0", NULL, MAVLINK_TYPE_INT32_T, 0, 32, offsetof(mavlink_openhd_stats_wb_video_air_t, unused0) }, \
         { "unused1", NULL, MAVLINK_TYPE_INT32_T, 0, 36, offsetof(mavlink_openhd_stats_wb_video_air_t, unused1) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_stats_wb_video_air message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param link_index  link_index
 * @param curr_video_codec  curr_video_codec
 * @param curr_recommended_bitrate  curr_recommended_bitrate
 * @param curr_measured_encoder_bitrate  curr_measured_encoder_bitrate
 * @param curr_injected_bitrate  curr_injected_bitrate (+FEC overhead)
 * @param curr_injected_pps  curr_injected_pps
 * @param curr_dropped_packets  curr_dropped_packets
 * @param curr_fec_encode_time_avg_ms  curr_fec_encode_time_avg_ms
 * @param curr_fec_encode_time_min_ms  curr_fec_encode_time_min_ms
 * @param curr_fec_encode_time_max_ms  curr_fec_encode_time_max_ms
 * @param curr_fec_block_size_avg  curr_fec_block_size_avg
 * @param curr_fec_block_size_min  curr_fec_block_size_min
 * @param curr_fec_block_size_max  curr_fec_block_size_max
 * @param unused0  unused0
 * @param unused1  unused1
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_stats_wb_video_air_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t link_index, uint8_t curr_video_codec, int32_t curr_recommended_bitrate, int32_t curr_measured_encoder_bitrate, int32_t curr_injected_bitrate, int32_t curr_injected_pps, int32_t curr_dropped_packets, int32_t curr_fec_encode_time_avg_ms, int32_t curr_fec_encode_time_min_ms, int32_t curr_fec_encode_time_max_ms, int16_t curr_fec_block_size_avg, int16_t curr_fec_block_size_min, int16_t curr_fec_block_size_max, int32_t unused0, int32_t unused1)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN];
    _mav_put_int32_t(buf, 0, curr_recommended_bitrate);
    _mav_put_int32_t(buf, 4, curr_measured_encoder_bitrate);
    _mav_put_int32_t(buf, 8, curr_injected_bitrate);
    _mav_put_int32_t(buf, 12, curr_injected_pps);
    _mav_put_int32_t(buf, 16, curr_dropped_packets);
    _mav_put_int32_t(buf, 20, curr_fec_encode_time_avg_ms);
    _mav_put_int32_t(buf, 24, curr_fec_encode_time_min_ms);
    _mav_put_int32_t(buf, 28, curr_fec_encode_time_max_ms);
    _mav_put_int32_t(buf, 32, unused0);
    _mav_put_int32_t(buf, 36, unused1);
    _mav_put_int16_t(buf, 40, curr_fec_block_size_avg);
    _mav_put_int16_t(buf, 42, curr_fec_block_size_min);
    _mav_put_int16_t(buf, 44, curr_fec_block_size_max);
    _mav_put_uint8_t(buf, 46, link_index);
    _mav_put_uint8_t(buf, 47, curr_video_codec);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN);
#else
    mavlink_openhd_stats_wb_video_air_t packet;
    packet.curr_recommended_bitrate = curr_recommended_bitrate;
    packet.curr_measured_encoder_bitrate = curr_measured_encoder_bitrate;
    packet.curr_injected_bitrate = curr_injected_bitrate;
    packet.curr_injected_pps = curr_injected_pps;
    packet.curr_dropped_packets = curr_dropped_packets;
    packet.curr_fec_encode_time_avg_ms = curr_fec_encode_time_avg_ms;
    packet.curr_fec_encode_time_min_ms = curr_fec_encode_time_min_ms;
    packet.curr_fec_encode_time_max_ms = curr_fec_encode_time_max_ms;
    packet.unused0 = unused0;
    packet.unused1 = unused1;
    packet.curr_fec_block_size_avg = curr_fec_block_size_avg;
    packet.curr_fec_block_size_min = curr_fec_block_size_min;
    packet.curr_fec_block_size_max = curr_fec_block_size_max;
    packet.link_index = link_index;
    packet.curr_video_codec = curr_video_codec;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_CRC);
}

/**
 * @brief Pack a openhd_stats_wb_video_air message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param link_index  link_index
 * @param curr_video_codec  curr_video_codec
 * @param curr_recommended_bitrate  curr_recommended_bitrate
 * @param curr_measured_encoder_bitrate  curr_measured_encoder_bitrate
 * @param curr_injected_bitrate  curr_injected_bitrate (+FEC overhead)
 * @param curr_injected_pps  curr_injected_pps
 * @param curr_dropped_packets  curr_dropped_packets
 * @param curr_fec_encode_time_avg_ms  curr_fec_encode_time_avg_ms
 * @param curr_fec_encode_time_min_ms  curr_fec_encode_time_min_ms
 * @param curr_fec_encode_time_max_ms  curr_fec_encode_time_max_ms
 * @param curr_fec_block_size_avg  curr_fec_block_size_avg
 * @param curr_fec_block_size_min  curr_fec_block_size_min
 * @param curr_fec_block_size_max  curr_fec_block_size_max
 * @param unused0  unused0
 * @param unused1  unused1
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_stats_wb_video_air_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t link_index,uint8_t curr_video_codec,int32_t curr_recommended_bitrate,int32_t curr_measured_encoder_bitrate,int32_t curr_injected_bitrate,int32_t curr_injected_pps,int32_t curr_dropped_packets,int32_t curr_fec_encode_time_avg_ms,int32_t curr_fec_encode_time_min_ms,int32_t curr_fec_encode_time_max_ms,int16_t curr_fec_block_size_avg,int16_t curr_fec_block_size_min,int16_t curr_fec_block_size_max,int32_t unused0,int32_t unused1)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN];
    _mav_put_int32_t(buf, 0, curr_recommended_bitrate);
    _mav_put_int32_t(buf, 4, curr_measured_encoder_bitrate);
    _mav_put_int32_t(buf, 8, curr_injected_bitrate);
    _mav_put_int32_t(buf, 12, curr_injected_pps);
    _mav_put_int32_t(buf, 16, curr_dropped_packets);
    _mav_put_int32_t(buf, 20, curr_fec_encode_time_avg_ms);
    _mav_put_int32_t(buf, 24, curr_fec_encode_time_min_ms);
    _mav_put_int32_t(buf, 28, curr_fec_encode_time_max_ms);
    _mav_put_int32_t(buf, 32, unused0);
    _mav_put_int32_t(buf, 36, unused1);
    _mav_put_int16_t(buf, 40, curr_fec_block_size_avg);
    _mav_put_int16_t(buf, 42, curr_fec_block_size_min);
    _mav_put_int16_t(buf, 44, curr_fec_block_size_max);
    _mav_put_uint8_t(buf, 46, link_index);
    _mav_put_uint8_t(buf, 47, curr_video_codec);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN);
#else
    mavlink_openhd_stats_wb_video_air_t packet;
    packet.curr_recommended_bitrate = curr_recommended_bitrate;
    packet.curr_measured_encoder_bitrate = curr_measured_encoder_bitrate;
    packet.curr_injected_bitrate = curr_injected_bitrate;
    packet.curr_injected_pps = curr_injected_pps;
    packet.curr_dropped_packets = curr_dropped_packets;
    packet.curr_fec_encode_time_avg_ms = curr_fec_encode_time_avg_ms;
    packet.curr_fec_encode_time_min_ms = curr_fec_encode_time_min_ms;
    packet.curr_fec_encode_time_max_ms = curr_fec_encode_time_max_ms;
    packet.unused0 = unused0;
    packet.unused1 = unused1;
    packet.curr_fec_block_size_avg = curr_fec_block_size_avg;
    packet.curr_fec_block_size_min = curr_fec_block_size_min;
    packet.curr_fec_block_size_max = curr_fec_block_size_max;
    packet.link_index = link_index;
    packet.curr_video_codec = curr_video_codec;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_CRC);
}

/**
 * @brief Encode a openhd_stats_wb_video_air struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_stats_wb_video_air C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_stats_wb_video_air_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_stats_wb_video_air_t* openhd_stats_wb_video_air)
{
    return mavlink_msg_openhd_stats_wb_video_air_pack(system_id, component_id, msg, openhd_stats_wb_video_air->link_index, openhd_stats_wb_video_air->curr_video_codec, openhd_stats_wb_video_air->curr_recommended_bitrate, openhd_stats_wb_video_air->curr_measured_encoder_bitrate, openhd_stats_wb_video_air->curr_injected_bitrate, openhd_stats_wb_video_air->curr_injected_pps, openhd_stats_wb_video_air->curr_dropped_packets, openhd_stats_wb_video_air->curr_fec_encode_time_avg_ms, openhd_stats_wb_video_air->curr_fec_encode_time_min_ms, openhd_stats_wb_video_air->curr_fec_encode_time_max_ms, openhd_stats_wb_video_air->curr_fec_block_size_avg, openhd_stats_wb_video_air->curr_fec_block_size_min, openhd_stats_wb_video_air->curr_fec_block_size_max, openhd_stats_wb_video_air->unused0, openhd_stats_wb_video_air->unused1);
}

/**
 * @brief Encode a openhd_stats_wb_video_air struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_stats_wb_video_air C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_stats_wb_video_air_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_stats_wb_video_air_t* openhd_stats_wb_video_air)
{
    return mavlink_msg_openhd_stats_wb_video_air_pack_chan(system_id, component_id, chan, msg, openhd_stats_wb_video_air->link_index, openhd_stats_wb_video_air->curr_video_codec, openhd_stats_wb_video_air->curr_recommended_bitrate, openhd_stats_wb_video_air->curr_measured_encoder_bitrate, openhd_stats_wb_video_air->curr_injected_bitrate, openhd_stats_wb_video_air->curr_injected_pps, openhd_stats_wb_video_air->curr_dropped_packets, openhd_stats_wb_video_air->curr_fec_encode_time_avg_ms, openhd_stats_wb_video_air->curr_fec_encode_time_min_ms, openhd_stats_wb_video_air->curr_fec_encode_time_max_ms, openhd_stats_wb_video_air->curr_fec_block_size_avg, openhd_stats_wb_video_air->curr_fec_block_size_min, openhd_stats_wb_video_air->curr_fec_block_size_max, openhd_stats_wb_video_air->unused0, openhd_stats_wb_video_air->unused1);
}

/**
 * @brief Send a openhd_stats_wb_video_air message
 * @param chan MAVLink channel to send the message
 *
 * @param link_index  link_index
 * @param curr_video_codec  curr_video_codec
 * @param curr_recommended_bitrate  curr_recommended_bitrate
 * @param curr_measured_encoder_bitrate  curr_measured_encoder_bitrate
 * @param curr_injected_bitrate  curr_injected_bitrate (+FEC overhead)
 * @param curr_injected_pps  curr_injected_pps
 * @param curr_dropped_packets  curr_dropped_packets
 * @param curr_fec_encode_time_avg_ms  curr_fec_encode_time_avg_ms
 * @param curr_fec_encode_time_min_ms  curr_fec_encode_time_min_ms
 * @param curr_fec_encode_time_max_ms  curr_fec_encode_time_max_ms
 * @param curr_fec_block_size_avg  curr_fec_block_size_avg
 * @param curr_fec_block_size_min  curr_fec_block_size_min
 * @param curr_fec_block_size_max  curr_fec_block_size_max
 * @param unused0  unused0
 * @param unused1  unused1
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_stats_wb_video_air_send(mavlink_channel_t chan, uint8_t link_index, uint8_t curr_video_codec, int32_t curr_recommended_bitrate, int32_t curr_measured_encoder_bitrate, int32_t curr_injected_bitrate, int32_t curr_injected_pps, int32_t curr_dropped_packets, int32_t curr_fec_encode_time_avg_ms, int32_t curr_fec_encode_time_min_ms, int32_t curr_fec_encode_time_max_ms, int16_t curr_fec_block_size_avg, int16_t curr_fec_block_size_min, int16_t curr_fec_block_size_max, int32_t unused0, int32_t unused1)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN];
    _mav_put_int32_t(buf, 0, curr_recommended_bitrate);
    _mav_put_int32_t(buf, 4, curr_measured_encoder_bitrate);
    _mav_put_int32_t(buf, 8, curr_injected_bitrate);
    _mav_put_int32_t(buf, 12, curr_injected_pps);
    _mav_put_int32_t(buf, 16, curr_dropped_packets);
    _mav_put_int32_t(buf, 20, curr_fec_encode_time_avg_ms);
    _mav_put_int32_t(buf, 24, curr_fec_encode_time_min_ms);
    _mav_put_int32_t(buf, 28, curr_fec_encode_time_max_ms);
    _mav_put_int32_t(buf, 32, unused0);
    _mav_put_int32_t(buf, 36, unused1);
    _mav_put_int16_t(buf, 40, curr_fec_block_size_avg);
    _mav_put_int16_t(buf, 42, curr_fec_block_size_min);
    _mav_put_int16_t(buf, 44, curr_fec_block_size_max);
    _mav_put_uint8_t(buf, 46, link_index);
    _mav_put_uint8_t(buf, 47, curr_video_codec);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR, buf, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_CRC);
#else
    mavlink_openhd_stats_wb_video_air_t packet;
    packet.curr_recommended_bitrate = curr_recommended_bitrate;
    packet.curr_measured_encoder_bitrate = curr_measured_encoder_bitrate;
    packet.curr_injected_bitrate = curr_injected_bitrate;
    packet.curr_injected_pps = curr_injected_pps;
    packet.curr_dropped_packets = curr_dropped_packets;
    packet.curr_fec_encode_time_avg_ms = curr_fec_encode_time_avg_ms;
    packet.curr_fec_encode_time_min_ms = curr_fec_encode_time_min_ms;
    packet.curr_fec_encode_time_max_ms = curr_fec_encode_time_max_ms;
    packet.unused0 = unused0;
    packet.unused1 = unused1;
    packet.curr_fec_block_size_avg = curr_fec_block_size_avg;
    packet.curr_fec_block_size_min = curr_fec_block_size_min;
    packet.curr_fec_block_size_max = curr_fec_block_size_max;
    packet.link_index = link_index;
    packet.curr_video_codec = curr_video_codec;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_CRC);
#endif
}

/**
 * @brief Send a openhd_stats_wb_video_air message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_stats_wb_video_air_send_struct(mavlink_channel_t chan, const mavlink_openhd_stats_wb_video_air_t* openhd_stats_wb_video_air)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_stats_wb_video_air_send(chan, openhd_stats_wb_video_air->link_index, openhd_stats_wb_video_air->curr_video_codec, openhd_stats_wb_video_air->curr_recommended_bitrate, openhd_stats_wb_video_air->curr_measured_encoder_bitrate, openhd_stats_wb_video_air->curr_injected_bitrate, openhd_stats_wb_video_air->curr_injected_pps, openhd_stats_wb_video_air->curr_dropped_packets, openhd_stats_wb_video_air->curr_fec_encode_time_avg_ms, openhd_stats_wb_video_air->curr_fec_encode_time_min_ms, openhd_stats_wb_video_air->curr_fec_encode_time_max_ms, openhd_stats_wb_video_air->curr_fec_block_size_avg, openhd_stats_wb_video_air->curr_fec_block_size_min, openhd_stats_wb_video_air->curr_fec_block_size_max, openhd_stats_wb_video_air->unused0, openhd_stats_wb_video_air->unused1);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR, (const char *)openhd_stats_wb_video_air, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_stats_wb_video_air_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t link_index, uint8_t curr_video_codec, int32_t curr_recommended_bitrate, int32_t curr_measured_encoder_bitrate, int32_t curr_injected_bitrate, int32_t curr_injected_pps, int32_t curr_dropped_packets, int32_t curr_fec_encode_time_avg_ms, int32_t curr_fec_encode_time_min_ms, int32_t curr_fec_encode_time_max_ms, int16_t curr_fec_block_size_avg, int16_t curr_fec_block_size_min, int16_t curr_fec_block_size_max, int32_t unused0, int32_t unused1)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_int32_t(buf, 0, curr_recommended_bitrate);
    _mav_put_int32_t(buf, 4, curr_measured_encoder_bitrate);
    _mav_put_int32_t(buf, 8, curr_injected_bitrate);
    _mav_put_int32_t(buf, 12, curr_injected_pps);
    _mav_put_int32_t(buf, 16, curr_dropped_packets);
    _mav_put_int32_t(buf, 20, curr_fec_encode_time_avg_ms);
    _mav_put_int32_t(buf, 24, curr_fec_encode_time_min_ms);
    _mav_put_int32_t(buf, 28, curr_fec_encode_time_max_ms);
    _mav_put_int32_t(buf, 32, unused0);
    _mav_put_int32_t(buf, 36, unused1);
    _mav_put_int16_t(buf, 40, curr_fec_block_size_avg);
    _mav_put_int16_t(buf, 42, curr_fec_block_size_min);
    _mav_put_int16_t(buf, 44, curr_fec_block_size_max);
    _mav_put_uint8_t(buf, 46, link_index);
    _mav_put_uint8_t(buf, 47, curr_video_codec);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR, buf, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_CRC);
#else
    mavlink_openhd_stats_wb_video_air_t *packet = (mavlink_openhd_stats_wb_video_air_t *)msgbuf;
    packet->curr_recommended_bitrate = curr_recommended_bitrate;
    packet->curr_measured_encoder_bitrate = curr_measured_encoder_bitrate;
    packet->curr_injected_bitrate = curr_injected_bitrate;
    packet->curr_injected_pps = curr_injected_pps;
    packet->curr_dropped_packets = curr_dropped_packets;
    packet->curr_fec_encode_time_avg_ms = curr_fec_encode_time_avg_ms;
    packet->curr_fec_encode_time_min_ms = curr_fec_encode_time_min_ms;
    packet->curr_fec_encode_time_max_ms = curr_fec_encode_time_max_ms;
    packet->unused0 = unused0;
    packet->unused1 = unused1;
    packet->curr_fec_block_size_avg = curr_fec_block_size_avg;
    packet->curr_fec_block_size_min = curr_fec_block_size_min;
    packet->curr_fec_block_size_max = curr_fec_block_size_max;
    packet->link_index = link_index;
    packet->curr_video_codec = curr_video_codec;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR, (const char *)packet, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_STATS_WB_VIDEO_AIR UNPACKING


/**
 * @brief Get field link_index from openhd_stats_wb_video_air message
 *
 * @return  link_index
 */
static inline uint8_t mavlink_msg_openhd_stats_wb_video_air_get_link_index(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  46);
}

/**
 * @brief Get field curr_video_codec from openhd_stats_wb_video_air message
 *
 * @return  curr_video_codec
 */
static inline uint8_t mavlink_msg_openhd_stats_wb_video_air_get_curr_video_codec(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  47);
}

/**
 * @brief Get field curr_recommended_bitrate from openhd_stats_wb_video_air message
 *
 * @return  curr_recommended_bitrate
 */
static inline int32_t mavlink_msg_openhd_stats_wb_video_air_get_curr_recommended_bitrate(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  0);
}

/**
 * @brief Get field curr_measured_encoder_bitrate from openhd_stats_wb_video_air message
 *
 * @return  curr_measured_encoder_bitrate
 */
static inline int32_t mavlink_msg_openhd_stats_wb_video_air_get_curr_measured_encoder_bitrate(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  4);
}

/**
 * @brief Get field curr_injected_bitrate from openhd_stats_wb_video_air message
 *
 * @return  curr_injected_bitrate (+FEC overhead)
 */
static inline int32_t mavlink_msg_openhd_stats_wb_video_air_get_curr_injected_bitrate(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  8);
}

/**
 * @brief Get field curr_injected_pps from openhd_stats_wb_video_air message
 *
 * @return  curr_injected_pps
 */
static inline int32_t mavlink_msg_openhd_stats_wb_video_air_get_curr_injected_pps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  12);
}

/**
 * @brief Get field curr_dropped_packets from openhd_stats_wb_video_air message
 *
 * @return  curr_dropped_packets
 */
static inline int32_t mavlink_msg_openhd_stats_wb_video_air_get_curr_dropped_packets(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  16);
}

/**
 * @brief Get field curr_fec_encode_time_avg_ms from openhd_stats_wb_video_air message
 *
 * @return  curr_fec_encode_time_avg_ms
 */
static inline int32_t mavlink_msg_openhd_stats_wb_video_air_get_curr_fec_encode_time_avg_ms(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  20);
}

/**
 * @brief Get field curr_fec_encode_time_min_ms from openhd_stats_wb_video_air message
 *
 * @return  curr_fec_encode_time_min_ms
 */
static inline int32_t mavlink_msg_openhd_stats_wb_video_air_get_curr_fec_encode_time_min_ms(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  24);
}

/**
 * @brief Get field curr_fec_encode_time_max_ms from openhd_stats_wb_video_air message
 *
 * @return  curr_fec_encode_time_max_ms
 */
static inline int32_t mavlink_msg_openhd_stats_wb_video_air_get_curr_fec_encode_time_max_ms(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  28);
}

/**
 * @brief Get field curr_fec_block_size_avg from openhd_stats_wb_video_air message
 *
 * @return  curr_fec_block_size_avg
 */
static inline int16_t mavlink_msg_openhd_stats_wb_video_air_get_curr_fec_block_size_avg(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int16_t(msg,  40);
}

/**
 * @brief Get field curr_fec_block_size_min from openhd_stats_wb_video_air message
 *
 * @return  curr_fec_block_size_min
 */
static inline int16_t mavlink_msg_openhd_stats_wb_video_air_get_curr_fec_block_size_min(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int16_t(msg,  42);
}

/**
 * @brief Get field curr_fec_block_size_max from openhd_stats_wb_video_air message
 *
 * @return  curr_fec_block_size_max
 */
static inline int16_t mavlink_msg_openhd_stats_wb_video_air_get_curr_fec_block_size_max(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int16_t(msg,  44);
}

/**
 * @brief Get field unused0 from openhd_stats_wb_video_air message
 *
 * @return  unused0
 */
static inline int32_t mavlink_msg_openhd_stats_wb_video_air_get_unused0(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  32);
}

/**
 * @brief Get field unused1 from openhd_stats_wb_video_air message
 *
 * @return  unused1
 */
static inline int32_t mavlink_msg_openhd_stats_wb_video_air_get_unused1(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  36);
}

/**
 * @brief Decode a openhd_stats_wb_video_air message into a struct
 *
 * @param msg The message to decode
 * @param openhd_stats_wb_video_air C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_stats_wb_video_air_decode(const mavlink_message_t* msg, mavlink_openhd_stats_wb_video_air_t* openhd_stats_wb_video_air)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_stats_wb_video_air->curr_recommended_bitrate = mavlink_msg_openhd_stats_wb_video_air_get_curr_recommended_bitrate(msg);
    openhd_stats_wb_video_air->curr_measured_encoder_bitrate = mavlink_msg_openhd_stats_wb_video_air_get_curr_measured_encoder_bitrate(msg);
    openhd_stats_wb_video_air->curr_injected_bitrate = mavlink_msg_openhd_stats_wb_video_air_get_curr_injected_bitrate(msg);
    openhd_stats_wb_video_air->curr_injected_pps = mavlink_msg_openhd_stats_wb_video_air_get_curr_injected_pps(msg);
    openhd_stats_wb_video_air->curr_dropped_packets = mavlink_msg_openhd_stats_wb_video_air_get_curr_dropped_packets(msg);
    openhd_stats_wb_video_air->curr_fec_encode_time_avg_ms = mavlink_msg_openhd_stats_wb_video_air_get_curr_fec_encode_time_avg_ms(msg);
    openhd_stats_wb_video_air->curr_fec_encode_time_min_ms = mavlink_msg_openhd_stats_wb_video_air_get_curr_fec_encode_time_min_ms(msg);
    openhd_stats_wb_video_air->curr_fec_encode_time_max_ms = mavlink_msg_openhd_stats_wb_video_air_get_curr_fec_encode_time_max_ms(msg);
    openhd_stats_wb_video_air->unused0 = mavlink_msg_openhd_stats_wb_video_air_get_unused0(msg);
    openhd_stats_wb_video_air->unused1 = mavlink_msg_openhd_stats_wb_video_air_get_unused1(msg);
    openhd_stats_wb_video_air->curr_fec_block_size_avg = mavlink_msg_openhd_stats_wb_video_air_get_curr_fec_block_size_avg(msg);
    openhd_stats_wb_video_air->curr_fec_block_size_min = mavlink_msg_openhd_stats_wb_video_air_get_curr_fec_block_size_min(msg);
    openhd_stats_wb_video_air->curr_fec_block_size_max = mavlink_msg_openhd_stats_wb_video_air_get_curr_fec_block_size_max(msg);
    openhd_stats_wb_video_air->link_index = mavlink_msg_openhd_stats_wb_video_air_get_link_index(msg);
    openhd_stats_wb_video_air->curr_video_codec = mavlink_msg_openhd_stats_wb_video_air_get_curr_video_codec(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN;
        memset(openhd_stats_wb_video_air, 0, MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_LEN);
    memcpy(openhd_stats_wb_video_air, _MAV_PAYLOAD(msg), len);
#endif
}
