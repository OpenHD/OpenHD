//
// Created by consti10 on 26.04.22.
//

#ifndef XMAVLINKSERVICE_SYSTEMREADUTIL_H
#define XMAVLINKSERVICE_SYSTEMREADUTIL_H

#include "OnboardComputerStatusHelper.h"
#include "mav_include.h"
#include "openhd-util.hpp"
#include <sstream>
#include <chrono>

// https://mavlink.io/en/messages/common.html#ONBOARD_COMPUTER_STATUS
// used to be a custom message for a short amount of time.
namespace OnboardComputerStatus {

// from https://github.com/OpenHD/Open.HD/blob/35b6b10fbeda43cd06bbfbd90e2daf29629c2f8a/openhd-status/src/statusmicroservice.cpp#L165
// Return the CPU/SOC temperature of the system the generator is running on
// Unit: Degree ?
static int readTemperature() {
  int cpu_temperature = 0;
  FILE *fp;
  try {
	fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
	fscanf(fp, "%d", &cpu_temperature);
  } catch (...) {
	std::cerr << "ERROR: thermal reading" << std::endl;
	return -1;
  }
  fclose(fp);
  cpu_temperature = cpu_temperature / 1000;
  return cpu_temperature;
}

// copy and paste from QOpenHD, I think we can get the under-voltage warning on rpi this way.
static int readRpiUnderVoltError() {
  auto fp3 = fopen("/tmp/undervolt", "r");
  if (fp3 == nullptr) {
	return 0;
  }
  int undervolt_gnd = 0;
  fscanf(fp3, "%d", &undervolt_gnd);
  fclose(fp3);
  return undervolt_gnd;
}

// Stuff that works only on rpi
namespace rpi {

// https://www.elinux.org/RPI_vcgencmd_usage

// most vcgen commands return "blablabla=wanted"
// where wanted is what we are acutally after
static std::string everything_after_equal(const std::string &unparsed) {
  //x.substr(x.find(":") + 1);
  const auto npos = unparsed.find("=");
  if (npos != std::string::npos) {
	return unparsed.substr(npos + 1);
  }
  std::cout << "everything_after_equal - no equal sign found\n";
  return unparsed;
}

static float vcgencmd_result_parse_float(const std::string& result){
  const auto tmp = rpi::everything_after_equal(result);
  // atof cuts away the part after the number for us (everything not a number)
  return std::atof(tmp.c_str());
}
static long vcgencmd_result_parse_long(const std::string& result){
  const auto tmp = rpi::everything_after_equal(result);
  // atol cuts away the part after rhe number for us (everything not a number)
  return std::atol(tmp.c_str());
}

static int8_t read_temperature_soc_degree() {
  int8_t ret = -1;
  const auto vcgencmd_measure_temp_opt = OHDUtil::run_command_out("vcgencmd measure_temp");
  //const auto vcgencmd_measure_temp_opt=std::optional<std::string>("temp=47.2'C");
  if (!vcgencmd_measure_temp_opt.has_value()) {
	return ret;
  }
  const auto tmp_float = vcgencmd_result_parse_float(vcgencmd_measure_temp_opt.value());
  //std::cout << "soc_degree():{" << tmp_float << "}\n";
  return static_cast<int8_t>(lround(tmp_float));
}
static constexpr auto VCGENCMD_CLOCK_CPU="arm";
static constexpr auto VCGENCMD_CLOCK_ISP="isp";
static constexpr auto VCGENCMD_CLOCK_H264="h264";
static constexpr auto VCGENCMD_CLOCK_CORE="core";
// See https://elinux.org/RPI_vcgencmd_usage
// Shows clock frequency, clock can be one of arm, core, h264, isp, v3d, uart, pwm, emmc, pixel, vec, hdmi, dpi.
// NOTE: vcgencmd returns values in hertz, use the "mhz" util for more easy to read values.
static int vcgencmd_measure_clock(const std::string& which){
  int ret = -1;
  std::stringstream command;
  command<<"vcgencmd measure_clock "<<which;
  const auto vcgencmd_result = OHDUtil::run_command_out(command.str().c_str());
  if (!vcgencmd_result.has_value()) {
	return ret;
  }
  const auto tmp2 = vcgencmd_result_parse_long(vcgencmd_result.value());
  //std::cout << "clock for "<<which<<" :{" << tmp2 << "}\n";
  return static_cast<int>(tmp2);
}
static int read_curr_frequency_mhz(const std::string& which){
  return static_cast<uint16_t>(vcgencmd_measure_clock(which)/1000/1000);
}

}

// For rpi, we have 2 messages - the generic mavlink one (which unfortunately doesn't match the pi well)
// and a custom openhd onboard computer status extension one
static std::vector<MavlinkMessage> createOnboardComputerStatus(const uint8_t sys_id,const uint8_t comp_id,const bool is_platform_rpi,const int cpu_usage){
  MavlinkMessage msg;
  mavlink_onboard_computer_status_t mavlink_onboard_computer_status{};
  mavlink_onboard_computer_status.cpu_cores[0]=cpu_usage;
  const auto before=std::chrono::steady_clock::now();
  if(is_platform_rpi){
    mavlink_onboard_computer_status.temperature_core[0]=rpi::read_temperature_soc_degree();
	// temporary, until we have our own message
	mavlink_onboard_computer_status.storage_type[0]=rpi::read_curr_frequency_mhz(rpi::VCGENCMD_CLOCK_CPU);
	mavlink_onboard_computer_status.storage_type[1]=rpi::read_curr_frequency_mhz(rpi::VCGENCMD_CLOCK_ISP);
	mavlink_onboard_computer_status.storage_type[2]=rpi::read_curr_frequency_mhz(rpi::VCGENCMD_CLOCK_H264);
	mavlink_onboard_computer_status.storage_type[3]=rpi::read_curr_frequency_mhz(rpi::VCGENCMD_CLOCK_CORE);

  }else{
    const auto cpu_temp=(int8_t)OnboardComputerStatus::readTemperature();
    mavlink_onboard_computer_status.temperature_core[0]=cpu_temp;
  }
  const auto processing_time=std::chrono::steady_clock::now()-before;
  if(processing_time>std::chrono::milliseconds(100)){
	std::cout<<"Warning: measuring stats took unexpected long:"<<std::chrono::duration_cast<std::chrono::milliseconds>(processing_time).count()<<" ms\n";
  }
  mavlink_msg_onboard_computer_status_encode(sys_id,comp_id,&msg.m,&mavlink_onboard_computer_status);
  std::vector<MavlinkMessage> ret;
  ret.push_back(msg);
  if(is_platform_rpi){
	//TODO
  }
  return ret;
}

// TODO more telemetry here
/*static MavlinkMessage createOnboardComputerStatusExtension(const bool IS_PLATFORM_RPI,const uint8_t sys_id,const uint8_t comp_id){
  MavlinkMessage msg;
  mavlink_openhd_onboard_computer_status_extension_t values{};
  if(IS_PLATFORM_RPI){
    //
  }
  mavlink_msg_openhd_onboard_computer_status_extension_encode(sys_id,comp_id,&msg.m,&values);
  return msg;
}*/


}
#endif //XMAVLINKSERVICE_SYSTEMREADUTIL_H
