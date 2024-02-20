//
// Created by consti10 on 26.04.22.
//

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

// really stupid, but for now the best solution I came up with
// loosely based on
// https://stackoverflow.com/questions/9229333/how-to-get-overall-cpu-usage-e-g-57-on-linux
// NOTE: top -v returns procps-ng on both pi4 and my ubuntu laptop
// Also note, we want the CPU usage from all processes - not only -p 1
// 28.July 2022: This seems to work on both rpi4 and my ubuntu pc.
// Also, I am pretty sure we can use -bn1 - top should report from "the last
// refresh."
static std::optional<int> read_cpuload_once_blocking() {
  auto res_opt = OHDUtil::run_command_out(
      R"lit(top -bn1 | grep -i '^%cpu')lit");  // ???? cat /proc/loadavg
  // The result from that should look like this: %Cpu(s): 31,0 us,  2,0 sy,  0,0
  // ni, 67,0 id,  0,0 wa,  0,0 hi,  0,0 si,  0,0 st Where "67.0 id" is what we
  // are after - "time spent in the kernel idle handler" from that, we can
  // deduce the usage
  if (!res_opt.has_value()) {
    return std::nullopt;
  }
  const std::string res = res_opt.value();
  // std::cout<<"read_cpuload_once_blocking res:{"<<res<<"}\n";
  std::smatch result;
  const std::regex r1{"ni,(.*) id"};
  auto res1 = std::regex_search(res, result, r1);
  if (!res1 || result.size() < 1) {
    return std::nullopt;
  }
  const std::string intermediate1 = result[0];
  // std::cout<<"Intermediate:{"<<intermediate1<<"}\n";
  if (intermediate1.length() < 3) {
    return std::nullopt;
  }
  std::regex begin("ni,");
  const auto intermediate2 = std::regex_replace(intermediate1, begin, "");
  // std::cout<<"Intermediate2:{"<<intermediate2<<"}\n";
  const auto cpu_idle_perc = std::atof(intermediate2.c_str());
  // std::cout<<"cpu_idle_perc:{"<<cpu_idle_perc<<"}\n";
  const auto cpu_idle_perc_int = static_cast<int>(lround(cpu_idle_perc));
  return 100 - cpu_idle_perc_int;
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
