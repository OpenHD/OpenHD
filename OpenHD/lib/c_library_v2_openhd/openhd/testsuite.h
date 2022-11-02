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


static void mavlink_test_openhd_wifibroadcast_wifi_card(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_openhd_wifibroadcast_wifi_card_t packet_in = {
        93372036854775807ULL,93372036854776311ULL,93372036854776815ULL,93372036854777319ULL,101,168
    };
    mavlink_openhd_wifibroadcast_wifi_card_t packet1, packet2;
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
           memset(MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_wifibroadcast_wifi_card_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_openhd_wifibroadcast_wifi_card_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_wifibroadcast_wifi_card_pack(system_id, component_id, &msg , packet1.card_index , packet1.rx_rssi , packet1.count_p_received , packet1.count_p_injected , packet1.dummy0 , packet1.dummy1 );
    mavlink_msg_openhd_wifibroadcast_wifi_card_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_wifibroadcast_wifi_card_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.card_index , packet1.rx_rssi , packet1.count_p_received , packet1.count_p_injected , packet1.dummy0 , packet1.dummy1 );
    mavlink_msg_openhd_wifibroadcast_wifi_card_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_openhd_wifibroadcast_wifi_card_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_wifibroadcast_wifi_card_send(MAVLINK_COMM_1 , packet1.card_index , packet1.rx_rssi , packet1.count_p_received , packet1.count_p_injected , packet1.dummy0 , packet1.dummy1 );
    mavlink_msg_openhd_wifibroadcast_wifi_card_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("OPENHD_WIFIBROADCAST_WIFI_CARD") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_WIFI_CARD) != NULL);
#endif
}

static void mavlink_test_openhd_stats_total_all_wifibroadcast_streams(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_openhd_stats_total_all_wifibroadcast_streams_t packet_in = {
        93372036854775807ULL,93372036854776311ULL,93372036854776815ULL,93372036854777319ULL,93372036854777823ULL,93372036854778327ULL,93372036854778831ULL,93372036854779335ULL,93372036854779839ULL,93372036854780343ULL,93372036854780847ULL,93372036854781351ULL,93372036854781855ULL,93372036854782359ULL,93372036854782863ULL,93372036854783367ULL,93372036854783871ULL
    };
    mavlink_openhd_stats_total_all_wifibroadcast_streams_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.count_wifi_packets_received = packet_in.count_wifi_packets_received;
        packet1.count_bytes_received = packet_in.count_bytes_received;
        packet1.count_wifi_packets_injected = packet_in.count_wifi_packets_injected;
        packet1.count_bytes_injected = packet_in.count_bytes_injected;
        packet1.count_telemetry_tx_injections_error_hint = packet_in.count_telemetry_tx_injections_error_hint;
        packet1.count_video_tx_injections_error_hint = packet_in.count_video_tx_injections_error_hint;
        packet1.curr_video0_bps = packet_in.curr_video0_bps;
        packet1.curr_video1_bps = packet_in.curr_video1_bps;
        packet1.curr_video0_tx_pps = packet_in.curr_video0_tx_pps;
        packet1.curr_video1_tx_pps = packet_in.curr_video1_tx_pps;
        packet1.curr_telemetry_tx_pps = packet_in.curr_telemetry_tx_pps;
        packet1.curr_telemetry_rx_bps = packet_in.curr_telemetry_rx_bps;
        packet1.curr_telemetry_tx_bps = packet_in.curr_telemetry_tx_bps;
        packet1.unused_0 = packet_in.unused_0;
        packet1.unused_1 = packet_in.unused_1;
        packet1.unused_2 = packet_in.unused_2;
        packet1.unused_3 = packet_in.unused_3;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_pack(system_id, component_id, &msg , packet1.count_wifi_packets_received , packet1.count_bytes_received , packet1.count_wifi_packets_injected , packet1.count_bytes_injected , packet1.count_telemetry_tx_injections_error_hint , packet1.count_video_tx_injections_error_hint , packet1.curr_video0_bps , packet1.curr_video1_bps , packet1.curr_video0_tx_pps , packet1.curr_video1_tx_pps , packet1.curr_telemetry_tx_pps , packet1.curr_telemetry_rx_bps , packet1.curr_telemetry_tx_bps , packet1.unused_0 , packet1.unused_1 , packet1.unused_2 , packet1.unused_3 );
    mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.count_wifi_packets_received , packet1.count_bytes_received , packet1.count_wifi_packets_injected , packet1.count_bytes_injected , packet1.count_telemetry_tx_injections_error_hint , packet1.count_video_tx_injections_error_hint , packet1.curr_video0_bps , packet1.curr_video1_bps , packet1.curr_video0_tx_pps , packet1.curr_video1_tx_pps , packet1.curr_telemetry_tx_pps , packet1.curr_telemetry_rx_bps , packet1.curr_telemetry_tx_bps , packet1.unused_0 , packet1.unused_1 , packet1.unused_2 , packet1.unused_3 );
    mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_send(MAVLINK_COMM_1 , packet1.count_wifi_packets_received , packet1.count_bytes_received , packet1.count_wifi_packets_injected , packet1.count_bytes_injected , packet1.count_telemetry_tx_injections_error_hint , packet1.count_video_tx_injections_error_hint , packet1.curr_video0_bps , packet1.curr_video1_bps , packet1.curr_video0_tx_pps , packet1.curr_video1_tx_pps , packet1.curr_telemetry_tx_pps , packet1.curr_telemetry_rx_bps , packet1.curr_telemetry_tx_bps , packet1.unused_0 , packet1.unused_1 , packet1.unused_2 , packet1.unused_3 );
    mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_OPENHD_STATS_TOTAL_ALL_WIFIBROADCAST_STREAMS) != NULL);
#endif
}

static void mavlink_test_openhd_fec_link_rx_statistics(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_openhd_fec_link_rx_statistics_t packet_in = {
        93372036854775807ULL,93372036854776311ULL,93372036854776815ULL,93372036854777319ULL,93372036854777823ULL,93372036854778327ULL,93372036854778831ULL,173
    };
    mavlink_openhd_fec_link_rx_statistics_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.count_blocks_total = packet_in.count_blocks_total;
        packet1.count_blocks_lost = packet_in.count_blocks_lost;
        packet1.count_blocks_recovered = packet_in.count_blocks_recovered;
        packet1.count_fragments_recovered = packet_in.count_fragments_recovered;
        packet1.count_bytes_forwarded = packet_in.count_bytes_forwarded;
        packet1.unused_0 = packet_in.unused_0;
        packet1.unused_1 = packet_in.unused_1;
        packet1.link_index = packet_in.link_index;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_fec_link_rx_statistics_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_openhd_fec_link_rx_statistics_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_fec_link_rx_statistics_pack(system_id, component_id, &msg , packet1.link_index , packet1.count_blocks_total , packet1.count_blocks_lost , packet1.count_blocks_recovered , packet1.count_fragments_recovered , packet1.count_bytes_forwarded , packet1.unused_0 , packet1.unused_1 );
    mavlink_msg_openhd_fec_link_rx_statistics_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_fec_link_rx_statistics_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.link_index , packet1.count_blocks_total , packet1.count_blocks_lost , packet1.count_blocks_recovered , packet1.count_fragments_recovered , packet1.count_bytes_forwarded , packet1.unused_0 , packet1.unused_1 );
    mavlink_msg_openhd_fec_link_rx_statistics_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_openhd_fec_link_rx_statistics_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_fec_link_rx_statistics_send(MAVLINK_COMM_1 , packet1.link_index , packet1.count_blocks_total , packet1.count_blocks_lost , packet1.count_blocks_recovered , packet1.count_fragments_recovered , packet1.count_bytes_forwarded , packet1.unused_0 , packet1.unused_1 );
    mavlink_msg_openhd_fec_link_rx_statistics_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("OPENHD_FEC_LINK_RX_STATISTICS") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS) != NULL);
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
        93372036854775807ULL,29,"JKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF"
    };
    mavlink_openhd_log_message_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.timestamp = packet_in.timestamp;
        packet1.severity = packet_in.severity;
        
        mav_array_memcpy(packet1.text, packet_in.text, sizeof(char)*50);
        
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
    mavlink_msg_openhd_log_message_pack(system_id, component_id, &msg , packet1.severity , packet1.text , packet1.timestamp );
    mavlink_msg_openhd_log_message_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_log_message_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.severity , packet1.text , packet1.timestamp );
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
    mavlink_msg_openhd_log_message_send(MAVLINK_COMM_1 , packet1.severity , packet1.text , packet1.timestamp );
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
    mavlink_test_openhd_wifibroadcast_wifi_card(system_id, component_id, last_msg);
    mavlink_test_openhd_stats_total_all_wifibroadcast_streams(system_id, component_id, last_msg);
    mavlink_test_openhd_fec_link_rx_statistics(system_id, component_id, last_msg);
    mavlink_test_openhd_onboard_computer_status_extension(system_id, component_id, last_msg);
    mavlink_test_openhd_log_message(system_id, component_id, last_msg);
    mavlink_test_openhd_version_message(system_id, component_id, last_msg);
}

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // OPENHD_TESTSUITE_H
