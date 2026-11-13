#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace openhd::v4l2 {
// Raw pixel formats alone do not identify a USB camera: Unicam exposes them too.
// Thermal userspace drivers intentionally export a V4L2 loopback capture node.
inline bool is_usb_or_loopback(const std::string& bus,
                               const std::string& driver) {
  return bus.rfind("usb", 0) == 0 || driver == "v4l2 loopback" ||
         driver == "v4l2loopback";
}

inline std::optional<std::string> find_unique_capture_device(
    const std::filesystem::path& root, std::string_view name,
    std::string_view alternate_name = {}) {
  std::error_code error;
  std::optional<std::string> device;
  for (std::filesystem::directory_iterator it(root, error), end;
       !error && it != end; it.increment(error)) {
    const auto node = it->path().filename().string();
    if (node.rfind("video", 0) != 0 || node.size() == 5 ||
        node.find_first_not_of("0123456789", 5) != std::string::npos) continue;
    std::ifstream input(it->path() / "name");
    std::string node_name;
    if (!std::getline(input, node_name) || node_name.empty()) continue;
    if (node_name != name && node_name != alternate_name) continue;
    if (device) return std::nullopt;
    device = "/dev/" + node;
  }
  return error ? std::nullopt : device;
}

inline std::optional<std::string> find_veye_capture_device(
    const std::filesystem::path& root = "/sys/class/video4linux") {
  return find_unique_capture_device(root, "unicam-image", "unicam");
}

inline std::optional<std::string> find_rockchip_isp_mainpath(
    const std::filesystem::path& root = "/sys/class/video4linux") {
  return find_unique_capture_device(root, "rkisp_mainpath");
}
}  // namespace openhd::v4l2
