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


static void mavlink_test_openhd_wifibroadcast_statistics(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_openhd_wifibroadcast_statistics_t packet_in = {
        5,72,139,206,17,84,151
    };
    mavlink_openhd_wifibroadcast_statistics_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.radio_port = packet_in.radio_port;
        packet1.count_p_all = packet_in.count_p_all;
        packet1.count_p_bad = packet_in.count_p_bad;
        packet1.count_p_dec_err = packet_in.count_p_dec_err;
        packet1.count_p_dec_ok = packet_in.count_p_dec_ok;
        packet1.count_p_fec_recovered = packet_in.count_p_fec_recovered;
        packet1.count_p_lost = packet_in.count_p_lost;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_wifibroadcast_statistics_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_openhd_wifibroadcast_statistics_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_wifibroadcast_statistics_pack(system_id, component_id, &msg , packet1.radio_port , packet1.count_p_all , packet1.count_p_bad , packet1.count_p_dec_err , packet1.count_p_dec_ok , packet1.count_p_fec_recovered , packet1.count_p_lost );
    mavlink_msg_openhd_wifibroadcast_statistics_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_wifibroadcast_statistics_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.radio_port , packet1.count_p_all , packet1.count_p_bad , packet1.count_p_dec_err , packet1.count_p_dec_ok , packet1.count_p_fec_recovered , packet1.count_p_lost );
    mavlink_msg_openhd_wifibroadcast_statistics_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_openhd_wifibroadcast_statistics_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_wifibroadcast_statistics_send(MAVLINK_COMM_1 , packet1.radio_port , packet1.count_p_all , packet1.count_p_bad , packet1.count_p_dec_err , packet1.count_p_dec_ok , packet1.count_p_fec_recovered , packet1.count_p_lost );
    mavlink_msg_openhd_wifibroadcast_statistics_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("OPENHD_WIFIBROADCAST_STATISTICS") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS) != NULL);
#endif
}

static void mavlink_test_openhd_wifi_card(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_OPENHD_WIFI_CARD >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_openhd_wifi_card_t packet_in = {
        963497464,963497672,963497880,963498088,963498296,65
    };
    mavlink_openhd_wifi_card_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.signal_millidBm = packet_in.signal_millidBm;
        packet1.count_p_received = packet_in.count_p_received;
        packet1.count_p_injected = packet_in.count_p_injected;
        packet1.count_p_tx_err = packet_in.count_p_tx_err;
        packet1.count_p_rx_err = packet_in.count_p_rx_err;
        packet1.card_index = packet_in.card_index;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_OPENHD_WIFI_CARD_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_OPENHD_WIFI_CARD_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_wifi_card_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_openhd_wifi_card_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_wifi_card_pack(system_id, component_id, &msg , packet1.card_index , packet1.signal_millidBm , packet1.count_p_received , packet1.count_p_injected , packet1.count_p_tx_err , packet1.count_p_rx_err );
    mavlink_msg_openhd_wifi_card_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_wifi_card_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.card_index , packet1.signal_millidBm , packet1.count_p_received , packet1.count_p_injected , packet1.count_p_tx_err , packet1.count_p_rx_err );
    mavlink_msg_openhd_wifi_card_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_openhd_wifi_card_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_wifi_card_send(MAVLINK_COMM_1 , packet1.card_index , packet1.signal_millidBm , packet1.count_p_received , packet1.count_p_injected , packet1.count_p_tx_err , packet1.count_p_rx_err );
    mavlink_msg_openhd_wifi_card_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("OPENHD_WIFI_CARD") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_OPENHD_WIFI_CARD) != NULL);
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
        963497464,963497672,963497880,963498088,53
    };
    mavlink_openhd_fec_link_rx_statistics_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.count_blocks_total = packet_in.count_blocks_total;
        packet1.count_blocks_lost = packet_in.count_blocks_lost;
        packet1.count_blocks_recovered = packet_in.count_blocks_recovered;
        packet1.count_fragments_recovered = packet_in.count_fragments_recovered;
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
    mavlink_msg_openhd_fec_link_rx_statistics_pack(system_id, component_id, &msg , packet1.link_index , packet1.count_blocks_total , packet1.count_blocks_lost , packet1.count_blocks_recovered , packet1.count_fragments_recovered );
    mavlink_msg_openhd_fec_link_rx_statistics_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_fec_link_rx_statistics_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.link_index , packet1.count_blocks_total , packet1.count_blocks_lost , packet1.count_blocks_recovered , packet1.count_fragments_recovered );
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
    mavlink_msg_openhd_fec_link_rx_statistics_send(MAVLINK_COMM_1 , packet1.link_index , packet1.count_blocks_total , packet1.count_blocks_lost , packet1.count_blocks_recovered , packet1.count_fragments_recovered );
    mavlink_msg_openhd_fec_link_rx_statistics_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("OPENHD_FEC_LINK_RX_STATISTICS") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_OPENHD_FEC_LINK_RX_STATISTICS) != NULL);
#endif
}

static void mavlink_test_openhd_standard_link_rx_statistics(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_openhd_standard_link_rx_statistics_t packet_in = {
        963497464,963497672,29
    };
    mavlink_openhd_standard_link_rx_statistics_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.count_p_received = packet_in.count_p_received;
        packet1.count_p_lost = packet_in.count_p_lost;
        packet1.link_index = packet_in.link_index;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_standard_link_rx_statistics_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_openhd_standard_link_rx_statistics_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_standard_link_rx_statistics_pack(system_id, component_id, &msg , packet1.link_index , packet1.count_p_received , packet1.count_p_lost );
    mavlink_msg_openhd_standard_link_rx_statistics_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_standard_link_rx_statistics_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.link_index , packet1.count_p_received , packet1.count_p_lost );
    mavlink_msg_openhd_standard_link_rx_statistics_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_openhd_standard_link_rx_statistics_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_standard_link_rx_statistics_send(MAVLINK_COMM_1 , packet1.link_index , packet1.count_p_received , packet1.count_p_lost );
    mavlink_msg_openhd_standard_link_rx_statistics_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("OPENHD_STANDARD_LINK_RX_STATISTICS") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_OPENHD_STANDARD_LINK_RX_STATISTICS) != NULL);
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
        "ABCDEFGHIJKLMNOPQRSTUVWXYZABC"
    };
    mavlink_openhd_version_message_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        
        mav_array_memcpy(packet1.version, packet_in.version, sizeof(char)*30);
        
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
    mavlink_msg_openhd_version_message_pack(system_id, component_id, &msg , packet1.version );
    mavlink_msg_openhd_version_message_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_openhd_version_message_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.version );
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
    mavlink_msg_openhd_version_message_send(MAVLINK_COMM_1 , packet1.version );
    mavlink_msg_openhd_version_message_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("OPENHD_VERSION_MESSAGE") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_OPENHD_VERSION_MESSAGE) != NULL);
#endif
}

static void mavlink_test_openhd(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
    mavlink_test_openhd_wifibroadcast_statistics(system_id, component_id, last_msg);
    mavlink_test_openhd_wifi_card(system_id, component_id, last_msg);
    mavlink_test_openhd_fec_link_rx_statistics(system_id, component_id, last_msg);
    mavlink_test_openhd_standard_link_rx_statistics(system_id, component_id, last_msg);
    mavlink_test_openhd_log_message(system_id, component_id, last_msg);
    mavlink_test_openhd_version_message(system_id, component_id, last_msg);
}

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // OPENHD_TESTSUITE_H
