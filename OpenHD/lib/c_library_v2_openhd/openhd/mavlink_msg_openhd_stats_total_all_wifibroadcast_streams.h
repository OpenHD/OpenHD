#pragma once
// MESSAGE OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS PACKING

#define MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS 1212


typedef struct __mavlink_openhd_stats_total_all_wifibroadcast_streams_t {
 uint64_t count_wifi_packets_received; /*<  current count of all received Wi-Fi packets*/
 uint64_t count_bytes_received; /*<  current count of all received bytes, does not include IEE802 header or similar, but does include FEC overhead*/
 uint64_t count_wifi_packets_injected; /*<  current count of all injected Wi-Fi packets*/
 uint64_t count_bytes_injected; /*<  current count of all outgoing bytes, does not include IEE802 header or similar, but does include FEC overhead*/
 uint64_t count_telemetry_tx_injections_error_hint; /*<  count_telemetry_tx_injections_error_hint*/
 uint64_t count_video_tx_injections_error_hint; /*<  count_video_tx_injections_error_hint*/
 uint64_t curr_video0_bps; /*<  curr_video0_bps*/
 uint64_t curr_video1_bps; /*<  curr_video1_bps*/
 uint64_t curr_video0_tx_pps; /*<  curr_video0_tx_pps*/
 uint64_t curr_video1_tx_pps; /*<  curr_video1_tx_pps*/
 uint64_t curr_telemetry_tx_pps; /*<  curr_telemetry_tx_pps*/
 uint64_t curr_telemetry_rx_bps; /*<  curr_telemetry_rx_bps*/
 uint64_t curr_telemetry_tx_bps; /*<  curr_telemetry_tx_bps*/
 uint64_t unused_0; /*<  unused_0*/
 uint64_t unused_1; /*<  unused_1*/
 uint64_t unused_2; /*<  unused_2*/
 uint64_t unused_3; /*<  unused_3*/
} mavlink_openhd_stats_total_all_wifibroadcast_streams_t;

#define MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN 136
#define MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_MIN_LEN 136
#define MAVLINK_MSG_ID_1212_LEN 136
#define MAVLINK_MSG_ID_1212_MIN_LEN 136

#define MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_CRC 117
#define MAVLINK_MSG_ID_1212_CRC 117



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS { \
    1212, \
    "OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS", \
    17, \
    {  { "count_wifi_packets_received", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, count_wifi_packets_received) }, \
         { "count_bytes_received", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, count_bytes_received) }, \
         { "count_wifi_packets_injected", NULL, MAVLINK_TYPE_UINT64_T, 0, 16, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, count_wifi_packets_injected) }, \
         { "count_bytes_injected", NULL, MAVLINK_TYPE_UINT64_T, 0, 24, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, count_bytes_injected) }, \
         { "count_telemetry_tx_injections_error_hint", NULL, MAVLINK_TYPE_UINT64_T, 0, 32, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, count_telemetry_tx_injections_error_hint) }, \
         { "count_video_tx_injections_error_hint", NULL, MAVLINK_TYPE_UINT64_T, 0, 40, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, count_video_tx_injections_error_hint) }, \
         { "curr_video0_bps", NULL, MAVLINK_TYPE_UINT64_T, 0, 48, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, curr_video0_bps) }, \
         { "curr_video1_bps", NULL, MAVLINK_TYPE_UINT64_T, 0, 56, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, curr_video1_bps) }, \
         { "curr_video0_tx_pps", NULL, MAVLINK_TYPE_UINT64_T, 0, 64, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, curr_video0_tx_pps) }, \
         { "curr_video1_tx_pps", NULL, MAVLINK_TYPE_UINT64_T, 0, 72, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, curr_video1_tx_pps) }, \
         { "curr_telemetry_tx_pps", NULL, MAVLINK_TYPE_UINT64_T, 0, 80, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, curr_telemetry_tx_pps) }, \
         { "curr_telemetry_rx_bps", NULL, MAVLINK_TYPE_UINT64_T, 0, 88, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, curr_telemetry_rx_bps) }, \
         { "curr_telemetry_tx_bps", NULL, MAVLINK_TYPE_UINT64_T, 0, 96, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, curr_telemetry_tx_bps) }, \
         { "unused_0", NULL, MAVLINK_TYPE_UINT64_T, 0, 104, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, unused_0) }, \
         { "unused_1", NULL, MAVLINK_TYPE_UINT64_T, 0, 112, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, unused_1) }, \
         { "unused_2", NULL, MAVLINK_TYPE_UINT64_T, 0, 120, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, unused_2) }, \
         { "unused_3", NULL, MAVLINK_TYPE_UINT64_T, 0, 128, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, unused_3) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS { \
    "OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS", \
    17, \
    {  { "count_wifi_packets_received", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, count_wifi_packets_received) }, \
         { "count_bytes_received", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, count_bytes_received) }, \
         { "count_wifi_packets_injected", NULL, MAVLINK_TYPE_UINT64_T, 0, 16, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, count_wifi_packets_injected) }, \
         { "count_bytes_injected", NULL, MAVLINK_TYPE_UINT64_T, 0, 24, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, count_bytes_injected) }, \
         { "count_telemetry_tx_injections_error_hint", NULL, MAVLINK_TYPE_UINT64_T, 0, 32, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, count_telemetry_tx_injections_error_hint) }, \
         { "count_video_tx_injections_error_hint", NULL, MAVLINK_TYPE_UINT64_T, 0, 40, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, count_video_tx_injections_error_hint) }, \
         { "curr_video0_bps", NULL, MAVLINK_TYPE_UINT64_T, 0, 48, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, curr_video0_bps) }, \
         { "curr_video1_bps", NULL, MAVLINK_TYPE_UINT64_T, 0, 56, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, curr_video1_bps) }, \
         { "curr_video0_tx_pps", NULL, MAVLINK_TYPE_UINT64_T, 0, 64, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, curr_video0_tx_pps) }, \
         { "curr_video1_tx_pps", NULL, MAVLINK_TYPE_UINT64_T, 0, 72, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, curr_video1_tx_pps) }, \
         { "curr_telemetry_tx_pps", NULL, MAVLINK_TYPE_UINT64_T, 0, 80, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, curr_telemetry_tx_pps) }, \
         { "curr_telemetry_rx_bps", NULL, MAVLINK_TYPE_UINT64_T, 0, 88, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, curr_telemetry_rx_bps) }, \
         { "curr_telemetry_tx_bps", NULL, MAVLINK_TYPE_UINT64_T, 0, 96, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, curr_telemetry_tx_bps) }, \
         { "unused_0", NULL, MAVLINK_TYPE_UINT64_T, 0, 104, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, unused_0) }, \
         { "unused_1", NULL, MAVLINK_TYPE_UINT64_T, 0, 112, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, unused_1) }, \
         { "unused_2", NULL, MAVLINK_TYPE_UINT64_T, 0, 120, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, unused_2) }, \
         { "unused_3", NULL, MAVLINK_TYPE_UINT64_T, 0, 128, offsetof(mavlink_openhd_stats_total_all_wifibroadcast_streams_t, unused_3) }, \
         } \
}
#endif

/**
 * @brief Pack a openhd_stats_total_all_wifibroadcast_streams message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param count_wifi_packets_received  current count of all received Wi-Fi packets
 * @param count_bytes_received  current count of all received bytes, does not include IEE802 header or similar, but does include FEC overhead
 * @param count_wifi_packets_injected  current count of all injected Wi-Fi packets
 * @param count_bytes_injected  current count of all outgoing bytes, does not include IEE802 header or similar, but does include FEC overhead
 * @param count_telemetry_tx_injections_error_hint  count_telemetry_tx_injections_error_hint
 * @param count_video_tx_injections_error_hint  count_video_tx_injections_error_hint
 * @param curr_video0_bps  curr_video0_bps
 * @param curr_video1_bps  curr_video1_bps
 * @param curr_video0_tx_pps  curr_video0_tx_pps
 * @param curr_video1_tx_pps  curr_video1_tx_pps
 * @param curr_telemetry_tx_pps  curr_telemetry_tx_pps
 * @param curr_telemetry_rx_bps  curr_telemetry_rx_bps
 * @param curr_telemetry_tx_bps  curr_telemetry_tx_bps
 * @param unused_0  unused_0
 * @param unused_1  unused_1
 * @param unused_2  unused_2
 * @param unused_3  unused_3
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint64_t count_wifi_packets_received, uint64_t count_bytes_received, uint64_t count_wifi_packets_injected, uint64_t count_bytes_injected, uint64_t count_telemetry_tx_injections_error_hint, uint64_t count_video_tx_injections_error_hint, uint64_t curr_video0_bps, uint64_t curr_video1_bps, uint64_t curr_video0_tx_pps, uint64_t curr_video1_tx_pps, uint64_t curr_telemetry_tx_pps, uint64_t curr_telemetry_rx_bps, uint64_t curr_telemetry_tx_bps, uint64_t unused_0, uint64_t unused_1, uint64_t unused_2, uint64_t unused_3)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN];
    _mav_put_uint64_t(buf, 0, count_wifi_packets_received);
    _mav_put_uint64_t(buf, 8, count_bytes_received);
    _mav_put_uint64_t(buf, 16, count_wifi_packets_injected);
    _mav_put_uint64_t(buf, 24, count_bytes_injected);
    _mav_put_uint64_t(buf, 32, count_telemetry_tx_injections_error_hint);
    _mav_put_uint64_t(buf, 40, count_video_tx_injections_error_hint);
    _mav_put_uint64_t(buf, 48, curr_video0_bps);
    _mav_put_uint64_t(buf, 56, curr_video1_bps);
    _mav_put_uint64_t(buf, 64, curr_video0_tx_pps);
    _mav_put_uint64_t(buf, 72, curr_video1_tx_pps);
    _mav_put_uint64_t(buf, 80, curr_telemetry_tx_pps);
    _mav_put_uint64_t(buf, 88, curr_telemetry_rx_bps);
    _mav_put_uint64_t(buf, 96, curr_telemetry_tx_bps);
    _mav_put_uint64_t(buf, 104, unused_0);
    _mav_put_uint64_t(buf, 112, unused_1);
    _mav_put_uint64_t(buf, 120, unused_2);
    _mav_put_uint64_t(buf, 128, unused_3);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN);
#else
    mavlink_openhd_stats_total_all_wifibroadcast_streams_t packet;
    packet.count_wifi_packets_received = count_wifi_packets_received;
    packet.count_bytes_received = count_bytes_received;
    packet.count_wifi_packets_injected = count_wifi_packets_injected;
    packet.count_bytes_injected = count_bytes_injected;
    packet.count_telemetry_tx_injections_error_hint = count_telemetry_tx_injections_error_hint;
    packet.count_video_tx_injections_error_hint = count_video_tx_injections_error_hint;
    packet.curr_video0_bps = curr_video0_bps;
    packet.curr_video1_bps = curr_video1_bps;
    packet.curr_video0_tx_pps = curr_video0_tx_pps;
    packet.curr_video1_tx_pps = curr_video1_tx_pps;
    packet.curr_telemetry_tx_pps = curr_telemetry_tx_pps;
    packet.curr_telemetry_rx_bps = curr_telemetry_rx_bps;
    packet.curr_telemetry_tx_bps = curr_telemetry_tx_bps;
    packet.unused_0 = unused_0;
    packet.unused_1 = unused_1;
    packet.unused_2 = unused_2;
    packet.unused_3 = unused_3;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_CRC);
}

/**
 * @brief Pack a openhd_stats_total_all_wifibroadcast_streams message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param count_wifi_packets_received  current count of all received Wi-Fi packets
 * @param count_bytes_received  current count of all received bytes, does not include IEE802 header or similar, but does include FEC overhead
 * @param count_wifi_packets_injected  current count of all injected Wi-Fi packets
 * @param count_bytes_injected  current count of all outgoing bytes, does not include IEE802 header or similar, but does include FEC overhead
 * @param count_telemetry_tx_injections_error_hint  count_telemetry_tx_injections_error_hint
 * @param count_video_tx_injections_error_hint  count_video_tx_injections_error_hint
 * @param curr_video0_bps  curr_video0_bps
 * @param curr_video1_bps  curr_video1_bps
 * @param curr_video0_tx_pps  curr_video0_tx_pps
 * @param curr_video1_tx_pps  curr_video1_tx_pps
 * @param curr_telemetry_tx_pps  curr_telemetry_tx_pps
 * @param curr_telemetry_rx_bps  curr_telemetry_rx_bps
 * @param curr_telemetry_tx_bps  curr_telemetry_tx_bps
 * @param unused_0  unused_0
 * @param unused_1  unused_1
 * @param unused_2  unused_2
 * @param unused_3  unused_3
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint64_t count_wifi_packets_received,uint64_t count_bytes_received,uint64_t count_wifi_packets_injected,uint64_t count_bytes_injected,uint64_t count_telemetry_tx_injections_error_hint,uint64_t count_video_tx_injections_error_hint,uint64_t curr_video0_bps,uint64_t curr_video1_bps,uint64_t curr_video0_tx_pps,uint64_t curr_video1_tx_pps,uint64_t curr_telemetry_tx_pps,uint64_t curr_telemetry_rx_bps,uint64_t curr_telemetry_tx_bps,uint64_t unused_0,uint64_t unused_1,uint64_t unused_2,uint64_t unused_3)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN];
    _mav_put_uint64_t(buf, 0, count_wifi_packets_received);
    _mav_put_uint64_t(buf, 8, count_bytes_received);
    _mav_put_uint64_t(buf, 16, count_wifi_packets_injected);
    _mav_put_uint64_t(buf, 24, count_bytes_injected);
    _mav_put_uint64_t(buf, 32, count_telemetry_tx_injections_error_hint);
    _mav_put_uint64_t(buf, 40, count_video_tx_injections_error_hint);
    _mav_put_uint64_t(buf, 48, curr_video0_bps);
    _mav_put_uint64_t(buf, 56, curr_video1_bps);
    _mav_put_uint64_t(buf, 64, curr_video0_tx_pps);
    _mav_put_uint64_t(buf, 72, curr_video1_tx_pps);
    _mav_put_uint64_t(buf, 80, curr_telemetry_tx_pps);
    _mav_put_uint64_t(buf, 88, curr_telemetry_rx_bps);
    _mav_put_uint64_t(buf, 96, curr_telemetry_tx_bps);
    _mav_put_uint64_t(buf, 104, unused_0);
    _mav_put_uint64_t(buf, 112, unused_1);
    _mav_put_uint64_t(buf, 120, unused_2);
    _mav_put_uint64_t(buf, 128, unused_3);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN);
#else
    mavlink_openhd_stats_total_all_wifibroadcast_streams_t packet;
    packet.count_wifi_packets_received = count_wifi_packets_received;
    packet.count_bytes_received = count_bytes_received;
    packet.count_wifi_packets_injected = count_wifi_packets_injected;
    packet.count_bytes_injected = count_bytes_injected;
    packet.count_telemetry_tx_injections_error_hint = count_telemetry_tx_injections_error_hint;
    packet.count_video_tx_injections_error_hint = count_video_tx_injections_error_hint;
    packet.curr_video0_bps = curr_video0_bps;
    packet.curr_video1_bps = curr_video1_bps;
    packet.curr_video0_tx_pps = curr_video0_tx_pps;
    packet.curr_video1_tx_pps = curr_video1_tx_pps;
    packet.curr_telemetry_tx_pps = curr_telemetry_tx_pps;
    packet.curr_telemetry_rx_bps = curr_telemetry_rx_bps;
    packet.curr_telemetry_tx_bps = curr_telemetry_tx_bps;
    packet.unused_0 = unused_0;
    packet.unused_1 = unused_1;
    packet.unused_2 = unused_2;
    packet.unused_3 = unused_3;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_CRC);
}

/**
 * @brief Encode a openhd_stats_total_all_wifibroadcast_streams struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param openhd_stats_total_all_wifibroadcast_streams C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_openhd_stats_total_all_wifibroadcast_streams_t* openhd_stats_total_all_wifibroadcast_streams)
{
    return mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_pack(system_id, component_id, msg, openhd_stats_total_all_wifibroadcast_streams->count_wifi_packets_received, openhd_stats_total_all_wifibroadcast_streams->count_bytes_received, openhd_stats_total_all_wifibroadcast_streams->count_wifi_packets_injected, openhd_stats_total_all_wifibroadcast_streams->count_bytes_injected, openhd_stats_total_all_wifibroadcast_streams->count_telemetry_tx_injections_error_hint, openhd_stats_total_all_wifibroadcast_streams->count_video_tx_injections_error_hint, openhd_stats_total_all_wifibroadcast_streams->curr_video0_bps, openhd_stats_total_all_wifibroadcast_streams->curr_video1_bps, openhd_stats_total_all_wifibroadcast_streams->curr_video0_tx_pps, openhd_stats_total_all_wifibroadcast_streams->curr_video1_tx_pps, openhd_stats_total_all_wifibroadcast_streams->curr_telemetry_tx_pps, openhd_stats_total_all_wifibroadcast_streams->curr_telemetry_rx_bps, openhd_stats_total_all_wifibroadcast_streams->curr_telemetry_tx_bps, openhd_stats_total_all_wifibroadcast_streams->unused_0, openhd_stats_total_all_wifibroadcast_streams->unused_1, openhd_stats_total_all_wifibroadcast_streams->unused_2, openhd_stats_total_all_wifibroadcast_streams->unused_3);
}

/**
 * @brief Encode a openhd_stats_total_all_wifibroadcast_streams struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param openhd_stats_total_all_wifibroadcast_streams C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_openhd_stats_total_all_wifibroadcast_streams_t* openhd_stats_total_all_wifibroadcast_streams)
{
    return mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_pack_chan(system_id, component_id, chan, msg, openhd_stats_total_all_wifibroadcast_streams->count_wifi_packets_received, openhd_stats_total_all_wifibroadcast_streams->count_bytes_received, openhd_stats_total_all_wifibroadcast_streams->count_wifi_packets_injected, openhd_stats_total_all_wifibroadcast_streams->count_bytes_injected, openhd_stats_total_all_wifibroadcast_streams->count_telemetry_tx_injections_error_hint, openhd_stats_total_all_wifibroadcast_streams->count_video_tx_injections_error_hint, openhd_stats_total_all_wifibroadcast_streams->curr_video0_bps, openhd_stats_total_all_wifibroadcast_streams->curr_video1_bps, openhd_stats_total_all_wifibroadcast_streams->curr_video0_tx_pps, openhd_stats_total_all_wifibroadcast_streams->curr_video1_tx_pps, openhd_stats_total_all_wifibroadcast_streams->curr_telemetry_tx_pps, openhd_stats_total_all_wifibroadcast_streams->curr_telemetry_rx_bps, openhd_stats_total_all_wifibroadcast_streams->curr_telemetry_tx_bps, openhd_stats_total_all_wifibroadcast_streams->unused_0, openhd_stats_total_all_wifibroadcast_streams->unused_1, openhd_stats_total_all_wifibroadcast_streams->unused_2, openhd_stats_total_all_wifibroadcast_streams->unused_3);
}

/**
 * @brief Send a openhd_stats_total_all_wifibroadcast_streams message
 * @param chan MAVLink channel to send the message
 *
 * @param count_wifi_packets_received  current count of all received Wi-Fi packets
 * @param count_bytes_received  current count of all received bytes, does not include IEE802 header or similar, but does include FEC overhead
 * @param count_wifi_packets_injected  current count of all injected Wi-Fi packets
 * @param count_bytes_injected  current count of all outgoing bytes, does not include IEE802 header or similar, but does include FEC overhead
 * @param count_telemetry_tx_injections_error_hint  count_telemetry_tx_injections_error_hint
 * @param count_video_tx_injections_error_hint  count_video_tx_injections_error_hint
 * @param curr_video0_bps  curr_video0_bps
 * @param curr_video1_bps  curr_video1_bps
 * @param curr_video0_tx_pps  curr_video0_tx_pps
 * @param curr_video1_tx_pps  curr_video1_tx_pps
 * @param curr_telemetry_tx_pps  curr_telemetry_tx_pps
 * @param curr_telemetry_rx_bps  curr_telemetry_rx_bps
 * @param curr_telemetry_tx_bps  curr_telemetry_tx_bps
 * @param unused_0  unused_0
 * @param unused_1  unused_1
 * @param unused_2  unused_2
 * @param unused_3  unused_3
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_send(mavlink_channel_t chan, uint64_t count_wifi_packets_received, uint64_t count_bytes_received, uint64_t count_wifi_packets_injected, uint64_t count_bytes_injected, uint64_t count_telemetry_tx_injections_error_hint, uint64_t count_video_tx_injections_error_hint, uint64_t curr_video0_bps, uint64_t curr_video1_bps, uint64_t curr_video0_tx_pps, uint64_t curr_video1_tx_pps, uint64_t curr_telemetry_tx_pps, uint64_t curr_telemetry_rx_bps, uint64_t curr_telemetry_tx_bps, uint64_t unused_0, uint64_t unused_1, uint64_t unused_2, uint64_t unused_3)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN];
    _mav_put_uint64_t(buf, 0, count_wifi_packets_received);
    _mav_put_uint64_t(buf, 8, count_bytes_received);
    _mav_put_uint64_t(buf, 16, count_wifi_packets_injected);
    _mav_put_uint64_t(buf, 24, count_bytes_injected);
    _mav_put_uint64_t(buf, 32, count_telemetry_tx_injections_error_hint);
    _mav_put_uint64_t(buf, 40, count_video_tx_injections_error_hint);
    _mav_put_uint64_t(buf, 48, curr_video0_bps);
    _mav_put_uint64_t(buf, 56, curr_video1_bps);
    _mav_put_uint64_t(buf, 64, curr_video0_tx_pps);
    _mav_put_uint64_t(buf, 72, curr_video1_tx_pps);
    _mav_put_uint64_t(buf, 80, curr_telemetry_tx_pps);
    _mav_put_uint64_t(buf, 88, curr_telemetry_rx_bps);
    _mav_put_uint64_t(buf, 96, curr_telemetry_tx_bps);
    _mav_put_uint64_t(buf, 104, unused_0);
    _mav_put_uint64_t(buf, 112, unused_1);
    _mav_put_uint64_t(buf, 120, unused_2);
    _mav_put_uint64_t(buf, 128, unused_3);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS, buf, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_CRC);
#else
    mavlink_openhd_stats_total_all_wifibroadcast_streams_t packet;
    packet.count_wifi_packets_received = count_wifi_packets_received;
    packet.count_bytes_received = count_bytes_received;
    packet.count_wifi_packets_injected = count_wifi_packets_injected;
    packet.count_bytes_injected = count_bytes_injected;
    packet.count_telemetry_tx_injections_error_hint = count_telemetry_tx_injections_error_hint;
    packet.count_video_tx_injections_error_hint = count_video_tx_injections_error_hint;
    packet.curr_video0_bps = curr_video0_bps;
    packet.curr_video1_bps = curr_video1_bps;
    packet.curr_video0_tx_pps = curr_video0_tx_pps;
    packet.curr_video1_tx_pps = curr_video1_tx_pps;
    packet.curr_telemetry_tx_pps = curr_telemetry_tx_pps;
    packet.curr_telemetry_rx_bps = curr_telemetry_rx_bps;
    packet.curr_telemetry_tx_bps = curr_telemetry_tx_bps;
    packet.unused_0 = unused_0;
    packet.unused_1 = unused_1;
    packet.unused_2 = unused_2;
    packet.unused_3 = unused_3;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS, (const char *)&packet, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_CRC);
#endif
}

/**
 * @brief Send a openhd_stats_total_all_wifibroadcast_streams message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_send_struct(mavlink_channel_t chan, const mavlink_openhd_stats_total_all_wifibroadcast_streams_t* openhd_stats_total_all_wifibroadcast_streams)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_send(chan, openhd_stats_total_all_wifibroadcast_streams->count_wifi_packets_received, openhd_stats_total_all_wifibroadcast_streams->count_bytes_received, openhd_stats_total_all_wifibroadcast_streams->count_wifi_packets_injected, openhd_stats_total_all_wifibroadcast_streams->count_bytes_injected, openhd_stats_total_all_wifibroadcast_streams->count_telemetry_tx_injections_error_hint, openhd_stats_total_all_wifibroadcast_streams->count_video_tx_injections_error_hint, openhd_stats_total_all_wifibroadcast_streams->curr_video0_bps, openhd_stats_total_all_wifibroadcast_streams->curr_video1_bps, openhd_stats_total_all_wifibroadcast_streams->curr_video0_tx_pps, openhd_stats_total_all_wifibroadcast_streams->curr_video1_tx_pps, openhd_stats_total_all_wifibroadcast_streams->curr_telemetry_tx_pps, openhd_stats_total_all_wifibroadcast_streams->curr_telemetry_rx_bps, openhd_stats_total_all_wifibroadcast_streams->curr_telemetry_tx_bps, openhd_stats_total_all_wifibroadcast_streams->unused_0, openhd_stats_total_all_wifibroadcast_streams->unused_1, openhd_stats_total_all_wifibroadcast_streams->unused_2, openhd_stats_total_all_wifibroadcast_streams->unused_3);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS, (const char *)openhd_stats_total_all_wifibroadcast_streams, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_CRC);
#endif
}

#if MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint64_t count_wifi_packets_received, uint64_t count_bytes_received, uint64_t count_wifi_packets_injected, uint64_t count_bytes_injected, uint64_t count_telemetry_tx_injections_error_hint, uint64_t count_video_tx_injections_error_hint, uint64_t curr_video0_bps, uint64_t curr_video1_bps, uint64_t curr_video0_tx_pps, uint64_t curr_video1_tx_pps, uint64_t curr_telemetry_tx_pps, uint64_t curr_telemetry_rx_bps, uint64_t curr_telemetry_tx_bps, uint64_t unused_0, uint64_t unused_1, uint64_t unused_2, uint64_t unused_3)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint64_t(buf, 0, count_wifi_packets_received);
    _mav_put_uint64_t(buf, 8, count_bytes_received);
    _mav_put_uint64_t(buf, 16, count_wifi_packets_injected);
    _mav_put_uint64_t(buf, 24, count_bytes_injected);
    _mav_put_uint64_t(buf, 32, count_telemetry_tx_injections_error_hint);
    _mav_put_uint64_t(buf, 40, count_video_tx_injections_error_hint);
    _mav_put_uint64_t(buf, 48, curr_video0_bps);
    _mav_put_uint64_t(buf, 56, curr_video1_bps);
    _mav_put_uint64_t(buf, 64, curr_video0_tx_pps);
    _mav_put_uint64_t(buf, 72, curr_video1_tx_pps);
    _mav_put_uint64_t(buf, 80, curr_telemetry_tx_pps);
    _mav_put_uint64_t(buf, 88, curr_telemetry_rx_bps);
    _mav_put_uint64_t(buf, 96, curr_telemetry_tx_bps);
    _mav_put_uint64_t(buf, 104, unused_0);
    _mav_put_uint64_t(buf, 112, unused_1);
    _mav_put_uint64_t(buf, 120, unused_2);
    _mav_put_uint64_t(buf, 128, unused_3);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS, buf, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_CRC);
#else
    mavlink_openhd_stats_total_all_wifibroadcast_streams_t *packet = (mavlink_openhd_stats_total_all_wifibroadcast_streams_t *)msgbuf;
    packet->count_wifi_packets_received = count_wifi_packets_received;
    packet->count_bytes_received = count_bytes_received;
    packet->count_wifi_packets_injected = count_wifi_packets_injected;
    packet->count_bytes_injected = count_bytes_injected;
    packet->count_telemetry_tx_injections_error_hint = count_telemetry_tx_injections_error_hint;
    packet->count_video_tx_injections_error_hint = count_video_tx_injections_error_hint;
    packet->curr_video0_bps = curr_video0_bps;
    packet->curr_video1_bps = curr_video1_bps;
    packet->curr_video0_tx_pps = curr_video0_tx_pps;
    packet->curr_video1_tx_pps = curr_video1_tx_pps;
    packet->curr_telemetry_tx_pps = curr_telemetry_tx_pps;
    packet->curr_telemetry_rx_bps = curr_telemetry_rx_bps;
    packet->curr_telemetry_tx_bps = curr_telemetry_tx_bps;
    packet->unused_0 = unused_0;
    packet->unused_1 = unused_1;
    packet->unused_2 = unused_2;
    packet->unused_3 = unused_3;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS, (const char *)packet, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_MIN_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_CRC);
#endif
}
#endif

#endif

// MESSAGE OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS UNPACKING


/**
 * @brief Get field count_wifi_packets_received from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  current count of all received Wi-Fi packets
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_count_wifi_packets_received(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  0);
}

/**
 * @brief Get field count_bytes_received from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  current count of all received bytes, does not include IEE802 header or similar, but does include FEC overhead
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_count_bytes_received(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  8);
}

/**
 * @brief Get field count_wifi_packets_injected from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  current count of all injected Wi-Fi packets
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_count_wifi_packets_injected(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  16);
}

/**
 * @brief Get field count_bytes_injected from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  current count of all outgoing bytes, does not include IEE802 header or similar, but does include FEC overhead
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_count_bytes_injected(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  24);
}

/**
 * @brief Get field count_telemetry_tx_injections_error_hint from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  count_telemetry_tx_injections_error_hint
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_count_telemetry_tx_injections_error_hint(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  32);
}

/**
 * @brief Get field count_video_tx_injections_error_hint from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  count_video_tx_injections_error_hint
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_count_video_tx_injections_error_hint(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  40);
}

/**
 * @brief Get field curr_video0_bps from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  curr_video0_bps
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_curr_video0_bps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  48);
}

/**
 * @brief Get field curr_video1_bps from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  curr_video1_bps
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_curr_video1_bps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  56);
}

/**
 * @brief Get field curr_video0_tx_pps from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  curr_video0_tx_pps
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_curr_video0_tx_pps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  64);
}

/**
 * @brief Get field curr_video1_tx_pps from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  curr_video1_tx_pps
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_curr_video1_tx_pps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  72);
}

/**
 * @brief Get field curr_telemetry_tx_pps from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  curr_telemetry_tx_pps
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_curr_telemetry_tx_pps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  80);
}

/**
 * @brief Get field curr_telemetry_rx_bps from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  curr_telemetry_rx_bps
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_curr_telemetry_rx_bps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  88);
}

/**
 * @brief Get field curr_telemetry_tx_bps from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  curr_telemetry_tx_bps
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_curr_telemetry_tx_bps(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  96);
}

/**
 * @brief Get field unused_0 from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  unused_0
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_unused_0(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  104);
}

/**
 * @brief Get field unused_1 from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  unused_1
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_unused_1(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  112);
}

/**
 * @brief Get field unused_2 from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  unused_2
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_unused_2(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  120);
}

/**
 * @brief Get field unused_3 from openhd_stats_total_all_wifibroadcast_streams message
 *
 * @return  unused_3
 */
static inline uint64_t mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_unused_3(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  128);
}

/**
 * @brief Decode a openhd_stats_total_all_wifibroadcast_streams message into a struct
 *
 * @param msg The message to decode
 * @param openhd_stats_total_all_wifibroadcast_streams C-struct to decode the message contents into
 */
static inline void mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_decode(const mavlink_message_t* msg, mavlink_openhd_stats_total_all_wifibroadcast_streams_t* openhd_stats_total_all_wifibroadcast_streams)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    openhd_stats_total_all_wifibroadcast_streams->count_wifi_packets_received = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_count_wifi_packets_received(msg);
    openhd_stats_total_all_wifibroadcast_streams->count_bytes_received = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_count_bytes_received(msg);
    openhd_stats_total_all_wifibroadcast_streams->count_wifi_packets_injected = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_count_wifi_packets_injected(msg);
    openhd_stats_total_all_wifibroadcast_streams->count_bytes_injected = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_count_bytes_injected(msg);
    openhd_stats_total_all_wifibroadcast_streams->count_telemetry_tx_injections_error_hint = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_count_telemetry_tx_injections_error_hint(msg);
    openhd_stats_total_all_wifibroadcast_streams->count_video_tx_injections_error_hint = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_count_video_tx_injections_error_hint(msg);
    openhd_stats_total_all_wifibroadcast_streams->curr_video0_bps = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_curr_video0_bps(msg);
    openhd_stats_total_all_wifibroadcast_streams->curr_video1_bps = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_curr_video1_bps(msg);
    openhd_stats_total_all_wifibroadcast_streams->curr_video0_tx_pps = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_curr_video0_tx_pps(msg);
    openhd_stats_total_all_wifibroadcast_streams->curr_video1_tx_pps = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_curr_video1_tx_pps(msg);
    openhd_stats_total_all_wifibroadcast_streams->curr_telemetry_tx_pps = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_curr_telemetry_tx_pps(msg);
    openhd_stats_total_all_wifibroadcast_streams->curr_telemetry_rx_bps = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_curr_telemetry_rx_bps(msg);
    openhd_stats_total_all_wifibroadcast_streams->curr_telemetry_tx_bps = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_curr_telemetry_tx_bps(msg);
    openhd_stats_total_all_wifibroadcast_streams->unused_0 = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_unused_0(msg);
    openhd_stats_total_all_wifibroadcast_streams->unused_1 = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_unused_1(msg);
    openhd_stats_total_all_wifibroadcast_streams->unused_2 = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_unused_2(msg);
    openhd_stats_total_all_wifibroadcast_streams->unused_3 = mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_get_unused_3(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN? msg->len : MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN;
        memset(openhd_stats_total_all_wifibroadcast_streams, 0, MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_LEN);
    memcpy(openhd_stats_total_all_wifibroadcast_streams, _MAV_PAYLOAD(msg), len);
#endif
}
