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

#include "openhd_telemetry_recorder.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include <nlohmann/json.hpp>
#include "openhd_settings_directories.h"
#include "openhd_spdlog.h"
#include "openhd_util_filesystem.h"

namespace {

std::tm to_local_time(std::time_t time) {
  std::tm tm{};
#if defined(_WIN32)
  localtime_s(&tm, &time);
#else
  localtime_r(&time, &tm);
#endif
  return tm;
}

nlohmann::json telemetry_to_json(
    const openhd::link_statistics::Xmavlink_openhd_stats_telemetry_t& telemetry) {
  nlohmann::json j;
  j["curr_tx_bps"] = telemetry.curr_tx_bps;
  j["curr_rx_bps"] = telemetry.curr_rx_bps;
  j["dummy2"] = telemetry.dummy2;
  j["curr_tx_pps"] = telemetry.curr_tx_pps;
  j["curr_rx_pps"] = telemetry.curr_rx_pps;
  j["curr_rx_packet_loss_perc"] = telemetry.curr_rx_packet_loss_perc;
  j["dummy1"] = telemetry.dummy1;
  j["dummy0"] = static_cast<int>(telemetry.dummy0);
  return j;
}

nlohmann::json monitor_link_to_json(
    const openhd::link_statistics::Xmavlink_openhd_stats_monitor_mode_wifi_link_t& link) {
  nlohmann::json j;
  j["curr_tx_bps"] = link.curr_tx_bps;
  j["curr_rx_bps"] = link.curr_rx_bps;
  j["count_tx_inj_error_hint"] = link.count_tx_inj_error_hint;
  j["count_tx_dropped_packets"] = link.count_tx_dropped_packets;
  j["dummy2"] = link.dummy2;
  j["curr_tx_pps"] = link.curr_tx_pps;
  j["curr_rx_pps"] = link.curr_rx_pps;
  j["curr_rx_big_gaps_counter"] = link.curr_rx_big_gaps_counter;
  j["curr_tx_channel_mhz"] = link.curr_tx_channel_mhz;
  j["curr_rate_kbits"] = link.curr_rate_kbits;
  j["dummy1"] = link.dummy1;
  j["curr_rx_packet_loss_perc"] = static_cast<int>(link.curr_rx_packet_loss_perc);
  j["curr_tx_channel_w_mhz"] = static_cast<int>(link.curr_tx_channel_w_mhz);
  j["curr_tx_mcs_index"] = static_cast<int>(link.curr_tx_mcs_index);
  j["curr_n_rate_adjustments"] = static_cast<int>(link.curr_n_rate_adjustments);
  j["bitfield"] = static_cast<int>(link.bitfield);
  j["pollution_perc"] = static_cast<int>(link.pollution_perc);
  j["dummy0"] = static_cast<int>(link.dummy0);
  return j;
}

nlohmann::json card_stats_to_json(
    const openhd::link_statistics::Xmavlink_openhd_stats_monitor_mode_wifi_card_t& card) {
  nlohmann::json j;
  j["NON_MAVLINK_CARD_ACTIVE"] = card.NON_MAVLINK_CARD_ACTIVE;
  j["count_p_received"] = card.count_p_received;
  j["count_p_injected"] = card.count_p_injected;
  j["dummy2"] = card.dummy2;
  j["tx_power_current"] = card.tx_power_current;
  j["tx_power_armed"] = card.tx_power_armed;
  j["tx_power_disarmed"] = card.tx_power_disarmed;
  j["dummy1"] = card.dummy1;
  j["card_index"] = static_cast<int>(card.card_index);
  j["card_type"] = static_cast<int>(card.card_type);
  j["card_sub_type"] = static_cast<int>(card.card_sub_type);
  j["tx_active"] = static_cast<int>(card.tx_active);
  j["rx_rssi"] = static_cast<int>(card.rx_rssi);
  j["rx_rssi_1"] = static_cast<int>(card.rx_rssi_1);
  j["rx_rssi_2"] = static_cast<int>(card.rx_rssi_2);
  j["rx_noise_adapter"] = static_cast<int>(card.rx_noise_adapter);
  j["rx_noise_antenna1"] = static_cast<int>(card.rx_noise_antenna1);
  j["rx_noise_antenna2"] = static_cast<int>(card.rx_noise_antenna2);
  j["rx_signal_quality_adapter"] = static_cast<int>(card.rx_signal_quality_adapter);
  j["rx_signal_quality_antenna1"] = static_cast<int>(card.rx_signal_quality_antenna1);
  j["rx_signal_quality_antenna2"] = static_cast<int>(card.rx_signal_quality_antenna2);
  j["curr_rx_packet_loss_perc"] = static_cast<int>(card.curr_rx_packet_loss_perc);
  j["curr_status"] = static_cast<int>(card.curr_status);
  j["dummy0"] = static_cast<int>(card.dummy0);
  return j;
}

nlohmann::json video_air_stats_to_json(
    const openhd::link_statistics::Xmavlink_openhd_stats_wb_video_air_t& stats) {
  nlohmann::json j;
  j["curr_measured_encoder_bitrate"] = stats.curr_measured_encoder_bitrate;
  j["curr_injected_bitrate"] = stats.curr_injected_bitrate;
  j["curr_injected_pps"] = stats.curr_injected_pps;
  j["curr_dropped_frames"] = stats.curr_dropped_frames;
  j["dummy2"] = stats.dummy2;
  j["curr_recommended_bitrate"] = stats.curr_recommended_bitrate;
  j["curr_fec_percentage"] = stats.curr_fec_percentage;
  j["dummy1"] = stats.dummy1;
  j["link_index"] = stats.link_index;
  j["dummy0"] = static_cast<int>(stats.dummy0);
  return j;
}

nlohmann::json video_air_fec_to_json(
    const openhd::link_statistics::
        Xmavlink_openhd_stats_wb_video_air_fec_performance_t& stats) {
  nlohmann::json j;
  j["curr_fec_encode_time_avg_us"] = stats.curr_fec_encode_time_avg_us;
  j["curr_fec_encode_time_min_us"] = stats.curr_fec_encode_time_min_us;
  j["curr_fec_encode_time_max_us"] = stats.curr_fec_encode_time_max_us;
  j["dummy2"] = stats.dummy2;
  j["curr_fec_block_size_avg"] = stats.curr_fec_block_size_avg;
  j["curr_fec_block_size_min"] = stats.curr_fec_block_size_min;
  j["curr_fec_block_size_max"] = stats.curr_fec_block_size_max;
  j["curr_tx_delay_avg_us"] = stats.curr_tx_delay_avg_us;
  j["curr_tx_delay_min_us"] = stats.curr_tx_delay_min_us;
  j["curr_tx_delay_max_us"] = stats.curr_tx_delay_max_us;
  j["dummy1"] = stats.dummy1;
  j["link_index"] = stats.link_index;
  j["dummy0"] = static_cast<int>(stats.dummy0);
  return j;
}

nlohmann::json video_ground_stats_to_json(
    const openhd::link_statistics::Xmavlink_openhd_stats_wb_video_ground_t& stats) {
  nlohmann::json j;
  j["curr_incoming_bitrate"] = stats.curr_incoming_bitrate;
  j["count_blocks_total"] = stats.count_blocks_total;
  j["count_blocks_lost"] = stats.count_blocks_lost;
  j["count_blocks_recovered"] = stats.count_blocks_recovered;
  j["count_fragments_recovered"] = stats.count_fragments_recovered;
  j["dummy2"] = stats.dummy2;
  j["dummy1"] = stats.dummy1;
  j["link_index"] = stats.link_index;
  j["dummy0"] = static_cast<int>(stats.dummy0);
  return j;
}

nlohmann::json video_ground_fec_to_json(
    const openhd::link_statistics::
        Xmavlink_openhd_stats_wb_video_ground_fec_performance_t& stats) {
  nlohmann::json j;
  j["curr_fec_decode_time_avg_us"] = stats.curr_fec_decode_time_avg_us;
  j["curr_fec_decode_time_min_us"] = stats.curr_fec_decode_time_min_us;
  j["curr_fec_decode_time_max_us"] = stats.curr_fec_decode_time_max_us;
  j["dummy2"] = stats.dummy2;
  j["dummy1"] = stats.dummy1;
  j["link_index"] = stats.link_index;
  j["dummy0"] = stats.dummy0;
  return j;
}

nlohmann::json ground_operating_mode_to_json(
    const openhd::link_statistics::Xmavlink_openhd_wifbroadcast_gnd_operating_mode_t& mode) {
  nlohmann::json j;
  j["dummy1"] = mode.dummy1;
  j["dummy2"] = mode.dummy2;
  j["extra_channel_mhz"] = mode.extra_channel_mhz;
  j["dummy0"] = mode.dummy0;
  j["operating_mode"] = static_cast<int>(mode.operating_mode);
  j["extra_channel_width_mhz"] = static_cast<int>(mode.extra_channel_width_mhz);
  j["progress"] = static_cast<int>(mode.progress);
  j["success"] = static_cast<int>(mode.success);
  j["tx_passive_mode_is_enabled"] =
      static_cast<int>(mode.tx_passive_mode_is_enabled);
  return j;
}

}  // namespace

namespace openhd {

TelemetryRecorder& TelemetryRecorder::instance() {
  static TelemetryRecorder instance;
  return instance;
}

TelemetryRecorder::TelemetryRecorder() {
  m_console = openhd::log::create_or_get("tele_rec");
  try {
    openhd::generateSettingsDirectoryIfNonExists();
    const auto directory = openhd::get_telemetry_settings_directory();
    if (!directory.empty()) {
      OHDFilesystemUtil::create_directories(directory);
      const auto now = std::chrono::system_clock::now();
      m_file_path = directory + "telemetry" + create_filename_timestamp(now) + ".ohd";
      m_stream.open(m_file_path, std::ios::out | std::ios::app);
      if (!m_stream.is_open()) {
        m_console->error("Failed to open telemetry recording file {}", m_file_path);
      } else {
        OHDFilesystemUtil::make_file_read_write_everyone(m_file_path);
        m_stream_ready = true;
        m_console->info("Recording telemetry to {}", m_file_path);
      }
    }
  } catch (const std::exception& ex) {
    m_console->error("Exception while creating telemetry recorder: {}", ex.what());
  }
}

std::string TelemetryRecorder::create_filename_timestamp(
    std::chrono::system_clock::time_point tp) const {
  const auto time = std::chrono::system_clock::to_time_t(tp);
  const auto tm = to_local_time(time);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y%m%dT%H%M%S");
  return oss.str();
}

std::string TelemetryRecorder::create_entry_timestamp(
    std::chrono::system_clock::time_point tp) const {
  const auto time = std::chrono::system_clock::to_time_t(tp);
  const auto tm = to_local_time(time);
  const auto subseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()) %
      std::chrono::seconds(1);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
  oss << '.' << std::setfill('0') << std::setw(3) << subseconds.count();
  return oss.str();
}

void TelemetryRecorder::ensure_stream_is_ready() {
  if (m_stream_ready && m_stream.is_open()) {
    return;
  }
  if (!m_failed_once) {
    m_console->warn("Telemetry recorder stream not ready for {}", m_file_path);
    m_failed_once = true;
  }
}

nlohmann::json TelemetryRecorder::stats_to_json(
    const link_statistics::StatsAirGround& stats,
    const std::string& timestamp) const {
  nlohmann::json j;
  j["timestamp"] = timestamp;
  j["type"] = "link_stats";
  j["is_air"] = stats.is_air;
  j["ready"] = stats.ready;
  j["monitor_mode_link"] = monitor_link_to_json(stats.monitor_mode_link);
  j["telemetry"] = telemetry_to_json(stats.telemetry);

  nlohmann::json cards = nlohmann::json::array();
  for (const auto& card : stats.cards) {
    cards.push_back(card_stats_to_json(card));
  }
  j["cards"] = cards;

  nlohmann::json air_stats = nlohmann::json::array();
  for (const auto& entry : stats.stats_wb_video_air) {
    air_stats.push_back(video_air_stats_to_json(entry));
  }
  j["stats_wb_video_air"] = air_stats;
  j["air_fec_performance"] = video_air_fec_to_json(stats.air_fec_performance);

  nlohmann::json ground_stats = nlohmann::json::array();
  for (const auto& entry : stats.stats_wb_video_ground) {
    ground_stats.push_back(video_ground_stats_to_json(entry));
  }
  j["stats_wb_video_ground"] = ground_stats;
  j["gnd_fec_performance"] = video_ground_fec_to_json(stats.gnd_fec_performance);
  j["gnd_operating_mode"] = ground_operating_mode_to_json(stats.gnd_operating_mode);

  return j;
}

void TelemetryRecorder::record(const link_statistics::StatsAirGround& stats) {
  if (!stats.ready) {
    return;
  }
  const auto now = std::chrono::system_clock::now();
  const auto timestamp = create_entry_timestamp(now);
  const auto json_line = stats_to_json(stats, timestamp);
  write_json_line(json_line);
}

std::string TelemetryRecorder::bytes_to_hex(const uint8_t* data,
                                           std::size_t length) const {
  if (data == nullptr || length == 0) {
    return "";
  }
  static constexpr char HEX_DIGITS[] = "0123456789ABCDEF";
  std::string hex;
  hex.reserve(length * 2);
  for (std::size_t i = 0; i < length; ++i) {
    const auto value = data[i];
    hex.push_back(HEX_DIGITS[(value >> 4) & 0x0F]);
    hex.push_back(HEX_DIGITS[value & 0x0F]);
  }
  return hex;
}

void TelemetryRecorder::write_json_line(const nlohmann::json& json_line) {
  std::lock_guard<std::mutex> lock(m_mutex);
  ensure_stream_is_ready();
  if (!m_stream_ready || !m_stream.is_open()) {
    return;
  }
  m_stream << json_line.dump() << '\n';
  m_stream.flush();
}

void TelemetryRecorder::record_fc_mavlink_message(
    uint8_t sysid, uint8_t compid, uint32_t msgid, uint8_t sequence,
    const uint8_t* payload, std::size_t payload_length) {
  const auto now = std::chrono::system_clock::now();
  const auto timestamp = create_entry_timestamp(now);
  nlohmann::json entry;
  entry["timestamp"] = timestamp;
  entry["type"] = "fc_mavlink";
  entry["sysid"] = sysid;
  entry["compid"] = compid;
  entry["msgid"] = msgid;
  entry["sequence"] = sequence;
  entry["payload_length"] = payload_length;
  entry["payload_hex"] = bytes_to_hex(payload, payload_length);
  write_json_line(entry);
}

}  // namespace openhd
