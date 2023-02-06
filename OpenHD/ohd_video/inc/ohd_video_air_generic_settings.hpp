//
// Created by consti10 on 11.01.23.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_AIR_GENERIC_SETTINGS_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_AIR_GENERIC_SETTINGS_H_

// NOTE: These are not the camera-specific settings, but rather settings regarding the management
// of how those camera(s) should be used

#include "include_json.hpp"
#include "openhd-settings-directories.hpp"
#include "openhd-settings-persistent.hpp"

struct AirCameraGenericSettings {
  // Make primary camera secondary camera and other way around (aka if they are detected in the wrong order)
  bool switch_primary_and_secondary=false;
  // On startup, we wait for up to X seconds until this many camera(s) have been discovered
  // and create one or more dummy camera(s) if they are not found
  //int n_cameras_to_wait_for=1;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AirCameraGenericSettings,switch_primary_and_secondary);

class AirCameraGenericSettingsHolder: public openhd::settings::PersistentSettings<AirCameraGenericSettings>{
 public:
  AirCameraGenericSettingsHolder()
      :openhd::settings::PersistentSettings<AirCameraGenericSettings>(openhd::get_video_settings_directory()){
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
