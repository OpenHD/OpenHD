//
// Created by consti10 on 07.11.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_GROUNDTELEMETRYSETTINGS_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_GROUNDTELEMETRYSETTINGS_H_

#include <map>

#include "openhd_settings_directories.hpp"
#include "openhd_settings_persistent.h"

namespace openhd::telemetry::ground{

// We use an empty string for "serial disabled"
static constexpr auto UART_CONNECTION_TYPE_DISABLE="";

struct Settings{
  bool enable_rc_over_joystick=false;
  int rc_over_joystick_update_rate_hz=30;
  std::string rc_channel_mapping="0,1,2,3,4,5,6,7";
  // This is for outputting FC mavlink data via serial on the ground station
  std::string gnd_uart_connection_type=UART_CONNECTION_TYPE_DISABLE;
  int gnd_uart_baudrate=115200;
};

static bool valid_joystick_update_rate(int value){
    return value>=1 && value<=150;
}


NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings,enable_rc_over_joystick,rc_over_joystick_update_rate_hz,rc_channel_mapping,
                                   gnd_uart_connection_type,gnd_uart_baudrate);

class SettingsHolder:public openhd::PersistentJsonSettings<Settings>{
 public:
  SettingsHolder():
                     openhd::PersistentJsonSettings<Settings>(
                         openhd::get_telemetry_settings_directory()){
    init();
  }
  bool is_serial_enabled(){
    return !get_settings().gnd_uart_connection_type.empty();
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
