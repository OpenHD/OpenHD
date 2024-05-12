//
// Created by consti10 on 19.09.23.
//
#include "ohd_video_air_generic_settings.h"

#include "camera.hpp"
#include "include_json.hpp"
#include "openhd_platform.h"
#include "openhd_spdlog_include.h"
#include "openhd_util.h"
#include "x20_cam_helper.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    AirCameraGenericSettings, switch_primary_and_secondary,
    dualcam_primary_video_allocated_bandwidth_perc, primary_camera_type,
    secondary_camera_type, enable_audio);

std::optional<AirCameraGenericSettings>
AirCameraGenericSettingsHolder::impl_deserialize(
    const std::string &file_as_string) const {
  return openhd_json_parse<AirCameraGenericSettings>(file_as_string);
}

std::string AirCameraGenericSettingsHolder::imp_serialize(
    const AirCameraGenericSettings &data) const {
  const nlohmann::json tmp = data;
  return tmp.dump(4);
}

// The image writer writes the cam type to here
static constexpr auto IMAGE_WRITER_CAM_FILENAME = "/boot/openhd/camera1.txt";

static int rpi_get_default_primary_cam_type() {
  // The image writer writes the cam type to
  const auto opt_content =
      OHDFilesystemUtil::opt_read_file(IMAGE_WRITER_CAM_FILENAME);
  if (opt_content.has_value()) {
    openhd::log::get_default()->debug("Using[{}] from image writer",
                                      opt_content.value());
    const auto opt_value_as_int = OHDUtil::string_to_int(opt_content.value());
    if (opt_value_as_int.has_value()) {
      const int primary_cam_type = opt_value_as_int.value();
      openhd::log::get_default()->debug("Got from image writer: {}",
                                        x_cam_type_to_string(primary_cam_type));
      return primary_cam_type;
    }
  }
  openhd::log::get_default()->debug("No image writer default, using MMAL");
  return X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI;
}

AirCameraGenericSettings AirCameraGenericSettingsHolder::create_default()
    const {
  AirCameraGenericSettings ret{};
  ret.primary_camera_type = X_CAM_TYPE_DUMMY_SW;
  ret.secondary_camera_type = X_CAM_TYPE_DISABLED;
  if (OHDPlatform::instance().is_rpi()) {
    ret.primary_camera_type = rpi_get_default_primary_cam_type();
  } else if (OHDPlatform::instance().is_x20()) {
    ret.primary_camera_type = openhd::x20::detect_camera_type();
  } else if (OHDPlatform::instance().platform_type ==
             X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W) {
    ret.primary_camera_type = X_CAM_TYPE_ROCK_RK3566_IMX219;
  } else if (OHDPlatform::instance().is_rock()) {
    ret.primary_camera_type = X_CAM_TYPE_ROCK_HDMI_IN;
  } else if (OHDPlatform::instance().platform_type ==
             X_PLATFORM_TYPE_OPENIPC_SIGMASTAR_UNDEFINED) {
    ret.primary_camera_type = X_CAM_TYPE_EXTERNAL;
  } else if (OHDPlatform::instance().platform_type ==
             X_PLATFORM_TYPE_NVIDIA_XAVIER) {
    ret.primary_camera_type = X_CAM_TYPE_NVIDIA_XAVIER_IMX577;
  }
  return ret;
}

void AirCameraGenericSettingsHolder::x20_only_discover_and_save_camera_type() {
  // On the X20, every time openhd is started, we (newly) detect the camera
  // type. This is in contrast to pretty much any other platform (where we do
  // not have camera auto detection and therefore rely on the user setting the
  // camera)
  unsafe_get_settings().primary_camera_type = openhd::x20::detect_camera_type();
  unsafe_get_settings().secondary_camera_type = X_CAM_TYPE_DISABLED;
  persist(false);
}
