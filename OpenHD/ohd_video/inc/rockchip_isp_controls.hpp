#pragma once

#include <algorithm>
#include <fcntl.h>
#include <fstream>
#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>
#include <optional>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

namespace rockchip_isp {
struct Control {
  const char* param;
  unsigned int id;
};

inline constexpr Control controls[] = {
    {"RK_EXP_LINES", V4L2_CID_EXPOSURE},
    {"RK_ANALOG_GAIN", V4L2_CID_ANALOGUE_GAIN},
    {"RK_AE_BIAS", V4L2_CID_AUTO_EXPOSURE_BIAS},
    {"RK_BRIGHTNESS", V4L2_CID_BRIGHTNESS},
    {"RK_CONTRAST", V4L2_CID_CONTRAST},
    {"RK_SATURATION", V4L2_CID_SATURATION},
    {"RK_SHARPNESS", V4L2_CID_SHARPNESS},
};

inline std::string sensor_node() {
  std::string node;
  for (int i = 0; i < 64; ++i) {
    const auto name = "v4l-subdev" + std::to_string(i);
    std::ifstream file("/sys/class/video4linux/" + name + "/name");
    std::string entity;
    std::getline(file, entity);
    if (entity.size() < 7 || entity[0] != 'm' || entity[3] != '_' ||
        entity[5] != '_') continue;
    if (!node.empty()) return {};
    node = "/dev/" + name;
  }
  return node;
}

inline std::optional<int> access_control(const Control& control, int frame_height,
                                         std::optional<int> requested = {}) {
  const auto node = sensor_node();
  if (node.empty()) return {};
  const int fd = open(node.c_str(), O_RDWR | O_CLOEXEC);
  if (fd < 0) return {};
  v4l2_queryctrl query{};
  query.id = control.id;
  v4l2_control value{};
  value.id = control.id;
  bool ok = ioctl(fd, VIDIOC_QUERYCTRL, &query) == 0 &&
      !(query.flags & (V4L2_CTRL_FLAG_DISABLED | V4L2_CTRL_FLAG_READ_ONLY));
  if (ok && requested) {
    if (control.id == V4L2_CID_EXPOSURE) {
      v4l2_subdev_format format{};
      format.which = V4L2_SUBDEV_FORMAT_ACTIVE;
      if (ioctl(fd, VIDIOC_SUBDEV_G_FMT, &format) == 0 && format.format.height)
        frame_height = static_cast<int>(format.format.height);
    }
    const int maximum = control.id == V4L2_CID_EXPOSURE
        ? std::min(query.maximum, frame_height - 4) : query.maximum;
    value.value = *requested;
    ok = value.value >= query.minimum && value.value <= maximum;
    if (ok && query.type == V4L2_CTRL_TYPE_INTEGER_MENU) {
      v4l2_querymenu menu{};
      menu.id = control.id;
      menu.index = value.value;
      ok = ioctl(fd, VIDIOC_QUERYMENU, &menu) == 0;
    }
    if (ok) ok = ioctl(fd, VIDIOC_S_CTRL, &value) == 0;
  } else if (ok) {
    ok = ioctl(fd, VIDIOC_G_CTRL, &value) == 0;
  }
  close(fd);
  if (!ok) return {};
  return value.value;
}
}
