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

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_TELEMETRY_RECORDER_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_TELEMETRY_RECORDER_H_

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>

#include <nlohmann/json.hpp>

#include "openhd_link_statistics.hpp"

namespace spdlog {
class logger;
}

namespace openhd {

class TelemetryRecorder {
 public:
  TelemetryRecorder();
  TelemetryRecorder(const TelemetryRecorder&) = delete;
  TelemetryRecorder& operator=(const TelemetryRecorder&) = delete;
  TelemetryRecorder(TelemetryRecorder&&) = delete;
  TelemetryRecorder& operator=(TelemetryRecorder&&) = delete;

  static TelemetryRecorder& instance();

  void record(const link_statistics::StatsAirGround& stats);
  void record_fc_mavlink_message(uint8_t sysid, uint8_t compid, uint32_t msgid,
                                 uint8_t sequence, const uint8_t* payload,
                                 std::size_t payload_length);

 private:
  [[nodiscard]] std::string create_filename_timestamp(
      std::chrono::system_clock::time_point tp) const;
  [[nodiscard]] std::string create_entry_timestamp(
      std::chrono::system_clock::time_point tp) const;
  void ensure_stream_is_ready();
  [[nodiscard]] nlohmann::json stats_to_json(
      const link_statistics::StatsAirGround& stats,
      const std::string& timestamp) const;
  [[nodiscard]] std::string bytes_to_hex(const uint8_t* data,
                                         std::size_t length) const;
  void write_json_line(const nlohmann::json& json_line);

  std::mutex m_mutex;
  std::ofstream m_stream;
  std::string m_file_path;
  bool m_stream_ready = false;
  bool m_failed_once = false;
  std::shared_ptr<spdlog::logger> m_console;
};

}  // namespace openhd

#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_TELEMETRY_RECORDER_H_
