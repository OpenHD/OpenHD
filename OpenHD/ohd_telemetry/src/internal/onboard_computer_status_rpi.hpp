//
// Created by consti10 on 21.08.23.
//

#ifndef OPENHD_ONBOARD_COMPUTER_STATUS_RPI_H
#define OPENHD_ONBOARD_COMPUTER_STATUS_RPI_H

#include <chrono>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

#include "ina219.h"
#include "mav_include.h"
#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

// Stuff that works only on rpi
namespace openhd::onboard::rpi {

// copy and paste from QOpenHD, I think we can get the under-voltage warning on
// rpi this way.
static int readUnderVoltError() {
  auto undervolt_opt =
      OHDFilesystemUtil::opt_read_file("/tmp/undervolt", false);
  if (!undervolt_opt.has_value()) return 0;
  auto value = OHDUtil::string_to_int(undervolt_opt.value());
  ;
  if (!value.has_value()) return 0;
  return value.value();
}

// https://www.elinux.org/RPI_vcgencmd_usage
//
// most vcgen commands return "blablabla=wanted"
// where wanted is what we are acutally after
static std::string everything_after_equal(const std::string& unparsed) {
  // x.substr(x.find(":") + 1);
  const auto npos = unparsed.find("=");
  if (npos != std::string::npos) {
    return unparsed.substr(npos + 1);
  }
  openhd::log::get_default()->warn(
      "everything_after_equal - no equal sign found");
  return unparsed;
}

static float vcgencmd_result_parse_float(const std::string& result) {
  const auto tmp = rpi::everything_after_equal(result);
  return OHDUtil::string_to_float(tmp).value_or(0);
}
static long vcgencmd_result_parse_long(const std::string& result) {
  const auto tmp = rpi::everything_after_equal(result);
  return OHDUtil::string_to_long(tmp).value_or(0);
}

static int8_t read_temperature_soc_degree() {
  int8_t ret = -1;
  const auto vcgencmd_measure_temp_opt =
      OHDUtil::run_command_out("vcgencmd measure_temp");
  // const auto
  // vcgencmd_measure_temp_opt=std::optional<std::string>("temp=47.2'C");
  if (!vcgencmd_measure_temp_opt.has_value()) {
    return ret;
  }
  const auto tmp_float =
      vcgencmd_result_parse_float(vcgencmd_measure_temp_opt.value());
  return static_cast<int8_t>(lround(tmp_float));
}
static constexpr auto VCGENCMD_CLOCK_CPU = "arm";
static constexpr auto VCGENCMD_CLOCK_ISP = "isp";
static constexpr auto VCGENCMD_CLOCK_H264 = "h264";
static constexpr auto VCGENCMD_CLOCK_CORE = "core";
static constexpr auto VCGENCMD_CLOCK_V3D = "v3d";
// See https://elinux.org/RPI_vcgencmd_usage
// Shows clock frequency, clock can be one of arm, core, h264, isp, v3d, uart,
// pwm, emmc, pixel, vec, hdmi, dpi. NOTE: vcgencmd returns values in hertz, use
// the "mhz" util for more easy to read values.
static int vcgencmd_measure_clock(const std::string& which) {
  int ret = -1;
  const auto vcgencmd_result =
      OHDUtil::run_command_out(fmt::format("vcgencmd measure_clock {}", which));
  if (!vcgencmd_result.has_value()) {
    return ret;
  }
  const auto tmp2 = vcgencmd_result_parse_long(vcgencmd_result.value());
  return static_cast<int>(tmp2);
}
static int read_curr_frequency_mhz(const std::string& which) {
  return static_cast<uint16_t>(vcgencmd_measure_clock(which) / 1000 / 1000);
}

// Returns true if rpi currently has undervolt flag set
static bool vcgencmd_get_undervolt() {
  const auto opt_vcgencmd_result =
      OHDUtil::run_command_out(fmt::format("vcgencmd get_throttled"));
  if (!opt_vcgencmd_result.has_value()) {
    openhd::log::get_default()->debug("Cannot get vcgencmd throttled");
    return false;  // we don't know
  }
  const std::string& vcgencmd_result = opt_vcgencmd_result.value();
  const auto tmp = rpi::everything_after_equal(vcgencmd_result);
  const auto value_opt = OHDUtil::string_to_long_hex(tmp);
  if (!value_opt.has_value()) {
    return false;  // we don't know
  }
  const long value = value_opt.value();
  const auto undervolt_bit = OHDUtil::get_nth_bit(value_opt.value(), 0);
  // openhd::log::get_default()->debug("Undervolt {}/{} {:x} bit
  // set:{}",vcgencmd_result,value,value,undervolt_bit);
  return undervolt_bit;
}

}  // namespace openhd::onboard::rpi

#endif  // OPENHD_ONBOARD_COMPUTER_STATUS_RPI_H
