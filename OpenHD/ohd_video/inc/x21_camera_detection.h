#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

#include "camera_registry_generated.h"

namespace openhd::x21 {
// Pivariety identifies the camera family, not an IMX model. Do not infer a
// sensor model from the available resolutions or an undocumented firmware ID.
inline std::optional<int> detect_camera_type(
    const std::filesystem::path& video_class = "/sys/class/video4linux") {
  std::error_code error;
  std::filesystem::directory_iterator entries(video_class, error);
  if (error) return std::nullopt;
  for (const auto& entry : entries) {
    if (entry.path().filename().string().rfind("v4l-subdev", 0) != 0) continue;
    std::ifstream input(entry.path() / "name");
    std::string name;
    std::getline(input, name);
    if (name.rfind("arducam-pivariety", 0) == 0)
      return X_CAM_TYPE_X21_ARDUCAM_PIVARIETY;
  }
  return std::nullopt;
}

inline bool is_native_csi_type(int type) {
  return type == X_CAM_TYPE_ROCKCHIP_RV1126_CSI ||
         type == X_CAM_TYPE_X21_ARDUCAM_PIVARIETY ||
         type == X_CAM_TYPE_X21_OHD_Jaguar || (type >= 143 && type <= 150);
}
}  // namespace openhd::x21
