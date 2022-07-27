//
// Created by consti10 on 26.04.22.
//

#ifndef XMAVLINKSERVICE_SYSTEMREADUTIL_H
#define XMAVLINKSERVICE_SYSTEMREADUTIL_H

#include "OnboardComputerStatusHelper.h"
#include "mav_include.h"
#include "openhd-util.hpp"

// https://mavlink.io/en/messages/common.html#ONBOARD_COMPUTER_STATUS
// used to be a custom message for a short amount of time.
namespace OnboardComputerStatus {

// from https://github.com/OpenHD/Open.HD/blob/35b6b10fbeda43cd06bbfbd90e2daf29629c2f8a/openhd-status/src/statusmicroservice.cpp#L173
// Return the CPU load of the system the generator is running on
// Unit: Percentage ?
static int readCpuLoad() {
  int cpuload_gnd = 0;
  long double a[4];
  FILE *fp;
  try {
    fp = fopen("/proc/stat", "r");
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
  } catch (...) {
    std::cerr << "ERROR: proc reading1" << std::endl;
    return -1;
  }
  fclose(fp);
  cpuload_gnd = (a[0] + a[1] + a[2]) / (a[0] + a[1] + a[2] + a[3]) * 100;
  return cpuload_gnd;
}

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
static int readRpiUnderVoltError(){
  auto fp3 = fopen("/tmp/undervolt", "r");
  if (fp3 == nullptr) {
    return 0;
  }
  int undervolt_gnd = 0;
  fscanf(fp3,"%d", &undervolt_gnd);
  fclose(fp3);
  return undervolt_gnd;
}

// Stuff that works only on rpi
namespace rpi{

// https://www.elinux.org/RPI_vcgencmd_usage
/*static float read_voltage_core_volts(){
  const auto vcgencmd_measure_volts_opt=OHDUtil::run_command_out("vcgencmd measure_volts core");
  if(!vcgencmd_measure_volts_opt.has_value()){
    return -1;
  }
  const auto vcgencmd_measure_volts=vcgencmd_measure_volts_opt.value();
  const std::regex r{"temp=([\\w]+)"};
  std::smatch result;
  if (!std::regex_search(vcgencmd_measure_volts, result, r)) {
    return -1;
  }
  std::cout<<"!:"<<vcgencmd_measure_volts_opt.value()<<"\n";
  return 0;
}*/
static int8_t read_temperature_soc_degree(){
  int8_t ret=-1;
  const auto vcgencmd_measure_temp_opt=OHDUtil::run_command_out("vcgencmd measure_temp");
  //const auto vcgencmd_measure_temp_opt=std::optional<std::string>("temp=47.2'C");
  if(!vcgencmd_measure_temp_opt.has_value()){
    return ret;
  }
  const auto& vcgencmd_measure_temp=vcgencmd_measure_temp_opt.value();
  const std::regex r{"([[+-]?([0-9]*[.])?[0-9]+)"};
  std::smatch result;
  if (!std::regex_search(vcgencmd_measure_temp, result, r)) {
    return ret;
  }
  if (result.size() >=1) {
    const auto float_as_string=result[0];
    const auto temp_degree=std::stof(float_as_string);
    //std::cout<<"Result:"<<temp_degree<<"\n";
    return static_cast<int8_t>(temp_degree);
  }
  return ret;
}
}

// For rpi, we have 2 messages - the generic mavlink one (which unfortunately doesn't match the pi well)
// and a custom openhd onboard computer status extension one
static std::vector<MavlinkMessage> createOnboardComputerStatus(const uint8_t sys_id,const uint8_t comp_id,const bool is_platform_rpi,const int cpu_usage){
  MavlinkMessage msg;
  mavlink_onboard_computer_status_t mavlink_onboard_computer_status;
  mavlink_onboard_computer_status.cpu_cores[0]=cpu_usage;
  if(is_platform_rpi){
    mavlink_onboard_computer_status.temperature_core[0]=rpi::read_temperature_soc_degree();
  }else{
    const auto cpu_temp=(int8_t)OnboardComputerStatus::readTemperature();
    mavlink_onboard_computer_status.temperature_core[0]=cpu_temp;
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
