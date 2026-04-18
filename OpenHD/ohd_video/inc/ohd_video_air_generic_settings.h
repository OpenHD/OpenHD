/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_AIR_GENERIC_SETTINGS_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_AIR_GENERIC_SETTINGS_H_

// NOTE: These are not the camera-specific settings, but rather settings
// regarding the management of how those camera(s) should be used

#include "openhd_settings_directories.h"
#include "openhd_settings_persistent.h"

static int OPENHD_AUDIO_DISABLE = 1;
static int OPENHD_AUDIO_TEST = 100;

struct AirCameraGenericSettings {
  // Make primary camera secondary camera and other way around (aka if they are
  // detected in the wrong order)
  bool switch_primary_and_secondary = false;
  // the link recommends a total video bitrate to us - in case of dual camera,
  // we need to split that up into bitrate for primary and secondary video
  int dualcam_primary_video_allocated_bandwidth_perc =
      60;  // Default X%:Y split
  // Default camera type(s) depend on platform - see below
  int primary_camera_type = 0;
  int secondary_camera_type = 0;
  // Audio can be enabled, in which case gstreamer hopefully picks up the right
  // audio source via autoaudiosrc
  int enable_audio = OPENHD_AUDIO_DISABLE;
};

static bool is_valid_dualcam_primary_video_allocated_bandwidth(
    int dualcam_primary_video_allocated_bandwidth_perc) {
  return dualcam_primary_video_allocated_bandwidth_perc >= 10 &&
         dualcam_primary_video_allocated_bandwidth_perc <= 90;
}

class AirCameraGenericSettingsHolder
    : public openhd::PersistentSettings<AirCameraGenericSettings> {
 public:
  AirCameraGenericSettingsHolder()
      : openhd::PersistentSettings<AirCameraGenericSettings>(
            openhd::get_video_settings_directory()) {
    init();
  }
  void x20_only_discover_and_save_camera_type();

 private:
  [[nodiscard]] std::string get_unique_filename() const override {
    return "air_camera_generic.json";
  }
  [[nodiscard]] AirCameraGenericSettings create_default() const override;
  std::optional<AirCameraGenericSettings> impl_deserialize(
      const std::string& file_as_string) const override;
  std::string imp_serialize(
      const AirCameraGenericSettings& data) const override;
};

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_AIR_GENERIC_SETTINGS_H_
