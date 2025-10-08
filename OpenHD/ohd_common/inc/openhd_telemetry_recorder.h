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
#include <fstream>
#include <memory>
#include <mutex>
#include <string>

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

 private:
  [[nodiscard]] std::string create_filename_timestamp(
      std::chrono::system_clock::time_point tp) const;
  [[nodiscard]] std::string create_entry_timestamp(
      std::chrono::system_clock::time_point tp) const;
  void ensure_stream_is_ready();
  [[nodiscard]] std::string stats_to_json_string(
      const link_statistics::StatsAirGround& stats,
      const std::string& timestamp) const;

  std::mutex m_mutex;
  std::ofstream m_stream;
  std::string m_file_path;
  bool m_stream_ready = false;
  bool m_failed_once = false;
  std::shared_ptr<spdlog::logger> m_console;
};

}  // namespace openhd

#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_TELEMETRY_RECORDER_H_
