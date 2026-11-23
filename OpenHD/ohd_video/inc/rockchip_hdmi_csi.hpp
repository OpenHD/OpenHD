#pragma once

#include <fcntl.h>
#include <linux/media-bus-format.h>
#include <linux/v4l2-subdev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

#include "hdmi_edid.hpp"
#include "openhd_spdlog.h"
#include "v4l2_camera_selection.h"

namespace openhd::rockchip_hdmi_csi {

using hdmi_edid::call;

inline std::string find_subdevice(const std::string& name) {
  std::error_code error;
  std::string found;
  for (std::filesystem::directory_iterator it("/sys/class/video4linux", error),
       end; !error && it != end; it.increment(error)) {
    const auto device = it->path().filename().string();
    if (device.rfind("v4l-subdev", 0) != 0) continue;
    std::ifstream file(it->path() / "name");
    std::string value;
    if (!std::getline(file, value) || value.find(name) == std::string::npos)
      continue;
    if (!found.empty()) return {};
    found = "/dev/" + device;
  }
  return error ? std::string{} : found;
}

enum class Bridge { Toshiba2Lane, Toshiba4Lane, RadxaRk628 };

inline std::string setup_capture(int width, int height, int fps, Bridge type) {
  const auto log = openhd::log::get_default();
  const bool rk628 = type == Bridge::RadxaRk628;
  const bool limit_pixel_clock = type == Bridge::Toshiba2Lane;
  const auto* mode = hdmi_edid::find_mode(width, height, fps);
  if (!mode ||
      (limit_pixel_clock && mode->pixel_clock_10khz > 7425)) {
    log->error("Unsupported HDMI-to-CSI mode {}x{}@{}", width, height, fps);
    return {};
  }

  const auto bridge = find_subdevice(rk628 ? "rk628" : "tc35874");
  const auto capture = rk628
      ? v4l2::find_unique_capture_device("/sys/class/video4linux",
                                         "stream_cif_mipi_id0")
      : v4l2::find_rockchip_isp_mainpath();
  if (bridge.empty() || !capture) {
    log->warn("Cannot find the HDMI bridge and Rockchip capture nodes");
    return {};
  }

  const int fd = open(bridge.c_str(), O_RDWR | O_CLOEXEC);
  if (fd < 0) return {};
  const auto result = hdmi_edid::apply(fd, *mode);
  if (result == hdmi_edid::ApplyResult::Failed)
    log->error("Cannot program HDMI EDID on {}: errno {}", bridge, errno);
  bool ready = result != hdmi_edid::ApplyResult::Failed &&
               hdmi_edid::wait_for_input(fd, *mode,
                   limit_pixel_clock ? 74500000u : 149000000u,
                   result == hdmi_edid::ApplyResult::Written);
  v4l2_subdev_format bridge_format{};
  bridge_format.which = V4L2_SUBDEV_FORMAT_ACTIVE;
  bridge_format.format.width = width;
  bridge_format.format.height = height;
  bridge_format.format.code = MEDIA_BUS_FMT_UYVY8_2X8;
  ready = ready && call(fd, VIDIOC_SUBDEV_S_FMT, &bridge_format) == 0 &&
          bridge_format.format.code == MEDIA_BUS_FMT_UYVY8_2X8;
  close(fd);
  if (!ready) {
    log->warn("Waiting for progressive HDMI input at {}x{}@{}", width, height, fps);
    return {};
  }
  if (rk628) return *capture;

  const auto isp = find_subdevice("rkisp-isp-subdev");
  const int isp_fd = open(isp.c_str(), O_RDWR | O_CLOEXEC);
  if (isp_fd < 0) return {};
  v4l2_subdev_format format{};
  format.which = V4L2_SUBDEV_FORMAT_ACTIVE;
  format.format.width = width;
  format.format.height = height;
  format.format.code = bridge_format.format.code;
  format.format.field = V4L2_FIELD_NONE;
  ready = call(isp_fd, VIDIOC_SUBDEV_S_FMT, &format) == 0;
  v4l2_subdev_selection crop{};
  crop.which = V4L2_SUBDEV_FORMAT_ACTIVE;
  crop.target = V4L2_SEL_TGT_CROP;
  crop.r.width = width;
  crop.r.height = height;
  ready = ready && call(isp_fd, VIDIOC_SUBDEV_S_SELECTION, &crop) == 0;
  format.pad = 2;
  format.format.code = MEDIA_BUS_FMT_YUYV8_2X8;
  ready = ready && call(isp_fd, VIDIOC_SUBDEV_S_FMT, &format) == 0;
  crop.pad = 2;
  ready = ready && call(isp_fd, VIDIOC_SUBDEV_S_SELECTION, &crop) == 0 &&
          crop.r.width == static_cast<unsigned>(width) &&
          crop.r.height == static_cast<unsigned>(height);
  close(isp_fd);
  if (!ready) log->error("Cannot set the Rockchip ISP to the HDMI input mode");
  return ready ? *capture : std::string{};
}

}  // namespace openhd::rockchip_hdmi_csi
