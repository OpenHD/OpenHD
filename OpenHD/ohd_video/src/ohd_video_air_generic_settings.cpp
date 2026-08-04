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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    AirCameraGenericSettings, switch_primary_and_secondary,
    dualcam_primary_video_allocated_bandwidth_perc, primary_camera_type,
    secondary_camera_type, enable_audio, ip_camera_bitrate_mbits);

std::optional<AirCameraGenericSettings>
AirCameraGenericSettingsHolder::impl_deserialize(
    const std::string &file_as_string) const {
  auto parsed = openhd_json_parse<AirCameraGenericSettings>(file_as_string);
  if (parsed.has_value() && OHDPlatform::instance().is_rpi5() &&
      parsed->primary_camera_type == X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI) {
    openhd::log::get_default()->warn(
        "Migrating legacy MMAL camera setting to Pi 5 libcamera IMX708");
    parsed->primary_camera_type = X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V3_IMX708;
  }
  return parsed;
}

std::string AirCameraGenericSettingsHolder::imp_serialize(
    const AirCameraGenericSettings &data) const {
  const nlohmann::json tmp = data;
  return tmp.dump(4);
}

struct SysutilCameraOverrides {
  std::optional<int> primary;
  std::optional<int> secondary;
  std::optional<int> ip_camera_bitrate_mbits;
};

static SysutilCameraOverrides get_sysutil_camera_overrides() {
  SysutilCameraOverrides overrides{};
  const auto settings_opt = openhd::request_sysutil_settings();
  if (!settings_opt.has_value()) {
    return overrides;
  }

  if (settings_opt->has_camera_type &&
      is_valid_primary_cam_type(settings_opt->camera_type)) {
    overrides.primary = settings_opt->camera_type;
  }
  if (settings_opt->has_camera2_type &&
      is_valid_secondary_cam_type(settings_opt->camera2_type)) {
    overrides.secondary = settings_opt->camera2_type;
  }
  if (settings_opt->has_ip_camera_bitrate_mbits &&
      is_valid_ip_camera_bitrate_mbits(settings_opt->ip_camera_bitrate_mbits)) {
    overrides.ip_camera_bitrate_mbits = settings_opt->ip_camera_bitrate_mbits;
  }
  return overrides;
}

static int rpi_get_default_primary_cam_type(const OHDPlatform &platform) {
  if (platform.is_rpi5()) {
    openhd::log::get_default()->debug(
        "No sysutils primary camera override on Pi 5, using libcamera IMX708");
    return X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V3_IMX708;
  }
  openhd::log::get_default()->debug(
      "No sysutils primary camera override, using MMAL");
  return X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI;
}

AirCameraGenericSettings AirCameraGenericSettingsHolder::create_default()
    const {
  AirCameraGenericSettings ret{};
  ret.primary_camera_type = X_CAM_TYPE_DUMMY_SW;
  ret.secondary_camera_type = X_CAM_TYPE_DISABLED;

  const auto sysutil_overrides = get_sysutil_camera_overrides();
  if (sysutil_overrides.ip_camera_bitrate_mbits.has_value()) {
    ret.ip_camera_bitrate_mbits =
        sysutil_overrides.ip_camera_bitrate_mbits.value();
  }
  if (sysutil_overrides.primary.has_value()) {
    ret.primary_camera_type = sysutil_overrides.primary.value();
    if (OHDPlatform::instance().is_rpi5() &&
        ret.primary_camera_type == X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI) {
      openhd::log::get_default()->warn(
          "Pi 5 does not support the legacy MMAL camera pipeline; using "
          "libcamera IMX708 instead");
      ret.primary_camera_type = X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V3_IMX708;
    }
    openhd::log::get_default()->debug(
        "Using sysutils primary camera type: {}",
        x_cam_type_to_string(ret.primary_camera_type));
  }
  if (sysutil_overrides.secondary.has_value()) {
    ret.secondary_camera_type = sysutil_overrides.secondary.value();
    openhd::log::get_default()->debug(
        "Using sysutils secondary camera type: {}",
        x_cam_type_to_string(ret.secondary_camera_type));
  }

  if (sysutil_overrides.primary.has_value()) {
    return ret;
  }

  if (OHDPlatform::instance().is_rpi()) {
    ret.primary_camera_type =
        rpi_get_default_primary_cam_type(OHDPlatform::instance());
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
    ret.primary_camera_type = X_CAM_TYPE_ORQA_ORCA_DIGITAL_V2;
  } else if (OHDPlatform::instance().platform_type ==
             X_PLATFORM_TYPE_NXP_IMX8) {
    ret.primary_camera_type = X_CAM_TYPE_NXP_IMX8_OS08A20;
  } else if (OHDPlatform::instance().platform_type ==
             X_PLATFORM_TYPE_LUCKFOX_RV110X) {
    ret.primary_camera_type = X_CAM_TYPE_ROCKCHIP_RV110X;
  } else if (OHDPlatform::instance().platform_type ==
             X_PLATFORM_TYPE_OPENHD_X21) {
    ret.primary_camera_type = X_CAM_TYPE_ROCKCHIP_RV1126_CSI;
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
