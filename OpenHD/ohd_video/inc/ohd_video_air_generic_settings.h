//
// Created by consti10 on 11.01.23.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_AIR_GENERIC_SETTINGS_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_AIR_GENERIC_SETTINGS_H_

// NOTE: These are not the camera-specific settings, but rather settings regarding the management
// of how those camera(s) should be used

#include "openhd_settings_directories.hpp"
#include "openhd_settings_persistent.h"

struct AirCameraGenericSettings {
  // Make primary camera secondary camera and other way around (aka if they are detected in the wrong order)
  bool switch_primary_and_secondary=false;
  // On startup, we wait for up to X seconds until this many camera(s) have been discovered
  // and create one or more dummy camera(s) if they are not found in the given time span.
  // (Only used if camera autodetect is on)
  int n_cameras_to_wait_for=1;
  // the link recommends a total video bitrate to us - in case of dual camera, we need to split that up into
  // bitrate for primary and secondary video
  int dualcam_primary_video_allocated_bandwidth_perc=60; // Default X%:Y split
  int primary_camera_type=0;
  int secondary_camera_type=0;
};

static bool is_valid_dualcam_primary_video_allocated_bandwidth(int dualcam_primary_video_allocated_bandwidth_perc){
  return dualcam_primary_video_allocated_bandwidth_perc>=10 && dualcam_primary_video_allocated_bandwidth_perc<=90;
}

class AirCameraGenericSettingsHolder: public openhd::PersistentSettings<AirCameraGenericSettings>{
 public:
  AirCameraGenericSettingsHolder()
      :openhd::PersistentSettings<AirCameraGenericSettings>(openhd::get_video_settings_directory()){
    init();
  }
 private:
  [[nodiscard]] std::string get_unique_filename()const override{
    return "air_camera_generic.json";
  }
  [[nodiscard]] AirCameraGenericSettings create_default()const override{
    return AirCameraGenericSettings{};
  }
std::optional<AirCameraGenericSettings> impl_deserialize(const std::string& file_as_string)const override;
std::string imp_serialize(const AirCameraGenericSettings& data)const override;
};

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_AIR_GENERIC_SETTINGS_H_
