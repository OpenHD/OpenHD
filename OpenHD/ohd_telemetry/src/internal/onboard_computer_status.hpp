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

#ifndef XMAVLINKSERVICE_SYSTEMREADUTIL_H
#define XMAVLINKSERVICE_SYSTEMREADUTIL_H

#include <chrono>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

#include "ina219.h"
#include "mav_include.h"
#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"
#include "cpu_usage.hpp"

// Namespace for util methods regarding onboard computer status
namespace openhd::onboard {

// from
// https://github.com/OpenHD/Open.HD/blob/35b6b10fbeda43cd06bbfbd90e2daf29629c2f8a/openhd-status/src/statusmicroservice.cpp#L165
// Return the CPU/SOC temperature of the system or 0 if not available
// Unit: Degree ?
static int readTemperature() {
  auto temp_file_opt = OHDFilesystemUtil::opt_read_file(
      "/sys/class/hwmon/hwmon0/temp1_input", false);
  if (!temp_file_opt.has_value()) {
    temp_file_opt = OHDFilesystemUtil::opt_read_file(
        "/sys/class/thermal/thermal_zone0/temp", false);
    if (!temp_file_opt.has_value()) {
      return 0;
    }
  }
  auto temp = OHDUtil::string_to_int(temp_file_opt.value());
  if (!temp.has_value()) return 0;
  return temp.value() / 1000;
}

// The status thread samples once per second. Read aggregate kernel counters
// directly: top's saved per-core/graph layout and locale vary across images.
static std::optional<int> read_cpuload_once_blocking() {
  static thread_local std::optional<CpuTimes> previous;
  std::ifstream input("/proc/stat");
  const auto current = parse_cpu_times(input);
  if (!current) return {};
  const auto usage = previous ? cpu_usage_between(*previous, *current)
                              : std::optional<int>{};
  previous = current;
  return usage;
}

// Taken from ChatGPT
struct RamUsage {
  double ram_usage_perc;
  int ram_total_mb;
};
static RamUsage calculate_memory_usage_percent() {
  try {
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    std::regex pattern("\\s+");
    long long total_memory = 0;
    long long free_memory = 0;
    long long used_memory = 0;
    while (std::getline(meminfo, line)) {
      std::vector<std::string> fields;
      auto words_begin =
          std::sregex_token_iterator(line.begin(), line.end(), pattern, -1);
      auto words_end = std::sregex_token_iterator();
      for (auto i = words_begin; i != words_end; ++i) {
        fields.push_back(*i);
      }
      if (fields[0] == "MemTotal:") {
        total_memory = std::stoll(fields[1]);
      } else if (fields[0] == "MemFree:") {
        free_memory = std::stoll(fields[1]);
      }
    }
    meminfo.close();
    used_memory = total_memory - free_memory;
    double memory_usage = 100.0 * used_memory / total_memory;
    /*std::cout << "Total Memory: " << total_memory / 1024 << " MB" <<
    std::endl; std::cout << "Used Memory: " << used_memory / 1024 << " MB" <<
    std::endl; std::cout << "Free Memory: " << free_memory / 1024 << " MB" <<
    std::endl; std::cout << "Memory Usage: " << memory_usage << "%" <<
    std::endl;*/
    return RamUsage{memory_usage, (int)total_memory};
  } catch (std::exception& e) {
    return RamUsage{0, 0};
  }
}

}  // namespace openhd::onboard
#endif  // XMAVLINKSERVICE_SYSTEMREADUTIL_H
