//
// Created by consti10 on 07.11.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_GROUNDTELEMETRYSETTINGS_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_GROUNDTELEMETRYSETTINGS_H_

#include "openhd-settings.hpp"
#include "openhd-settings2.hpp"
#include <map>

namespace openhd::telemetry::ground{

static const std::string SETTINGS_DIRECTORY =std::string(BASE_PATH)+std::string("telemetry/");

struct Settings{
  bool enable_rc_over_joystick=false;
  int dummy=0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings,enable_rc_over_joystick,dummy);

class SettingsHolder:public openhd::settings::PersistentSettings<Settings>{
 public:
  SettingsHolder():
                     openhd::settings::PersistentSettings<Settings>(
                         SETTINGS_DIRECTORY){
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
