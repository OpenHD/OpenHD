/** @file
 *    @brief MAVLink comm protocol testsuite generated from openhd.xml
 *    @see https://mavlink.io/en/
 */
#pragma once
#ifndef OPENHD_TESTSUITE_H
#define OPENHD_TESTSUITE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAVLINK_TEST_ALL
#define MAVLINK_TEST_ALL
static void mavlink_test_common(uint8_t, uint8_t, mavlink_message_t *last_msg);
static void mavlink_test_ardupilotmega(uint8_t, uint8_t, mavlink_message_t *last_msg);
static void mavlink_test_openhd(uint8_t, uint8_t, mavlink_message_t *last_msg);

static void mavlink_test_all(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
    mavlink_test_common(system_id, component_id, last_msg);
    mavlink_test_ardupilotmega(system_id, component_id, last_msg);
    mavlink_test_openhd(system_id, component_id, last_msg);
}
#endif

#include "../common/testsuite.h"
#include "../ardupilotmega/testsuite.h"


static void mavlink_test_openhd_stats_monitor_mode_wifi_card(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_CARD >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_openhd_stats_monitor_mode_wifi_card_t packet_in = {
        93372036854775807ULL,93372036854776311ULL,93372036854776815ULL,93372036854777319ULL,101,168
    };
    mavlink_openhd_stats_monitor_mode_wifi_card_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.count_p_received = packet_in.count_p_received;
        packet1.count_p_injected = packet_in.count_p_injected;
        packet1.dummy0 = packet_in.dummy0;
        packet1.dummy1 = packet_in.dummy1;
        packet1.card_index = packet_in.card_index;
        packet1.rx_rssi = packet_in.rx_rssi;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_CARD_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_CARD_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_monitor_mode_wifi_card_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_openhd_stats_monitor_mode_wifi_card_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_monitor_mode_wifi_card_pack(system_id, component_id, &msg , packet1.card_index , packet1.rx_rssi , packet1.count_p_received , packet1.count_p_injected , packet1.dummy0 , packet1.dummy1 );
    mavlink_msg_openhd_stats_monitor_mode_wifi_card_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_monitor_mode_wifi_card_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.card_index , packet1.rx_rssi , packet1.count_p_received , packet1.count_p_injected , packet1.dummy0 , packet1.dummy1 );
    mavlink_msg_openhd_stats_monitor_mode_wifi_card_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_openhd_stats_monitor_mode_wifi_card_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_monitor_mode_wifi_card_send(MAVLINK_COMM_1 , packet1.card_index , packet1.rx_rssi , packet1.count_p_received , packet1.count_p_injected , packet1.dummy0 , packet1.dummy1 );
    mavlink_msg_openhd_stats_monitor_mode_wifi_card_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("OPENHD_STATS_MONITOR_MODE_WIFI_CARD") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_CARD) != NULL);
#endif
}

static void mavlink_test_openhd_stats_monitor_mode_wifi_link(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_openhd_stats_monitor_mode_wifi_link_t packet_in = {
        93372036854775807ULL,93372036854776311ULL,93372036854776815ULL,93372036854777319ULL,93372036854777823ULL,963499544,963499752,963499960,963500168
    };
    mavlink_openhd_stats_monitor_mode_wifi_link_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.curr_tx_pps = packet_in.curr_tx_pps;
        packet1.curr_rx_pps = packet_in.curr_rx_pps;
        packet1.curr_tx_bps = packet_in.curr_tx_bps;
        packet1.curr_rx_bps = packet_in.curr_rx_bps;
        packet1.curr_rx_packet_loss = packet_in.curr_rx_packet_loss;
        packet1.unused0 = packet_in.unused0;
        packet1.unused1 = packet_in.unused1;
        packet1.unused2 = packet_in.unused2;
        packet1.unused3 = packet_in.unused3;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_monitor_mode_wifi_link_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_openhd_stats_monitor_mode_wifi_link_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_monitor_mode_wifi_link_pack(system_id, component_id, &msg , packet1.curr_tx_pps , packet1.curr_rx_pps , packet1.curr_tx_bps , packet1.curr_rx_bps , packet1.curr_rx_packet_loss , packet1.unused0 , packet1.unused1 , packet1.unused2 , packet1.unused3 );
    mavlink_msg_openhd_stats_monitor_mode_wifi_link_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_monitor_mode_wifi_link_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.curr_tx_pps , packet1.curr_rx_pps , packet1.curr_tx_bps , packet1.curr_rx_bps , packet1.curr_rx_packet_loss , packet1.unused0 , packet1.unused1 , packet1.unused2 , packet1.unused3 );
    mavlink_msg_openhd_stats_monitor_mode_wifi_link_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_openhd_stats_monitor_mode_wifi_link_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_monitor_mode_wifi_link_send(MAVLINK_COMM_1 , packet1.curr_tx_pps , packet1.curr_rx_pps , packet1.curr_tx_bps , packet1.curr_rx_bps , packet1.curr_rx_packet_loss , packet1.unused0 , packet1.unused1 , packet1.unused2 , packet1.unused3 );
    mavlink_msg_openhd_stats_monitor_mode_wifi_link_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("OPENHD_STATS_MONITOR_MODE_WIFI_LINK") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_LINK) != NULL);
#endif
}

static void mavlink_test_openhd_stats_telemetry(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_openhd_stats_telemetry_t packet_in = {
        93372036854775807ULL,93372036854776311ULL,93372036854776815ULL,93372036854777319ULL,93372036854777823ULL,93372036854778327ULL,93372036854778831ULL
    };
    mavlink_openhd_stats_telemetry_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.curr_tx_pps = packet_in.curr_tx_pps;
        packet1.curr_rx_pps = packet_in.curr_rx_pps;
        packet1.curr_tx_bps = packet_in.curr_tx_bps;
        packet1.curr_rx_bps = packet_in.curr_rx_bps;
        packet1.curr_rx_packet_loss_perc = packet_in.curr_rx_packet_loss_perc;
        packet1.unused_0 = packet_in.unused_0;
        packet1.unused_1 = packet_in.unused_1;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_telemetry_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_openhd_stats_telemetry_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_telemetry_pack(system_id, component_id, &msg , packet1.curr_tx_pps , packet1.curr_rx_pps , packet1.curr_tx_bps , packet1.curr_rx_bps , packet1.curr_rx_packet_loss_perc , packet1.unused_0 , packet1.unused_1 );
    mavlink_msg_openhd_stats_telemetry_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_telemetry_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.curr_tx_pps , packet1.curr_rx_pps , packet1.curr_tx_bps , packet1.curr_rx_bps , packet1.curr_rx_packet_loss_perc , packet1.unused_0 , packet1.unused_1 );
    mavlink_msg_openhd_stats_telemetry_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_openhd_stats_telemetry_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_telemetry_send(MAVLINK_COMM_1 , packet1.curr_tx_pps , packet1.curr_rx_pps , packet1.curr_tx_bps , packet1.curr_rx_bps , packet1.curr_rx_packet_loss_perc , packet1.unused_0 , packet1.unused_1 );
    mavlink_msg_openhd_stats_telemetry_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("OPENHD_STATS_TELEMETRY") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_OPENHD_STATS_TELEMETRY) != NULL);
#endif
}

static void mavlink_test_openhd_stats_wb_video_air(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_openhd_stats_wb_video_air_t packet_in = {
        963497464,963497672,963497880,963498088,963498296,963498504,963498712,963498920,963499128,963499336,19315,19419,19523,15,82
    };
    mavlink_openhd_stats_wb_video_air_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.curr_recommended_bitrate = packet_in.curr_recommended_bitrate;
        packet1.curr_measured_encoder_bitrate = packet_in.curr_measured_encoder_bitrate;
        packet1.curr_injected_bitrate = packet_in.curr_injected_bitrate;
        packet1.curr_injected_pps = packet_in.curr_injected_pps;
        packet1.curr_dropped_packets = packet_in.curr_dropped_packets;
        packet1.curr_fec_encode_time_avg_us = packet_in.curr_fec_encode_time_avg_us;
        packet1.curr_fec_encode_time_min_us = packet_in.curr_fec_encode_time_min_us;
        packet1.curr_fec_encode_time_max_us = packet_in.curr_fec_encode_time_max_us;
        packet1.unused0 = packet_in.unused0;
        packet1.unused1 = packet_in.unused1;
        packet1.curr_fec_block_size_avg = packet_in.curr_fec_block_size_avg;
        packet1.curr_fec_block_size_min = packet_in.curr_fec_block_size_min;
        packet1.curr_fec_block_size_max = packet_in.curr_fec_block_size_max;
        packet1.link_index = packet_in.link_index;
        packet1.curr_video_codec = packet_in.curr_video_codec;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_wb_video_air_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_openhd_stats_wb_video_air_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_wb_video_air_pack(system_id, component_id, &msg , packet1.link_index , packet1.curr_video_codec , packet1.curr_recommended_bitrate , packet1.curr_measured_encoder_bitrate , packet1.curr_injected_bitrate , packet1.curr_injected_pps , packet1.curr_dropped_packets , packet1.curr_fec_encode_time_avg_us , packet1.curr_fec_encode_time_min_us , packet1.curr_fec_encode_time_max_us , packet1.curr_fec_block_size_avg , packet1.curr_fec_block_size_min , packet1.curr_fec_block_size_max , packet1.unused0 , packet1.unused1 );
    mavlink_msg_openhd_stats_wb_video_air_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_wb_video_air_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.link_index , packet1.curr_video_codec , packet1.curr_recommended_bitrate , packet1.curr_measured_encoder_bitrate , packet1.curr_injected_bitrate , packet1.curr_injected_pps , packet1.curr_dropped_packets , packet1.curr_fec_encode_time_avg_us , packet1.curr_fec_encode_time_min_us , packet1.curr_fec_encode_time_max_us , packet1.curr_fec_block_size_avg , packet1.curr_fec_block_size_min , packet1.curr_fec_block_size_max , packet1.unused0 , packet1.unused1 );
    mavlink_msg_openhd_stats_wb_video_air_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_openhd_stats_wb_video_air_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_wb_video_air_send(MAVLINK_COMM_1 , packet1.link_index , packet1.curr_video_codec , packet1.curr_recommended_bitrate , packet1.curr_measured_encoder_bitrate , packet1.curr_injected_bitrate , packet1.curr_injected_pps , packet1.curr_dropped_packets , packet1.curr_fec_encode_time_avg_us , packet1.curr_fec_encode_time_min_us , packet1.curr_fec_encode_time_max_us , packet1.curr_fec_block_size_avg , packet1.curr_fec_block_size_min , packet1.curr_fec_block_size_max , packet1.unused0 , packet1.unused1 );
    mavlink_msg_openhd_stats_wb_video_air_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("OPENHD_STATS_WB_VIDEO_AIR") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_AIR) != NULL);
#endif
}

static void mavlink_test_openhd_stats_wb_video_ground(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_openhd_stats_wb_video_ground_t packet_in = {
        93372036854775807ULL,93372036854776311ULL,93372036854776815ULL,93372036854777319ULL,963499128,963499336,963499544,963499752,963499960,963500168,173
    };
    mavlink_openhd_stats_wb_video_ground_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.count_blocks_total = packet_in.count_blocks_total;
        packet1.count_blocks_lost = packet_in.count_blocks_lost;
        packet1.count_blocks_recovered = packet_in.count_blocks_recovered;
        packet1.count_fragments_recovered = packet_in.count_fragments_recovered;
        packet1.curr_incoming_bitrate = packet_in.curr_incoming_bitrate;
        packet1.curr_fec_decode_time_avg_us = packet_in.curr_fec_decode_time_avg_us;
        packet1.curr_fec_decode_time_min_us = packet_in.curr_fec_decode_time_min_us;
        packet1.curr_fec_decode_time_max_us = packet_in.curr_fec_decode_time_max_us;
        packet1.unused0 = packet_in.unused0;
        packet1.unused1 = packet_in.unused1;
        packet1.link_index = packet_in.link_index;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_wb_video_ground_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_openhd_stats_wb_video_ground_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_wb_video_ground_pack(system_id, component_id, &msg , packet1.link_index , packet1.curr_incoming_bitrate , packet1.count_blocks_total , packet1.count_blocks_lost , packet1.count_blocks_recovered , packet1.count_fragments_recovered , packet1.curr_fec_decode_time_avg_us , packet1.curr_fec_decode_time_min_us , packet1.curr_fec_decode_time_max_us , packet1.unused0 , packet1.unused1 );
    mavlink_msg_openhd_stats_wb_video_ground_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_wb_video_ground_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.link_index , packet1.curr_incoming_bitrate , packet1.count_blocks_total , packet1.count_blocks_lost , packet1.count_blocks_recovered , packet1.count_fragments_recovered , packet1.curr_fec_decode_time_avg_us , packet1.curr_fec_decode_time_min_us , packet1.curr_fec_decode_time_max_us , packet1.unused0 , packet1.unused1 );
    mavlink_msg_openhd_stats_wb_video_ground_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_openhd_stats_wb_video_ground_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_wb_video_ground_send(MAVLINK_COMM_1 , packet1.link_index , packet1.curr_incoming_bitrate , packet1.count_blocks_total , packet1.count_blocks_lost , packet1.count_blocks_recovered , packet1.count_fragments_recovered , packet1.curr_fec_decode_time_avg_us , packet1.curr_fec_decode_time_min_us , packet1.curr_fec_decode_time_max_us , packet1.unused0 , packet1.unused1 );
    mavlink_msg_openhd_stats_wb_video_ground_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("OPENHD_STATS_WB_VIDEO_GROUND") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_OPENHD_STATS_WB_VIDEO_GROUND) != NULL);
#endif
}

static void mavlink_test_openhd_onboard_computer_status_extension(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_openhd_onboard_computer_status_extension_t packet_in = {
        17235,17339,17443,17547,17651,163
    };
    mavlink_openhd_onboard_computer_status_extension_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.cpu_core_voltage_milliV = packet_in.cpu_core_voltage_milliV;
        packet1.reserved1 = packet_in.reserved1;
        packet1.reserved2 = packet_in.reserved2;
        packet1.reserved3 = packet_in.reserved3;
        packet1.reserved4 = packet_in.reserved4;
        packet1.over_current = packet_in.over_current;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_onboard_computer_status_extension_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_openhd_onboard_computer_status_extension_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_onboard_computer_status_extension_pack(system_id, component_id, &msg , packet1.cpu_core_voltage_milliV , packet1.over_current , packet1.reserved1 , packet1.reserved2 , packet1.reserved3 , packet1.reserved4 );
    mavlink_msg_openhd_onboard_computer_status_extension_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_onboard_computer_status_extension_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.cpu_core_voltage_milliV , packet1.over_current , packet1.reserved1 , packet1.reserved2 , packet1.reserved3 , packet1.reserved4 );
    mavlink_msg_openhd_onboard_computer_status_extension_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_openhd_onboard_computer_status_extension_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_onboard_computer_status_extension_send(MAVLINK_COMM_1 , packet1.cpu_core_voltage_milliV , packet1.over_current , packet1.reserved1 , packet1.reserved2 , packet1.reserved3 , packet1.reserved4 );
    mavlink_msg_openhd_onboard_computer_status_extension_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_OPENHD_ONBOARD_COMPUTER_STATUS_EXTENSION) != NULL);
#endif
}

static void mavlink_test_openhd_log_message(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_openhd_log_message_t packet_in = {
        93372036854775807ULL,29,"JKLMNOPQR","TUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOP"
    };
    mavlink_openhd_log_message_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.timestamp = packet_in.timestamp;
        packet1.severity = packet_in.severity;
        
        mav_array_memcpy(packet1.tag, packet_in.tag, sizeof(char)*10);
        mav_array_memcpy(packet1.message, packet_in.message, sizeof(char)*50);
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_log_message_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_openhd_log_message_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_log_message_pack(system_id, component_id, &msg , packet1.severity , packet1.tag , packet1.message , packet1.timestamp );
    mavlink_msg_openhd_log_message_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_log_message_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.severity , packet1.tag , packet1.message , packet1.timestamp );
    mavlink_msg_openhd_log_message_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_openhd_log_message_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_log_message_send(MAVLINK_COMM_1 , packet1.severity , packet1.tag , packet1.message , packet1.timestamp );
    mavlink_msg_openhd_log_message_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("OPENHD_LOG_MESSAGE") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE) != NULL);
#endif
}

static void mavlink_test_openhd_version_message(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_OPENHD_VERSION_MESSAGE >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_openhd_version_message_t packet_in = {
        "ABCDEFGHIJKLMNOPQRSTUVWXYZABC","EFGHIJKLMNOPQRSTUVWXYZABCDEFG"
    };
    mavlink_openhd_version_message_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        
        mav_array_memcpy(packet1.version, packet_in.version, sizeof(char)*30);
        mav_array_memcpy(packet1.commit_hash, packet_in.commit_hash, sizeof(char)*30);
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_OPENHD_VERSION_MESSAGE_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_OPENHD_VERSION_MESSAGE_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_version_message_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_openhd_version_message_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_version_message_pack(system_id, component_id, &msg , packet1.version , packet1.commit_hash );
    mavlink_msg_openhd_version_message_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_version_message_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.version , packet1.commit_hash );
    mavlink_msg_openhd_version_message_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_openhd_version_message_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_version_message_send(MAVLINK_COMM_1 , packet1.version , packet1.commit_hash );
    mavlink_msg_openhd_version_message_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("OPENHD_VERSION_MESSAGE") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_OPENHD_VERSION_MESSAGE) != NULL);
#endif
}

static void mavlink_test_openhd(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
    mavlink_test_openhd_stats_monitor_mode_wifi_card(system_id, component_id, last_msg);
    mavlink_test_openhd_stats_monitor_mode_wifi_link(system_id, component_id, last_msg);
    mavlink_test_openhd_stats_telemetry(system_id, component_id, last_msg);
    mavlink_test_openhd_stats_wb_video_air(system_id, component_id, last_msg);
    mavlink_test_openhd_stats_wb_video_ground(system_id, component_id, last_msg);
    mavlink_test_openhd_onboard_computer_status_extension(system_id, component_id, last_msg);
    mavlink_test_openhd_log_message(system_id, component_id, last_msg);
    mavlink_test_openhd_version_message(system_id, component_id, last_msg);
}

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // OPENHD_TESTSUITE_H
