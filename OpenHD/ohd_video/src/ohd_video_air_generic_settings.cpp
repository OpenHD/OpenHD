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

#include "ohd_video_air_generic_settings.h"

#include "camera.hpp"
#include "include_json.hpp"
#include "openhd_platform.h"
#include "openhd_sock.h"
#include "openhd_spdlog_include.h"
#include "openhd_util.h"
#include "x20_cam_helper.h"

extern AirCameraGenericSettings g_airCameraGenericSettings;

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

static std::optional<int> get_sysutil_camera_type() {
  const auto settings_opt = openhd::request_sysutil_settings();
  if (!settings_opt.has_value() || !settings_opt->has_camera_type) {
    return std::nullopt;
  }
  return settings_opt->camera_type;
}

static int rpi_get_default_primary_cam_type() {
  const auto sysutil_cam = get_sysutil_camera_type();
  if (sysutil_cam.has_value()) {
    openhd::log::get_default()->debug("Using sysutils camera type: {}",
                                      x_cam_type_to_string(sysutil_cam.value()));
    return sysutil_cam.value();
  }
  openhd::log::get_default()->debug("No sysutils camera override, using MMAL");
  return X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI;
}

AirCameraGenericSettings AirCameraGenericSettingsHolder::create_default()
    const {
  AirCameraGenericSettings ret{};
  ret.primary_camera_type = X_CAM_TYPE_DUMMY_SW;
  ret.secondary_camera_type = X_CAM_TYPE_DISABLED;

  const auto sysutil_cam = get_sysutil_camera_type();
  if (sysutil_cam.has_value()) {
    ret.primary_camera_type = sysutil_cam.value();
    return ret;
  }

  if (OHDPlatform::instance().is_rpi()) {
    ret.primary_camera_type = rpi_get_default_primary_cam_type();
  } else if (OHDPlatform::instance().is_x20()) {
    ret.primary_camera_type = openhd::x20::detect_camera_type();
  } else if (OHDPlatform::instance().is_a733()) {
    ret.primary_camera_type = X_CAM_TYPE_A733_IMX415;
  } else if ((OHDPlatform::instance().platform_type ==
              X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W) ||
             (OHDPlatform::instance().platform_type ==
              X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_CM3)) {
    ret.primary_camera_type = X_CAM_TYPE_ROCK_3_IMX462;
  } else if (OHDPlatform::instance().platform_type ==
             X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_A) {
    ret.primary_camera_type = X_CAM_TYPE_ROCK_5_IMX462;
  } else if (OHDPlatform::instance().platform_type ==
             X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_B) {
    ret.primary_camera_type = X_CAM_TYPE_ROCK_5_HDMI_IN;
  } else if (OHDPlatform::instance().platform_type ==
             X_PLATFORM_TYPE_OPENIPC_SIGMASTAR_UNDEFINED) {
    ret.primary_camera_type = X_CAM_TYPE_EXTERNAL;
  } else if (OHDPlatform::instance().platform_type ==
             X_PLATFORM_TYPE_NVIDIA_XAVIER) {
    ret.primary_camera_type = X_CAM_TYPE_NVIDIA_XAVIER_IMX577;
  } else if (OHDPlatform::instance().platform_type ==
             X_PLATFORM_TYPE_QUALCOMM_QRB5165) {
    ret.primary_camera_type = X_CAM_TYPE_QC_IMX577;
  } else if (OHDPlatform::instance().platform_type == X_PLATFORM_TYPE_ORQA) {
    ret.primary_camera_type = X_CAM_TYPE_ORQA_HORNET;
  } else if (OHDPlatform::instance().platform_type == X_PLATFORM_TYPE_NXP_IMX8) {
    ret.primary_camera_type = X_CAM_TYPE_NXP_IMX8_V4L2;
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
