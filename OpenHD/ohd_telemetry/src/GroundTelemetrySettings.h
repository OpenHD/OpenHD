//
// Created by consti10 on 07.11.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_GROUNDTELEMETRYSETTINGS_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_GROUNDTELEMETRYSETTINGS_H_

#include <map>

#include "openhd_settings_directories.hpp"
#include "openhd_settings_persistent.h"

namespace openhd::telemetry::ground{

struct Settings{
  bool enable_rc_over_joystick=false;
  int rc_over_joystick_update_rate_hz=30;
  std::string rc_channel_mapping="0,1,2,3,4,5,6,7";
  // This is for outputting FC mavlink data via serial on the ground station
  int gnd_uart_connection_type=0;
  int gnd_uart_baudrate=115200;
};

static bool valid_joystick_update_rate(int value){
    return value>=1 && value<=150;
}

static bool validate_uart_connection_type(int type){
  return type >=0 && type <=6;
}

// If disabled, return nullopt
// If enabled, return the linux fd string for this (UART) connection type
static std::string uart_fd_from_connection_type(int connection_type){
  switch (connection_type) {
    case 1:return "/dev/serial0";
    case 2:return "/dev/serial1";
    case 3:return "/dev/ttyUSB0";
    case 4:return "/dev/ttyACM0";
    case 5:return "/dev/ttyACM1";
    case 6:return "/dev/ttyS7";
    default:
      return "/dev/serial0";
  }
  assert(false);
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings,enable_rc_over_joystick,rc_over_joystick_update_rate_hz,rc_channel_mapping,
                                   gnd_uart_connection_type,gnd_uart_baudrate);

class SettingsHolder:public openhd::settings::PersistentSettings<Settings>{
 public:
  SettingsHolder():
                     openhd::settings::PersistentSettings<Settings>(
                         openhd::get_telemetry_settings_directory()){
    init();
  }
 private:
  [[nodiscard]] std::string get_unique_filename()const override{
    std::stringstream ss;
    ss<<"ground_settings.json";
    return ss.str();
  }
  [[nodiscard]] Settings create_default()const override{
    return Settings{};
  }
};

}

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_GROUNDTELEMETRYSETTINGS_H_
