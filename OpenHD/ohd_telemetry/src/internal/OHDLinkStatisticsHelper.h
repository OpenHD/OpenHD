//
// Created by consti10 on 16.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_OHDLINKSTATISTICSHELPER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_OHDLINKSTATISTICSHELPER_H_

#include "../mav_include.h"
#include "openhd-link-statistics.hpp"

namespace openhd::LinkStatisticsHelper{

static MavlinkMessage
wifibroadcast_wifi_card_pack(const uint8_t system_id,const uint8_t component_id,int card_index,
                             const openhd::link_statistics::StatsPerCard& card_stats){
  MavlinkMessage msg;
  mavlink_openhd_wifibroadcast_wifi_card_t tmp;
  tmp.card_index=card_index;
  tmp.rx_rssi=card_stats.rx_rssi;
  tmp.count_p_received=card_stats.count_p_received;
  tmp.count_p_injected=card_stats.count_p_injected;
  tmp.dummy0=0;
  tmp.dummy1=0;
  mavlink_msg_openhd_wifibroadcast_wifi_card_encode(system_id,component_id,&msg.m,&tmp);
  return msg;
}

static MavlinkMessage
stats_total_all_wifibroadcast_streams_pack(const uint8_t system_id,const uint8_t component_id,
                                           const openhd::link_statistics::StatsTotalAllStreams& all_stats){
  MavlinkMessage msg;
  mavlink_openhd_stats_total_all_wifibroadcast_streams_t tmp;
  tmp.count_wifi_packets_received=all_stats.count_wifi_packets_received;
  tmp.count_bytes_received=all_stats.count_bytes_received;
  tmp.count_wifi_packets_injected=all_stats.count_wifi_packets_injected;
  tmp.count_bytes_injected=all_stats.count_bytes_injected;
  tmp.count_telemetry_tx_injections_error_hint=all_stats.count_telemetry_tx_injections_error_hint;
  tmp.count_video_tx_injections_error_hint=all_stats.count_video_tx_injections_error_hint;
  tmp.curr_video0_bps=all_stats.curr_video0_bps;
  tmp.curr_video1_bps=all_stats.curr_video1_bps;
  tmp.curr_video0_tx_pps=all_stats.curr_video0_tx_pps;
  tmp.curr_video1_tx_pps=all_stats.curr_video1_tx_pps;
  tmp.curr_telemetry_tx_pps=all_stats.curr_telemetry_tx_pps;
  tmp.curr_telemetry_rx_bps=all_stats.curr_telemetry_rx_bps;
  tmp.curr_telemetry_tx_bps=all_stats.curr_telemetry_tx_bps;
  tmp.unused_0=all_stats.curr_rx_packet_loss_perc;
  tmp.unused_1=all_stats.curr_n_of_big_gaps;
  tmp.unused_2=all_stats.count_video_tx_dropped_packets;
  tmp.unused_3=0;
  mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_encode(system_id,component_id,&msg.m,&tmp);
  return msg;
}

static MavlinkMessage
fec_link_rx_statistics_pack(const uint8_t system_id,const uint8_t component_id,int link_index,
                            const openhd::link_statistics::StatsFECVideoStreamRx& stats_video_stream_rx){
  MavlinkMessage msg;
  mavlink_openhd_fec_link_rx_statistics_t tmp;
  tmp.link_index=link_index;
  tmp.count_blocks_total=stats_video_stream_rx.count_blocks_total;
  tmp.count_blocks_lost=stats_video_stream_rx.count_blocks_lost;
  tmp.count_blocks_recovered=stats_video_stream_rx.count_blocks_recovered;
  tmp.count_fragments_recovered=stats_video_stream_rx.count_fragments_recovered;
  tmp.count_bytes_forwarded=stats_video_stream_rx.count_bytes_forwarded;
  tmp.unused_0=0;
  tmp.unused_1=0;
  mavlink_msg_openhd_fec_link_rx_statistics_encode(system_id,component_id,&msg.m,&tmp);
  return msg;
}

}
#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_OHDLINKSTATISTICSHELPER_H_
