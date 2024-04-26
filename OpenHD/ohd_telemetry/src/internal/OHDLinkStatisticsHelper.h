//
// Created by consti10 on 16.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_OHDLINKSTATISTICSHELPER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_OHDLINKSTATISTICSHELPER_H_

#include "../mav_include.h"
#include "openhd_action_handler.h"
#include "openhd_external_device.h"
#include "openhd_link_statistics.hpp"

namespace openhd::LinkStatisticsHelper {

static MavlinkMessage pack_card(
    const uint8_t system_id, const uint8_t component_id, int card_index,
    const openhd::link_statistics::
        Xmavlink_openhd_stats_monitor_mode_wifi_card_t& card_stats) {
  MavlinkMessage msg;
  mavlink_openhd_stats_monitor_mode_wifi_card_t tmp{};
  tmp.card_index = card_index;
  tmp.rx_rssi = card_stats.rx_rssi;
  tmp.rx_rssi_1 = card_stats.rx_rssi_1;
  tmp.rx_rssi_2 = card_stats.rx_rssi_2;
  tmp.count_p_received = card_stats.count_p_received;
  tmp.count_p_injected = card_stats.count_p_injected;
  tmp.curr_rx_packet_loss_perc = card_stats.curr_rx_packet_loss_perc;
  tmp.card_type = card_stats.card_type;
  tmp.dummy2 = card_stats.card_sub_type;
  tmp.tx_power_current = card_stats.tx_power_current;
  tmp.tx_power_armed = card_stats.tx_power_armed;
  tmp.tx_power_disarmed = card_stats.tx_power_disarmed;
  tmp.curr_status = card_stats.curr_status;
  tmp.rx_signal_quality_adapter = card_stats.rx_signal_quality_adapter;
  tmp.rx_noise_adapter = card_stats.rx_noise_adapter;
  tmp.tx_active = card_stats.tx_active;
  // openhd::log::get_default()->debug("XX {}",card_stats.to_string(0));
  mavlink_msg_openhd_stats_monitor_mode_wifi_card_encode(
      system_id, component_id, &msg.m, &tmp);
  return msg;
}

static MavlinkMessage pack_link_general(
    const uint8_t system_id, const uint8_t component_id,
    const openhd::link_statistics::
        Xmavlink_openhd_stats_monitor_mode_wifi_link_t&
            stats_monitor_mode_link) {
  MavlinkMessage msg;
  mavlink_openhd_stats_monitor_mode_wifi_link_t tmp{};
  tmp.curr_tx_pps = stats_monitor_mode_link.curr_tx_pps;
  tmp.curr_rx_pps = stats_monitor_mode_link.curr_rx_pps;
  tmp.curr_tx_bps = stats_monitor_mode_link.curr_tx_bps;
  tmp.curr_rx_bps = stats_monitor_mode_link.curr_rx_bps;
  tmp.count_tx_inj_error_hint = stats_monitor_mode_link.count_tx_inj_error_hint;
  tmp.count_tx_dropped_packets =
      stats_monitor_mode_link.count_tx_dropped_packets;
  tmp.curr_rx_packet_loss_perc =
      stats_monitor_mode_link.curr_rx_packet_loss_perc;
  tmp.curr_tx_mcs_index = stats_monitor_mode_link.curr_tx_mcs_index;
  tmp.curr_tx_channel_mhz = stats_monitor_mode_link.curr_tx_channel_mhz;
  tmp.curr_tx_channel_w_mhz = stats_monitor_mode_link.curr_tx_channel_w_mhz;
  tmp.curr_rx_big_gaps_counter =
      stats_monitor_mode_link.curr_rx_big_gaps_counter;
  tmp.bitfield = stats_monitor_mode_link.bitfield;
  tmp.curr_rate_kbits = stats_monitor_mode_link.curr_rate_kbits;
  tmp.curr_n_rate_adjustments = stats_monitor_mode_link.curr_n_rate_adjustments;
  tmp.pollution_perc = stats_monitor_mode_link.pollution_perc;
  tmp.dummy0 = stats_monitor_mode_link.dummy0;
  tmp.dummy1 = stats_monitor_mode_link.dummy1;
  tmp.dummy2 = stats_monitor_mode_link.dummy2;
  // tmp.unused2=stats_monitor_mode_link.unused2;
  // tmp.unused3=stats_monitor_mode_link.unused3;
  mavlink_msg_openhd_stats_monitor_mode_wifi_link_encode(
      system_id, component_id, &msg.m, &tmp);
  return msg;
}

static MavlinkMessage pack_tele(
    const uint8_t system_id, const uint8_t component_id,
    const openhd::link_statistics::Xmavlink_openhd_stats_telemetry_t& stats) {
  MavlinkMessage msg;
  mavlink_openhd_stats_telemetry_t tmp{};
  tmp.curr_tx_pps = stats.curr_tx_pps;
  tmp.curr_rx_pps = stats.curr_rx_pps;
  tmp.curr_tx_bps = stats.curr_tx_bps;
  tmp.curr_rx_bps = stats.curr_rx_bps;
  tmp.curr_rx_packet_loss_perc = stats.curr_rx_packet_loss_perc;
  // tmp.unused_0=stats.unused_0;
  // tmp.unused_1=stats.unused_1;
  mavlink_msg_openhd_stats_telemetry_encode(system_id, component_id, &msg.m,
                                            &tmp);
  return msg;
}

static MavlinkMessage pack_vid_air(
    const uint8_t system_id, const uint8_t component_id,
    const openhd::link_statistics::Xmavlink_openhd_stats_wb_video_air_t&
        stats) {
  MavlinkMessage msg;
  mavlink_openhd_stats_wb_video_air_t tmp{};
  tmp.link_index = stats.link_index;
  tmp.curr_recommended_bitrate = stats.curr_recommended_bitrate;
  tmp.curr_measured_encoder_bitrate = stats.curr_measured_encoder_bitrate;
  tmp.curr_injected_bitrate = stats.curr_injected_bitrate;
  tmp.curr_injected_pps = stats.curr_injected_pps;
  tmp.curr_dropped_frames = stats.curr_dropped_frames;
  tmp.curr_fec_percentage = stats.curr_fec_percentage;
  tmp.dummy0 = stats.dummy0;
  tmp.dummy1 = stats.dummy1;
  tmp.dummy2 = stats.dummy2;
  mavlink_msg_openhd_stats_wb_video_air_encode(system_id, component_id, &msg.m,
                                               &tmp);
  return msg;
}

static MavlinkMessage pack_vid_air_fec_performance(
    const uint8_t system_id, const uint8_t component_id,
    const openhd::link_statistics::
        Xmavlink_openhd_stats_wb_video_air_fec_performance_t& stats) {
  MavlinkMessage msg;
  mavlink_openhd_stats_wb_video_air_fec_performance_t tmp{};
  tmp.link_index = stats.link_index;
  tmp.curr_fec_encode_time_avg_us = stats.curr_fec_encode_time_avg_us;
  tmp.curr_fec_encode_time_min_us = stats.curr_fec_encode_time_min_us;
  tmp.curr_fec_encode_time_max_us = stats.curr_fec_encode_time_max_us;
  tmp.curr_fec_block_size_avg = stats.curr_fec_block_size_avg;
  tmp.curr_fec_block_size_min = stats.curr_fec_block_size_min;
  tmp.curr_fec_block_size_max = stats.curr_fec_block_size_max;
  tmp.curr_tx_delay_min_us = stats.curr_tx_delay_min_us;
  tmp.curr_tx_delay_max_us = stats.curr_tx_delay_max_us;
  tmp.curr_tx_delay_avg_us = stats.curr_tx_delay_avg_us;
  mavlink_msg_openhd_stats_wb_video_air_fec_performance_encode(
      system_id, component_id, &msg.m, &tmp);
  return msg;
}

static MavlinkMessage pack_vid_gnd(
    const uint8_t system_id, const uint8_t component_id,
    const openhd::link_statistics::Xmavlink_openhd_stats_wb_video_ground_t&
        stats) {
  MavlinkMessage msg;
  mavlink_openhd_stats_wb_video_ground_t tmp{};
  tmp.link_index = stats.link_index;
  tmp.curr_incoming_bitrate = stats.curr_incoming_bitrate;
  tmp.count_blocks_total = stats.count_blocks_total;
  tmp.count_blocks_lost = stats.count_blocks_lost;
  tmp.count_blocks_recovered = stats.count_blocks_recovered;
  tmp.count_fragments_recovered = stats.count_fragments_recovered;
  // tmp.unused0=stats.unused0;
  // tmp.unused1=stats.unused1;
  mavlink_msg_openhd_stats_wb_video_ground_encode(system_id, component_id,
                                                  &msg.m, &tmp);
  return msg;
}

static MavlinkMessage pack_vid_gnd_fec_performance(
    const uint8_t system_id, const uint8_t component_id,
    const openhd::link_statistics::
        Xmavlink_openhd_stats_wb_video_ground_fec_performance_t& stats) {
  MavlinkMessage msg;
  mavlink_openhd_stats_wb_video_ground_fec_performance_t tmp{};
  tmp.link_index = stats.link_index;
  tmp.curr_fec_decode_time_avg_us = stats.curr_fec_decode_time_avg_us;
  tmp.curr_fec_decode_time_min_us = stats.curr_fec_decode_time_min_us;
  tmp.curr_fec_decode_time_max_us = stats.curr_fec_decode_time_max_us;
  // tmp.unused0=stats.unused0;
  // tmp.unused1=stats.unused1;
  mavlink_msg_openhd_stats_wb_video_ground_fec_performance_encode(
      system_id, component_id, &msg.m, &tmp);
  return msg;
}

static MavlinkMessage pack_camera_stats(
    const uint8_t system_id, const uint8_t component_id,
    const openhd::LinkActionHandler::CamInfo& cam_info) {
  MavlinkMessage msg;
  mavlink_openhd_camera_status_air_t tmp{};
  tmp.cam_type = cam_info.cam_type;
  tmp.encoding_format = cam_info.encoding_format;
  tmp.air_recording_active = cam_info.air_recording_active;
  tmp.encoding_bitrate_kbits = cam_info.encoding_bitrate_kbits;
  tmp.encoding_keyframe_interval = cam_info.encoding_keyframe_interval;
  tmp.stream_w = cam_info.stream_w;
  tmp.stream_h = cam_info.stream_h;
  tmp.stream_fps = cam_info.stream_fps;
  tmp.encoding_format = cam_info.encoding_format;
  tmp.cam_status = cam_info.cam_status;
  tmp.supports_variable_bitrate = cam_info.supports_variable_bitrate;
  mavlink_msg_openhd_camera_status_air_encode(system_id, component_id, &msg.m,
                                              &tmp);
  return msg;
}
static MavlinkMessage pack_mavlink_openhd_wifbroadcast_gnd_operating_mode(
    const uint8_t system_id, const uint8_t component_id,
    const openhd::link_statistics::
        Xmavlink_openhd_wifbroadcast_gnd_operating_mode_t& stats) {
  MavlinkMessage msg;
  mavlink_openhd_wifbroadcast_gnd_operating_mode_t tmp{};
  tmp.operating_mode = stats.operating_mode;
  tmp.tx_passive_mode_is_enabled = stats.tx_passive_mode_is_enabled;
  mavlink_msg_openhd_wifbroadcast_gnd_operating_mode_encode(
      system_id, component_id, &msg.m, &tmp);
  return msg;
}

static MavlinkMessage generate_msg_openhd_wifibroadcast_supported_channels(
    const uint8_t system_id, const uint8_t component_id,
    const std::vector<uint16_t>& channels) {
  MavlinkMessage msg;
  mavlink_openhd_wifbroadcast_supported_channels_t tmp{};
  for (int i = 0; i < 60; i++) {
    if (i < channels.size()) {
      tmp.channels[i] = channels[i];
    } else {
      tmp.channels[i] = 0;
    }
  }
  mavlink_msg_openhd_wifbroadcast_supported_channels_encode(
      system_id, component_id, &msg.m, &tmp);
  return msg;
}

static MavlinkMessage generate_msg_analyze_channels_progress(
    const uint8_t system_id, const uint8_t component_id,
    const openhd::LinkActionHandler::AnalyzeChannelsResult progress) {
  MavlinkMessage msg;
  mavlink_openhd_wifbroadcast_analyze_channels_progress_t tmp{};
  static_assert(sizeof(tmp.channels_mhz) == sizeof(progress.channels_mhz));
  static_assert(sizeof(tmp.foreign_packets) ==
                sizeof(progress.foreign_packets));
  memcpy(tmp.channels_mhz, progress.channels_mhz.data(),
         sizeof(tmp.channels_mhz));
  memcpy(tmp.foreign_packets, progress.foreign_packets.data(),
         sizeof(tmp.foreign_packets));
  tmp.progress_perc = progress.progress;
  mavlink_msg_openhd_wifbroadcast_analyze_channels_progress_encode(
      system_id, component_id, &msg.m, &tmp);
  return msg;
}

static MavlinkMessage generate_msg_scan_channels_progress(
    const uint8_t system_id, const uint8_t component_id,
    const openhd::LinkActionHandler::ScanChannelsProgress progress) {
  MavlinkMessage msg;
  mavlink_openhd_wifbroadcast_scan_channels_progress_t tmp{};
  tmp.progress = progress.progress;
  tmp.channel_mhz = progress.channel_mhz;
  tmp.channel_width_mhz = progress.channel_width_mhz;
  tmp.success = progress.success;
  mavlink_msg_openhd_wifbroadcast_scan_channels_progress_encode(
      system_id, component_id, &msg.m, &tmp);
  return msg;
}

static MavlinkMessage generate_sys_status1(
    const uint8_t system_id, const uint8_t component_id,
    openhd::LinkActionHandler& action_handler) {
  MavlinkMessage msg;
  mavlink_openhd_sys_status1_t tmp{};
  tmp.wifi_hotspot_state = action_handler.m_wifi_hotspot_state;
  tmp.wifi_hotspot_frequency = action_handler.m_wifi_hotspot_frequency;
  tmp.ethernet_hotspot_state = action_handler.m_ethernet_hotspot_state;
  tmp.external_devices_count =
      openhd::ExternalDeviceManager::instance().get_external_device_count();
  mavlink_msg_openhd_sys_status1_encode(system_id, component_id, &msg.m, &tmp);
  return msg;
}

}  // namespace openhd::LinkStatisticsHelper
#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_OHDLINKSTATISTICSHELPER_H_
