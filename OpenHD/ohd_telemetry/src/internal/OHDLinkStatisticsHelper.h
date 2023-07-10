//
// Created by consti10 on 16.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_OHDLINKSTATISTICSHELPER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_OHDLINKSTATISTICSHELPER_H_

#include "../mav_include.h"
#include "openhd_link_statistics.hpp"

namespace openhd::LinkStatisticsHelper{

static MavlinkMessage pack_card(const uint8_t system_id,const uint8_t component_id,int card_index,
                             const openhd::link_statistics::StatsPerCard& card_stats){
  MavlinkMessage msg;
  mavlink_openhd_stats_monitor_mode_wifi_card_t tmp;
  tmp.card_index=card_index;
  tmp.rx_rssi_1=card_stats.rx_rssi_1;
  tmp.rx_rssi_2=card_stats.rx_rssi_2;
  tmp.count_p_received=card_stats.count_p_received;
  tmp.count_p_injected=card_stats.count_p_injected;
  tmp.curr_rx_packet_loss_perc=card_stats.curr_rx_packet_loss_perc;
  tmp.card_type=card_stats.card_type;
  tmp.tx_power=card_stats.tx_power;
  tmp.dummy0=0;
  tmp.dummy1=0;
  //openhd::log::get_default()->debug("XX {}",card_stats.to_string(0));
  mavlink_msg_openhd_stats_monitor_mode_wifi_card_encode(system_id,component_id,&msg.m,&tmp);
  return msg;
}

static MavlinkMessage pack_link_general(const uint8_t system_id,const uint8_t component_id,
                                        const openhd::link_statistics::StatsMonitorModeLink stats_monitor_mode_link){
  MavlinkMessage msg;
  mavlink_openhd_stats_monitor_mode_wifi_link_t tmp;
  tmp.curr_tx_pps=stats_monitor_mode_link.curr_tx_pps;
  tmp.curr_rx_pps=stats_monitor_mode_link.curr_rx_pps;
  tmp.curr_tx_bps=stats_monitor_mode_link.curr_tx_bps;
  tmp.curr_rx_bps=stats_monitor_mode_link.curr_rx_bps;
  tmp.count_tx_inj_error_hint=stats_monitor_mode_link.count_tx_inj_error_hint;
  tmp.count_tx_dropped_packets=stats_monitor_mode_link.count_tx_dropped_packets;
  tmp.curr_rx_packet_loss_perc=stats_monitor_mode_link.curr_rx_packet_loss_perc;
  tmp.curr_tx_card_idx=stats_monitor_mode_link.curr_tx_card_idx;
  tmp.curr_tx_mcs_index=stats_monitor_mode_link.curr_tx_mcs_index;
  tmp.curr_tx_channel_mhz=stats_monitor_mode_link.curr_tx_channel_mhz;
  tmp.curr_tx_channel_w_mhz=stats_monitor_mode_link.curr_tx_channel_w_mhz;
  tmp.curr_rx_big_gaps_counter=stats_monitor_mode_link.curr_rx_big_gaps_counter;
  tmp.dummy0=stats_monitor_mode_link.tx_passive_mode_is_enabled;
  //tmp.unused2=stats_monitor_mode_link.unused2;
  //tmp.unused3=stats_monitor_mode_link.unused3;
  mavlink_msg_openhd_stats_monitor_mode_wifi_link_encode(system_id,component_id,&msg.m,&tmp);
  return msg;
}

static MavlinkMessage pack_tele(const uint8_t system_id,const uint8_t component_id,const openhd::link_statistics::StatsTelemetry& stats){
  MavlinkMessage msg;
  mavlink_openhd_stats_telemetry_t tmp{};
  tmp.curr_tx_pps=stats.curr_tx_pps;
  tmp.curr_rx_pps=stats.curr_rx_pps;
  tmp.curr_tx_bps=stats.curr_tx_bps;
  tmp.curr_rx_bps=stats.curr_rx_bps;
  tmp.curr_rx_packet_loss_perc=stats.curr_rx_packet_loss_perc;
  //tmp.unused_0=stats.unused_0;
  //tmp.unused_1=stats.unused_1;
  mavlink_msg_openhd_stats_telemetry_encode(system_id,component_id,&msg.m,&tmp);
  return msg;
}

static MavlinkMessage pack_vid_air(const uint8_t system_id,const uint8_t component_id,const openhd::link_statistics::StatsWBVideoAir& stats){
  MavlinkMessage msg;
  mavlink_openhd_stats_wb_video_air_t tmp;
  tmp.link_index=stats.link_index;
  tmp.curr_video_codec=stats.curr_video_codec;
  tmp.curr_recommended_bitrate=stats.curr_recommended_bitrate;
  tmp.curr_measured_encoder_bitrate=stats.curr_measured_encoder_bitrate;
  tmp.curr_injected_bitrate=stats.curr_injected_bitrate;
  tmp.curr_injected_pps=stats.curr_injected_pps;
  tmp.curr_dropped_packets=stats.curr_dropped_packets;
  tmp.curr_fec_percentage=stats.curr_fec_percentage;
  tmp.curr_keyframe_interval=stats.curr_keyframe_interval;
  tmp.recording_active=stats.recording_active;
  mavlink_msg_openhd_stats_wb_video_air_encode(system_id,component_id,&msg.m,&tmp);
  return msg;
}

static MavlinkMessage pack_vid_air_fec_performance(const uint8_t system_id,const uint8_t component_id,const openhd::link_statistics::StatsWBVideoAir& stats){
  MavlinkMessage msg;
  mavlink_openhd_stats_wb_video_air_fec_performance_t tmp;
  tmp.link_index=stats.link_index;
  tmp.curr_fec_encode_time_avg_us=stats.curr_fec_encode_time_avg_us;
  tmp.curr_fec_encode_time_min_us=stats.curr_fec_encode_time_min_us;
  tmp.curr_fec_encode_time_max_us=stats.curr_fec_encode_time_max_us;
  tmp.curr_fec_block_size_avg=stats.curr_fec_block_size_avg;
  tmp.curr_fec_block_size_min=stats.curr_fec_block_size_min;
  tmp.curr_fec_block_size_max=stats.curr_fec_block_size_max;
  mavlink_msg_openhd_stats_wb_video_air_fec_performance_encode(system_id,component_id,&msg.m,&tmp);
  return msg;
}


static MavlinkMessage pack_vid_gnd(const uint8_t system_id,const uint8_t component_id,const openhd::link_statistics::StatsWBVideoGround& stats){
  MavlinkMessage msg;
  mavlink_openhd_stats_wb_video_ground_t tmp;
  tmp.link_index=stats.link_index;
  tmp.curr_incoming_bitrate=stats.curr_incoming_bitrate;
  tmp.count_blocks_total=stats.count_blocks_total;
  tmp.count_blocks_lost=stats.count_blocks_lost;
  tmp.count_blocks_recovered=stats.count_blocks_recovered;
  tmp.count_fragments_recovered=stats.count_fragments_recovered;
  //tmp.unused0=stats.unused0;
  //tmp.unused1=stats.unused1;
  mavlink_msg_openhd_stats_wb_video_ground_encode(system_id,component_id,&msg.m,&tmp);
  return msg;
}

static MavlinkMessage pack_vid_gnd_fec_performance(const uint8_t system_id,const uint8_t component_id,const openhd::link_statistics::StatsWBVideoGround& stats){
  MavlinkMessage msg;
  mavlink_openhd_stats_wb_video_ground_fec_performance_t tmp;
  tmp.link_index=stats.link_index;
  tmp.curr_fec_decode_time_avg_us=stats.curr_fec_decode_time_avg_us;
  tmp.curr_fec_decode_time_min_us=stats.curr_fec_decode_time_min_us;
  tmp.curr_fec_decode_time_max_us=stats.curr_fec_decode_time_max_us;
  //tmp.unused0=stats.unused0;
  //tmp.unused1=stats.unused1;
  mavlink_msg_openhd_stats_wb_video_ground_fec_performance_encode(system_id,component_id,&msg.m,&tmp);
  return msg;
}

}
#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_OHDLINKSTATISTICSHELPER_H_
