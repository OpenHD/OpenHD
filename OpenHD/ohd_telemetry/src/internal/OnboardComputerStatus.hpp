//
// Created by consti10 on 26.04.22.
//

#ifndef XMAVLINKSERVICE_SYSTEMREADUTIL_H
#define XMAVLINKSERVICE_SYSTEMREADUTIL_H

#include <boost/filesystem.hpp>
#include <chrono>
#include <regex>
#include <sstream>

#include "mav_include.h"
#include "openhd_spdlog.hpp"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

// https://mavlink.io/en/messages/common.html#ONBOARD_COMPUTER_STATUS
// used to be a custom message for a short amount of time.
namespace OnboardComputerStatus {

// from https://github.com/OpenHD/Open.HD/blob/35b6b10fbeda43cd06bbfbd90e2daf29629c2f8a/openhd-status/src/statusmicroservice.cpp#L165
// Return the CPU/SOC temperature of the system or 0 if not available
// Unit: Degree ?
static int readTemperature() {
  auto temp_file_opt=OHDFilesystemUtil::opt_read_file("/sys/class/thermal/thermal_zone0/temp", false);
  if(!temp_file_opt.has_value())return 0;
  auto temp=OHDUtil::string_to_int(temp_file_opt.value());
  if(!temp.has_value())return 0;
  return temp.value() / 1000;
}

// Stuff that works only on rpi
namespace rpi {

// copy and paste from QOpenHD, I think we can get the under-voltage warning on rpi this way.
static int readUnderVoltError() {
  auto undervolt_opt=OHDFilesystemUtil::opt_read_file("/tmp/undervolt", false);
  if(!undervolt_opt.has_value())return 0;
  auto value=OHDUtil::string_to_int(undervolt_opt.value());;
  if(!value.has_value())return 0;
  return value.value();
}

// https://www.elinux.org/RPI_vcgencmd_usage
//
// most vcgen commands return "blablabla=wanted"
// where wanted is what we are acutally after
static std::string everything_after_equal(const std::string &unparsed) {
  //x.substr(x.find(":") + 1);
  const auto npos = unparsed.find("=");
  if (npos != std::string::npos) {
	return unparsed.substr(npos + 1);
  }
  openhd::log::get_default()->warn("everything_after_equal - no equal sign found");
  return unparsed;
}

static float vcgencmd_result_parse_float(const std::string& result){
  const auto tmp = rpi::everything_after_equal(result);
  return OHDUtil::string_to_float(tmp).value_or(0);
}
static long vcgencmd_result_parse_long(const std::string& result){
  const auto tmp = rpi::everything_after_equal(result);
  return OHDUtil::string_to_long(tmp).value_or(0);
}

static int8_t read_temperature_soc_degree() {
  int8_t ret = -1;
  const auto vcgencmd_measure_temp_opt = OHDUtil::run_command_out("vcgencmd measure_temp");
  //const auto vcgencmd_measure_temp_opt=std::optional<std::string>("temp=47.2'C");
  if (!vcgencmd_measure_temp_opt.has_value()) {
	return ret;
  }
  const auto tmp_float = vcgencmd_result_parse_float(vcgencmd_measure_temp_opt.value());
  return static_cast<int8_t>(lround(tmp_float));
}
static constexpr auto VCGENCMD_CLOCK_CPU="arm";
static constexpr auto VCGENCMD_CLOCK_ISP="isp";
static constexpr auto VCGENCMD_CLOCK_H264="h264";
static constexpr auto VCGENCMD_CLOCK_CORE="core";
static constexpr auto VCGENCMD_CLOCK_V3D="v3d";
// See https://elinux.org/RPI_vcgencmd_usage
// Shows clock frequency, clock can be one of arm, core, h264, isp, v3d, uart, pwm, emmc, pixel, vec, hdmi, dpi.
// NOTE: vcgencmd returns values in hertz, use the "mhz" util for more easy to read values.
static int vcgencmd_measure_clock(const std::string& which){
  int ret = -1;
  const auto vcgencmd_result = OHDUtil::run_command_out(fmt::format("vcgencmd measure_clock {}",which));
  if (!vcgencmd_result.has_value()) {
	return ret;
  }
  const auto tmp2 = vcgencmd_result_parse_long(vcgencmd_result.value());
  return static_cast<int>(tmp2);
}
static int read_curr_frequency_mhz(const std::string& which){
  return static_cast<uint16_t>(vcgencmd_measure_clock(which)/1000/1000);
}

}

// really stupid, but for now the best solution I came up with
// loosely based on https://stackoverflow.com/questions/9229333/how-to-get-overall-cpu-usage-e-g-57-on-linux
// NOTE: top -v returns procps-ng on both pi4 and my ubuntu laptop
// Also note, we want the CPU usage from all processes - not only -p 1
// 28.July 2022: This seems to work on both rpi4 and my ubuntu pc.
// Also, I am pretty sure we can use -bn1 - top should report from "the last refresh."
static std::optional<int> read_cpuload_once_blocking(){
  auto res_opt=OHDUtil::run_command_out(R"lit(top -bn1 | grep "Cpu(s)")lit");
  // The result from that should look like this: %Cpu(s): 31,0 us,  2,0 sy,  0,0 ni, 67,0 id,  0,0 wa,  0,0 hi,  0,0 si,  0,0 st
  // Where "67.0 id" is what we are after - "time spent in the kernel idle handler"
  // from that, we can deduce the usage
  if(!res_opt.has_value()){
    return std::nullopt;
  }
  const std::string res=res_opt.value();
  //std::cout<<"read_cpuload_once_blocking res:{"<<res<<"}\n";
  std::smatch result;
  const std::regex r1{"ni,(.*) id"};
  auto res1 = std::regex_search(res, result, r1);
  if(!res1 || result.size()<1){
    return std::nullopt;
  }
  const std::string intermediate1=result[0];
  //std::cout<<"Intermediate:{"<<intermediate1<<"}\n";
  if(intermediate1.length()<3){
    return std::nullopt;
  }
  std::regex begin("ni,");
  const auto intermediate2=std::regex_replace(intermediate1, begin, "");
  //std::cout<<"Intermediate2:{"<<intermediate2<<"}\n";
  const auto cpu_idle_perc=std::atof(intermediate2.c_str());
  //std::cout<<"cpu_idle_perc:{"<<cpu_idle_perc<<"}\n";
  const auto cpu_idle_perc_int=static_cast<int>(lround(cpu_idle_perc));
  return 100-cpu_idle_perc_int;
}


}
#endif //XMAVLINKSERVICE_SYSTEMREADUTIL_H
