//
// Created by consti10 on 14.02.24.
//

#ifndef OPENHD_X20_CAM_HELPER_H
#define OPENHD_X20_CAM_HELPER_H

#include "camera_settings.hpp"
#include "openhd_spdlog.h"
#include "openhd_util.h"

namespace openhd::x20 {
// TODO: Right now we do not have a proper mapping between ranges, only none /
// some
static std::optional<int> get_x20_contrast(const CameraSettings& settings) {
  if (settings.openhd_contrast == OPENHD_CONTRAST_DEFAULT) return std::nullopt;
  if (settings.openhd_contrast > OPENHD_CONTRAST_DEFAULT) return 3;
  return std::nullopt;
}
static std::optional<int> get_x20_saturation(const CameraSettings& settings) {
  if (settings.openhd_saturation == OPENHD_SATURATION_DEFAULT)
    return std::nullopt;
  if (settings.openhd_contrast > OPENHD_CONTRAST_DEFAULT) return 5;
  return std::nullopt;
}
static std::optional<int> get_x20_flip(const CameraSettings& settings) {
  if (settings.openhd_flip == OPENHD_FLIP_NONE) return std::nullopt;
  return settings.openhd_flip;
}

// On allwinner / X20 we set IQ params with scripts
static void apply_x20_runcam_iq_settings(const CameraSettings& settings) {
  openhd::log::get_default()->debug("apply_x20_runcam_iq_settings begin");
  const auto flip = get_x20_flip(settings);
  if (flip.has_value()) {
    std::stringstream ss;
    ss << "bash /usr/local/bin/x20/runcam_v2/runcam_flip.sh " << flip.value();
    OHDUtil::run_command(ss.str(), {});
  }
  const auto contrast = get_x20_contrast(settings);
  if (contrast.has_value()) {
    std::stringstream ss;
    ss << "bash /usr/local/bin/x20/runcam_v2/runcam_contrast.sh "
       << contrast.value();
    OHDUtil::run_command(ss.str(), {});
  }
  const auto saturation = get_x20_saturation(settings);
  if (saturation.has_value()) {
    std::stringstream ss;
    ss << "bash /usr/local/bin/x20/runcam_v2/runcam_saturation.sh "
       << saturation.value();
    OHDUtil::run_command(ss.str(), {});
  }
  openhd::log::get_default()->debug("apply_x20_runcam_iq_settings end");
}

// On the X20, we can detect the camera type, as it should be to create
// a nice experience for the user.
// In case anything fails, we assume X_CAM_TYPE_X20_HDZERO_GENERIC
// which means we don't expose any controls.
static int detect_camera_type() {
  const auto result_opt = OHDUtil::run_command_out(
      "bash /usr/local/bin/x20/runcam_v2/detect_runcam.sh ");
  if (!result_opt.has_value()) {
    return X_CAM_TYPE_X20_HDZERO_GENERIC;
  }
  const auto result_int_opt = OHDUtil::string_to_int(result_opt.value());
  if (!result_int_opt.has_value()) {
    return X_CAM_TYPE_X20_HDZERO_GENERIC;
  }
  const int result_int = result_int_opt.value();
  if (result_int == 3) {
    return X_CAM_TYPE_X20_HDZERO_RUNCAM_V3;
  }
  if (result_int == 2) {
    return X_CAM_TYPE_X20_HDZERO_RUNCAM_V2;
  }
  if (result_int == 1) {
    return X_CAM_TYPE_X20_HDZERO_RUNCAM_V1;
  }
  if (result_int == 90) {
    return X_CAM_TYPE_X20_HDZERO_RUNCAM_NANO_90;
  }
  return X_CAM_TYPE_X20_HDZERO_GENERIC;
}

}  // namespace openhd::x20

#endif  // OPENHD_X20_CAM_HELPER_H
