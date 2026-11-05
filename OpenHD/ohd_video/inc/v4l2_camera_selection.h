#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace openhd::v4l2 {
// Raw pixel formats alone do not identify a USB camera: Unicam exposes them too.
// Thermal userspace drivers intentionally export a V4L2 loopback capture node.
inline bool is_usb_or_loopback(const std::string& bus,
                               const std::string& driver) {
  return bus.rfind("usb", 0) == 0 || driver == "v4l2 loopback" ||
         driver == "v4l2loopback";
}

inline std::optional<std::string> find_veye_capture_device(
    const std::filesystem::path& root = "/sys/class/video4linux") {
  std::error_code error;
  std::vector<std::string> candidates;
  for (std::filesystem::directory_iterator it(root, error), end;
       !error && it != end; it.increment(error)) {
    const auto node = it->path().filename().string();
    if (node.rfind("video", 0) != 0 || node.size() == 5 ||
        node.find_first_not_of("0123456789", 5) != std::string::npos) continue;
    std::ifstream input(it->path() / "name");
    std::string name;
    std::getline(input, name);
    // A VEYE overlay uses the legacy Unicam capture node (not its metadata
    // node). USB UVC devices, ISP outputs and encoders must never be selected.
    if (name == "unicam-image" || name == "unicam")
      candidates.push_back("/dev/" + node);
  }
  // Multiple CSI receivers require explicit port mapping, not an arbitrary pick.
  if (candidates.size() != 1) return std::nullopt;
  return candidates.front();
}
}  // namespace openhd::v4l2
