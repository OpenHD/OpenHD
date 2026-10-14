/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_OHDLINKSTATISTICSHELPER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_OHDLINKSTATISTICSHELPER_H_

#include "../mav_include.h"
#include "../mavlink_statistics_encode.hpp"
#include "openhd_action_handler.h"
#include "openhd_external_device.h"
#include "openhd_link_statistics.hpp"

#include <algorithm>
#include <limits>

namespace openhd::LinkStatisticsHelper {

static MavlinkMessage pack_card(
    const uint8_t system_id, const uint8_t component_id, int card_index,
    const openhd::link_statistics::
        Xmavlink_openhd_stats_monitor_mode_wifi_card_t& card_stats) {
  MavlinkMessage msg;
  auto tmp = openhd::mavlink_types::to_mavlink(card_stats);
  tmp.card_index = card_index;
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
  auto tmp = openhd::mavlink_types::to_mavlink(stats_monitor_mode_link);
  mavlink_msg_openhd_stats_monitor_mode_wifi_link_encode(
      system_id, component_id, &msg.m, &tmp);
  return msg;
}

static MavlinkMessage pack_tele(
    const uint8_t system_id, const uint8_t component_id,
    const openhd::link_statistics::Xmavlink_openhd_stats_telemetry_t& stats) {
  MavlinkMessage msg;
  auto tmp = openhd::mavlink_types::to_mavlink(stats);
  tmp.dummy0 = 0; tmp.dummy1 = 0; tmp.dummy2 = 0;
  mavlink_msg_openhd_stats_telemetry_encode(system_id, component_id, &msg.m,
                                            &tmp);
  return msg;
}

static MavlinkMessage pack_vid_air(
    const uint8_t system_id, const uint8_t component_id,
    const openhd::link_statistics::Xmavlink_openhd_stats_wb_video_air_t&
        stats) {
  MavlinkMessage msg;
  auto tmp = openhd::mavlink_types::to_mavlink(stats);
  mavlink_msg_openhd_stats_wb_video_air_encode(system_id, component_id, &msg.m,
                                               &tmp);
  return msg;
}

static MavlinkMessage pack_vid_air_fec_performance(
    const uint8_t system_id, const uint8_t component_id,
    const openhd::link_statistics::
        Xmavlink_openhd_stats_wb_video_air_fec_performance_t& stats) {
  MavlinkMessage msg;
  auto tmp = openhd::mavlink_types::to_mavlink(stats);
  tmp.dummy0 = 0; tmp.dummy1 = 0; tmp.dummy2 = 0;
  mavlink_msg_openhd_stats_wb_video_air_fec_performance_encode(
      system_id, component_id, &msg.m, &tmp);
  return msg;
}

static MavlinkMessage pack_vid_gnd(
    const uint8_t system_id, const uint8_t component_id,
    const openhd::link_statistics::Xmavlink_openhd_stats_wb_video_ground_t&
        stats) {
  MavlinkMessage msg;
  auto tmp = openhd::mavlink_types::to_mavlink(stats);
  mavlink_msg_openhd_stats_wb_video_ground_encode(system_id, component_id,
                                                  &msg.m, &tmp);
  return msg;
}

static MavlinkMessage pack_vid_gnd_fec_performance(
    const uint8_t system_id, const uint8_t component_id,
    const openhd::link_statistics::
        Xmavlink_openhd_stats_wb_video_ground_fec_performance_t& stats) {
  MavlinkMessage msg;
  auto tmp = openhd::mavlink_types::to_mavlink(stats);
  tmp.dummy0 = 0; tmp.dummy1 = 0; tmp.dummy2 = 0;
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
  tmp.encoding_bitrate_kbits = cam_info.target_bitrate_kbits > 0
                                   ? cam_info.target_bitrate_kbits
                                   : cam_info.encoding_bitrate_kbits;
  tmp.encoding_keyframe_interval = cam_info.encoding_keyframe_interval;
  tmp.stream_w = cam_info.stream_w;
  tmp.stream_h = cam_info.stream_h;
  tmp.stream_fps = cam_info.stream_fps;
  tmp.encoding_format = cam_info.encoding_format;
  tmp.cam_status = cam_info.cam_status;
  tmp.supports_variable_bitrate = cam_info.supports_variable_bitrate;
  tmp.dummy2 = static_cast<int32_t>(
      std::min<uint32_t>(cam_info.measured_bitrate_bps,
                         static_cast<uint32_t>(
                             std::numeric_limits<int32_t>::max())));
  tmp.dummy1 = static_cast<int16_t>(
      std::min<uint16_t>(cam_info.measured_fps,
                         static_cast<uint16_t>(
                             std::numeric_limits<int16_t>::max())));
  mavlink_msg_openhd_camera_status_air_encode(system_id, component_id, &msg.m,
                                              &tmp);
  return msg;
}
static MavlinkMessage pack_mavlink_openhd_wifbroadcast_gnd_operating_mode(
    const uint8_t system_id, const uint8_t component_id,
    const openhd::link_statistics::
        Xmavlink_openhd_wifbroadcast_gnd_operating_mode_t& stats) {
  MavlinkMessage msg;
  auto tmp = openhd::mavlink_types::to_mavlink(stats);
  tmp.dummy0 = 0; tmp.dummy1 = 0; tmp.dummy2 = 0;
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
  if (progress.rf_sampled_busy) {
    tmp.dummy1=progress.rf_batch_id;
    tmp.dummy2=(static_cast<uint32_t>(progress.rf_total_channels)<<16)|progress.rf_page_offset;
    tmp.dummy0=0x4553;  // ESP RF metric, not foreign packet counts.
    memcpy(tmp.dummy,progress.rf_busy_centipercent.data(),sizeof(tmp.dummy));
  }
  if (progress.devourer_survey) {
    // Versioned use of reserved fields; the MAVLink layout/CRC is unchanged.
    // Per channel: bit15 NHM valid, bit14 CLM valid, bits7..13 NHM env %, bits0..6 busy %.
    tmp.dummy0 = 0x4456;
    tmp.dummy1 = progress.rf_batch_id;
    tmp.dummy2 = (static_cast<uint32_t>(progress.rf_total_channels) << 16) | progress.rf_page_offset;
    memcpy(tmp.dummy, progress.survey_metrics.data(), sizeof(tmp.dummy));
  }
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
