#pragma once
#include <fstream>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string>
#include <vector>

namespace rk_camera_modes {
struct Format { int width, height, fps; };
inline bool sensor_present() {
  for (int i = 0; i < 32; ++i) {
    std::ifstream f("/sys/class/video4linux/v4l-subdev" + std::to_string(i) + "/name");
    std::string name; std::getline(f, name);
    if (name.find("imx662") != std::string::npos) return true;
  }
  return false;
}
inline bool register_driver_present() {
  for (int i = 0; i < 32; ++i) {
    std::ifstream nameFile("/sys/class/video4linux/v4l-subdev"+std::to_string(i)+"/name");
    std::string name; std::getline(nameFile,name);
    if (name.find("imx662") == std::string::npos) continue;
    const auto node = "/dev/v4l-subdev"+std::to_string(i);
    const int fd = open(node.c_str(),O_RDWR | O_CLOEXEC);
    if (fd < 0) return false;
    v4l2_queryctrl ctrl{}; ctrl.id = V4L2_CID_USER_BASE + 0x10f2;
    const bool found = ioctl(fd,VIDIOC_QUERYCTRL,&ctrl) == 0;
    close(fd); return found;
  }
  return false;
}
// Advertise processed-video choices, not unmerged RAW sensor modes.
inline std::vector<Format> formats(bool aura, int mode) {
  std::vector<Format> result;
  if (mode == 0) {
    for (int fps : {15, 24, 25, 30, 45, 50, 60}) {
      if (!aura && fps == 24) continue;
      result.push_back({1920,1080,fps});
    }
    if (!aura) result.push_back({1920,1080,90});
  } else if (mode == 1) {
    for (int fps : {30,60,90,120,137}) result.push_back({1280,720,fps});
  } else if (mode == 2 && aura) {
    for (int fps : {25,30,45}) result.push_back({1920,1080,fps});
    for (int fps : {60,68}) result.push_back({1280,720,fps});
  }
  return result;
}
inline bool contains(bool aura, int mode, int width, int height, int fps) {
  for (const auto& f : formats(aura, mode))
    if (f.width == width && f.height == height && f.fps == fps) return true;
  return false;
}
inline std::string format_list(bool aura, int mode) {
  std::string result;
  for (const auto& f : formats(aura,mode)) {
    if (!result.empty()) result += ";";
    result += std::to_string(f.width)+"x"+std::to_string(f.height)+"@"+std::to_string(f.fps);
  }
  return result;
}
} // namespace rk_camera_modes
