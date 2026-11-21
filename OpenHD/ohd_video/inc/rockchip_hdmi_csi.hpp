#pragma once

#include <fcntl.h>
#include <linux/media-bus-format.h>
#include <linux/v4l2-subdev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <string>

#include "openhd_spdlog.h"
#include "v4l2_camera_selection.h"

namespace openhd::rockchip_hdmi_csi {

struct Mode {
  int width, height, fps, pixel_clock_10khz;
  int h_front, h_sync, h_back, v_front, v_sync, v_back;
};

inline constexpr std::array<Mode, 4> modes{{
    // The CTA 720p30 front porch exceeds the EDID detailed timing field.
    {1280, 720, 30, 3713, 110, 40, 220, 5, 5, 20},
    {1280, 720, 60, 7425, 110, 40, 220, 5, 5, 20},
    {1920, 1080, 30, 7425, 88, 44, 148, 4, 5, 36},
    {1920, 1080, 60, 14850, 88, 44, 148, 4, 5, 36},
}};

// Advertise the selected progressive mode as the only detailed timing.
inline std::array<uint8_t, 128> edid_for(const Mode& mode) {
  std::array<uint8_t, 128> edid{};
  constexpr std::array<uint8_t, 25> identity{
      0, 255, 255, 255, 255, 255, 255, 0, 0x3d, 0x04, 1, 0, 0, 0,
      0, 0, 1, 36, 1, 4, 0x80, 16, 9, 120, 0x02};
  std::copy(identity.begin(), identity.end(), edid.begin());
  std::fill(edid.begin() + 38, edid.begin() + 54, 1);
  const int h_blank = mode.h_front + mode.h_sync + mode.h_back;
  const int v_blank = mode.v_front + mode.v_sync + mode.v_back;
  auto* timing = edid.data() + 54;
  timing[0] = mode.pixel_clock_10khz & 0xff;
  timing[1] = mode.pixel_clock_10khz >> 8;
  timing[2] = mode.width & 0xff;
  timing[3] = h_blank & 0xff;
  timing[4] = (mode.width >> 8) << 4 | (h_blank >> 8);
  timing[5] = mode.height & 0xff;
  timing[6] = v_blank & 0xff;
  timing[7] = (mode.height >> 8) << 4 | (v_blank >> 8);
  timing[8] = mode.h_front & 0xff;
  timing[9] = mode.h_sync & 0xff;
  timing[10] = (mode.v_front & 0xf) << 4 | (mode.v_sync & 0xf);
  timing[11] = (mode.h_front >> 8) << 6 | (mode.h_sync >> 8) << 4 |
               (mode.v_front >> 4) << 2 | (mode.v_sync >> 4);
  timing[12] = 160;
  timing[13] = 90;
  timing[17] = 0x1e;
  for (const int offset : {72, 90, 108}) edid[offset + 3] = 0x10;
  const auto sum = std::accumulate(edid.begin(), edid.end(), 0u);
  edid.back() = static_cast<uint8_t>(0u - sum);
  return edid;
}

inline int call(int fd, unsigned long request, void* data) {
  int result;
  do {
    result = ioctl(fd, request, data);
  } while (result < 0 && errno == EINTR);
  return result;
}

inline std::string find_subdevice(const std::string& name) {
  std::error_code error;
  for (std::filesystem::directory_iterator it("/sys/class/video4linux", error),
       end; !error && it != end; it.increment(error)) {
    const auto device = it->path().filename().string();
    if (device.rfind("v4l-subdev", 0) != 0) continue;
    std::ifstream file(it->path() / "name");
    std::string value;
    if (std::getline(file, value) && value.find(name) != std::string::npos)
      return "/dev/" + device;
  }
  return {};
}

enum class Bridge { Toshiba2Lane, Toshiba4Lane, RadxaRk628 };

inline std::string setup_capture(int width, int height, int fps, Bridge type) {
  const auto log = openhd::log::get_default();
  const bool rk628 = type == Bridge::RadxaRk628;
  const bool limit_pixel_clock = type == Bridge::Toshiba2Lane;
  const auto mode = std::find_if(modes.begin(), modes.end(), [&](const Mode& m) {
    return m.width == width && m.height == height && m.fps == fps;
  });
  if (mode == modes.end() ||
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
  auto edid = edid_for(*mode);
  decltype(edid) current{};
  v4l2_edid request{};
  request.blocks = 1;
  request.edid = current.data();
  bool ready = call(fd, VIDIOC_SUBDEV_G_EDID, &request) == 0 &&
               request.blocks == 1 && current == edid;
  if (!ready) {
    request = {};
    request.blocks = 1;
    request.edid = edid.data();
    ready = call(fd, VIDIOC_SUBDEV_S_EDID, &request) == 0;
    if (!ready) log->error("Cannot program HDMI EDID: errno {}", errno);
  }

  v4l2_dv_timings detected{};
  ready = ready && call(fd, VIDIOC_SUBDEV_QUERY_DV_TIMINGS, &detected) == 0;
  if (ready) {
    const auto& bt = detected.bt;
    const uint64_t pixels = static_cast<uint64_t>(bt.width + bt.hfrontporch +
                            bt.hsync + bt.hbackporch) *
                            (bt.height + bt.vfrontporch + bt.vsync + bt.vbackporch);
    // Fractional HDMI frame rates round to the corresponding integer mode.
    ready = detected.type == V4L2_DV_BT_656_1120 && !bt.interlaced &&
            bt.width == static_cast<unsigned>(width) &&
            bt.height == static_cast<unsigned>(height) && pixels &&
            (bt.pixelclock + pixels / 2) / pixels == static_cast<unsigned>(fps) &&
            bt.pixelclock <= (limit_pixel_clock ? 74500000u : 149000000u) &&
            call(fd, VIDIOC_SUBDEV_S_DV_TIMINGS, &detected) == 0;
  }
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
