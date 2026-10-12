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

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_HPP_

#include <cstring>
#include "mavlink_statistics.hpp"
#include <optional>
#include <sstream>
#include <string>

// Wire fields are generated from the OpenHD MAVLink XML without a transport
// dependency. Only application-only state and helpers are declared here.

namespace openhd::link_statistics {

using Xmavlink_openhd_stats_monitor_mode_wifi_link_t = mavlink_types::openhd_stats_monitor_mode_wifi_link;
struct Xmavlink_openhd_stats_telemetry_t : mavlink_types::openhd_stats_telemetry {
  [[nodiscard]] std::string to_string() const {
    std::stringstream ss;
    ss << "StatsTelemetry{";
    ss << "curr_tx_pps:" << curr_tx_pps << ",curr_rx_pps:" << curr_rx_pps
       << ",curr_tx_bps:" << curr_tx_bps << ", curr_rx_bps:" << curr_rx_bps;
    ss << ", curr_rx_packet_loss_perc" << curr_rx_packet_loss_perc;
    ss << "}";
    return ss.str();
  }
};
using Xmavlink_openhd_stats_wb_video_air_t = mavlink_types::openhd_stats_wb_video_air;
using Xmavlink_openhd_stats_wb_video_air_fec_performance_t = mavlink_types::openhd_stats_wb_video_air_fec_performance;
using Xmavlink_openhd_stats_wb_video_ground_t = mavlink_types::openhd_stats_wb_video_ground;
using Xmavlink_openhd_stats_wb_video_ground_fec_performance_t = mavlink_types::openhd_stats_wb_video_ground_fec_performance;
struct Xmavlink_openhd_stats_monitor_mode_wifi_card_t : mavlink_types::openhd_stats_monitor_mode_wifi_card {
  bool NON_MAVLINK_CARD_ACTIVE = false;
  uint8_t card_sub_type{};
};
struct Xmavlink_openhd_wifbroadcast_gnd_operating_mode_t : mavlink_types::openhd_wifbroadcast_gnd_operating_mode {
  uint16_t extra_channel_mhz{};
  uint8_t extra_channel_width_mhz{};
  uint8_t progress{};
  uint8_t success{};
};

// Stats per connected card
using StatsAllCards =
    std::array<Xmavlink_openhd_stats_monitor_mode_wifi_card_t, 4>;

struct StatsAirGround {
  bool is_air = false;
  bool ready = false;
  // air and ground
  Xmavlink_openhd_stats_monitor_mode_wifi_link_t monitor_mode_link;
  Xmavlink_openhd_stats_telemetry_t telemetry;
  StatsAllCards cards;
  // for air
  std::vector<Xmavlink_openhd_stats_wb_video_air_t> stats_wb_video_air;
  Xmavlink_openhd_stats_wb_video_air_fec_performance_t air_fec_performance;
  // for ground
  std::vector<Xmavlink_openhd_stats_wb_video_ground_t> stats_wb_video_ground;
  Xmavlink_openhd_stats_wb_video_ground_fec_performance_t gnd_fec_performance;
  Xmavlink_openhd_wifbroadcast_gnd_operating_mode_t gnd_operating_mode;
};

typedef std::function<void(StatsAirGround all_stats)> STATS_CALLBACK;

// Bit field for boolean only value(s)
struct MonitorModeLinkBitfield {
  unsigned int stbc : 1;
  unsigned int lpdc : 1;
  unsigned int short_guard : 1;
  unsigned int curr_rx_last_packet_status_good : 1;
  // Set when this message is emitted by ArtosynLink.
  unsigned int artosyn_link_active : 1;
  // Set when monitor_mode_link dummy fields carry Artosyn debug metrics.
  unsigned int artosyn_debug_stats_present : 1;
  unsigned int unused : 2;
} __attribute__((packed));
static_assert(sizeof(MonitorModeLinkBitfield) == 1);
static uint8_t write_monitor_link_bitfield(
    const MonitorModeLinkBitfield& bitfield) {
  uint8_t ret;
  std::memcpy(&ret, (uint8_t*)&bitfield, 1);
  return ret;
}
static MonitorModeLinkBitfield parse_monitor_link_bitfield(uint8_t bitfield) {
  MonitorModeLinkBitfield ret{};
  std::memcpy((uint8_t*)&ret, &bitfield, 1);
  return ret;
}

using VALUE_BITRATE = uint16_t;
using VALUE_DELAY = uint16_t;
// User can change: BW, MCS, FREQUENCY, RESILIENCY_MODE
struct LinkStatusAir {
  uint8_t mcs_index;
  uint8_t bw_mhz;
  uint16_t frequency;
  uint8_t throttle;
  uint8_t resiliency_mode;
  // Video bitrate (before FEC, ...)
  VALUE_BITRATE video_bitrate;
  VALUE_DELAY cam0_tx_delay;
  uint8_t cam0_dropped_frames;
  VALUE_DELAY cam1_tx_delay;
  uint8_t cam1_dropped_frames;
  uint8_t rx_loss_percent;
};
struct LinkStatusGround {};

}  // namespace openhd::link_statistics
#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_HPP_
