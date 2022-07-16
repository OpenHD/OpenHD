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
  mavlink_msg_openhd_wifibroadcast_wifi_card_pack(system_id,component_id,&msg.m,card_index,card_stats.rx_rssi,
												  card_stats.count_p_received,card_stats.count_p_injected,0,0);
  return msg;
}

static MavlinkMessage
stats_total_all_wifibroadcast_streams_pack(const uint8_t system_id,const uint8_t component_id,
												const openhd::link_statistics::StatsTotalAllStreams& all_stats){
  MavlinkMessage msg;
  mavlink_msg_openhd_stats_total_all_wifibroadcast_streams_pack(system_id,component_id,&msg.m,all_stats.count_wifi_packets_received,all_stats.count_bytes_received,all_stats.count_wifi_packets_injected,all_stats.count_bytes_injected,
																all_stats.count_telemetry_tx_injections_error_hint,all_stats.count_video_tx_injections_error_hint,all_stats.curr_video0_bps,all_stats.curr_video1_bps
	  ,all_stats.curr_telemetry_rx_bps,all_stats.curr_telemetry_tx_bps);
  return msg;
}

static MavlinkMessage
fec_link_rx_statistics_pack(const uint8_t system_id,const uint8_t component_id,int link_index,
															  const openhd::link_statistics::StatsFECVideoStreamRx& stats_video_stream_rx){
  MavlinkMessage msg;
  mavlink_msg_openhd_fec_link_rx_statistics_pack(system_id,component_id,&msg.m,link_index,stats_video_stream_rx.count_blocks_total,stats_video_stream_rx.count_blocks_lost,stats_video_stream_rx.count_blocks_recovered,stats_video_stream_rx.count_fragments_recovered,stats_video_stream_rx.count_bytes_forwarded);
  return msg;
}

}
#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_OHDLINKSTATISTICSHELPER_H_
