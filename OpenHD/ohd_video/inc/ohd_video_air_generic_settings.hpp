//
// Created by consti10 on 11.01.23.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_AIR_GENERIC_SETTINGS_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_AIR_GENERIC_SETTINGS_H_

// NOTE: These are not the camera-specific settings, but rather settings regarding the management
// of how those camera(s) should be used

#include "json.hpp"
#include "openhd-settings-directories.hpp"
#include "openhd-settings-persistent.hpp"

struct AirCameraGenericSettings {
  // Make primary camera secondary camera and other way around
  bool switch_primary_and_secondary=false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AirCameraGenericSettings,switch_primary_and_secondary);

class AirCameraGenericSettingsHolder: public openhd::settings::PersistentSettings<AirCameraGenericSettings>{
 public:
  AirCameraGenericSettingsHolder()
      :openhd::settings::PersistentSettings<AirCameraGenericSettings>(std::string(openhd::BASE_PATH)+std::string("video/")){
    init();
  }
 private:
  [[nodiscard]] std::string get_unique_filename()const override{
    return "air_camera_generic.json";
  }
  [[nodiscard]] AirCameraGenericSettings create_default()const override{
    return AirCameraGenericSettings{};
  }
};

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_AIR_GENERIC_SETTINGS_H_
