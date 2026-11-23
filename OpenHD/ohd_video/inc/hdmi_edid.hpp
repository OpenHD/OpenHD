#pragma once

#include <linux/v4l2-subdev.h>
#include <sys/ioctl.h>

#include <algorithm>
#include <memory>
#include <array>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <numeric>
#include <thread>

namespace openhd::hdmi_edid {

struct Mode {
  int width;
  int height;
  int fps;
  int pixel_clock_10khz;
  int h_front;
  int h_sync;
  int h_back;
  int v_front;
  int v_sync;
  int v_back;
  uint8_t vic;
};

inline constexpr std::array<Mode, 4> modes{{
    // CTA 720p30 needs a front porch too large for an EDID detailed timing.
    // Use its CTA VIC plus the compact 30 Hz detailed timing for DTD readers.
    {1280, 720, 30, 3713, 110, 40, 220, 5, 5, 20, 62},
    {1280, 720, 60, 7425, 110, 40, 220, 5, 5, 20, 4},
    {1920, 1080, 30, 7425, 88, 44, 148, 4, 5, 36, 34},
    {1920, 1080, 60, 14850, 88, 44, 148, 4, 5, 36, 16},
}};

inline const Mode* find_mode(int width, int height, int fps) {
  const auto mode = std::find_if(modes.begin(), modes.end(), [&](const Mode& m) {
    return m.width == width && m.height == height && m.fps == fps;
  });
  return mode == modes.end() ? nullptr : std::addressof(*mode);
}

using Edid = std::array<uint8_t, 256>;
inline void finish_block(uint8_t* block) {
  block[127] = static_cast<uint8_t>(0u - std::accumulate(block, block + 127, 0u));
}

// Advertise the selected progressive mode as the only detailed timing.
inline Edid edid_for(const Mode& mode, bool cta = true) {
  Edid edid{};
  constexpr std::array<uint8_t, 25> identity{
      0, 255, 255, 255, 255, 255, 255, 0, 0x3d, 0x04, 1, 0, 0, 0,
      0, 0, 1, 36, 1, 3, 0x80, 48, 27, 120, 0x06};
  std::copy(identity.begin(), identity.end(), edid.begin());
  // Standard sRGB primaries and the baseline 640x480p60 HDMI mode.
  constexpr std::array<uint8_t, 10> srgb{
      0xee, 0x91, 0xa3, 0x54, 0x4c, 0x99, 0x26, 0x0f, 0x50, 0x54};
  std::copy(srgb.begin(), srgb.end(), edid.begin() + 25);
  edid[35] = 0x20;
  std::fill(edid.begin() + 38, edid.begin() + 54, 1);
  const int h_blank = mode.h_front + mode.h_sync + mode.h_back;
  const int v_blank = mode.v_front + mode.v_sync + mode.v_back;
  auto* timing = edid.data() + 54;
  timing[0] = static_cast<uint8_t>(mode.pixel_clock_10khz & 0xff);
  timing[1] = static_cast<uint8_t>(mode.pixel_clock_10khz >> 8);
  timing[2] = static_cast<uint8_t>(mode.width & 0xff);
  timing[3] = static_cast<uint8_t>(h_blank & 0xff);
  timing[4] = static_cast<uint8_t>((mode.width >> 8) << 4 | (h_blank >> 8));
  timing[5] = static_cast<uint8_t>(mode.height & 0xff);
  timing[6] = static_cast<uint8_t>(v_blank & 0xff);
  timing[7] = static_cast<uint8_t>((mode.height >> 8) << 4 | (v_blank >> 8));
  timing[8] = static_cast<uint8_t>(mode.h_front & 0xff);
  timing[9] = static_cast<uint8_t>(mode.h_sync & 0xff);
  timing[10] = static_cast<uint8_t>((mode.v_front & 0xf) << 4 | (mode.v_sync & 0xf));
  timing[11] = static_cast<uint8_t>((mode.h_front >> 8) << 6 | (mode.h_sync >> 8) << 4 |
               (mode.v_front >> 4) << 2 | (mode.v_sync >> 4));
  timing[12] = static_cast<uint8_t>(480 & 0xff);
  timing[13] = static_cast<uint8_t>(270 & 0xff);
  timing[14] = static_cast<uint8_t>(0x11);
  timing[17] = static_cast<uint8_t>(0x1e);
  constexpr std::array<uint8_t, 18> name{
      0, 0, 0, 0xfc, 0, 'O', 'p', 'e', 'n', 'H', 'D', ' ', 'I', 'n', 10,
      ' ', ' ', ' '};
  std::copy(name.begin(), name.end(), edid.begin() + 72);
  constexpr std::array<uint8_t, 18> range{
      0, 0, 0, 0xfd, 0, 30, 60, 22, 68, 15, 0, 10,
      ' ', ' ', ' ', ' ', ' ', ' '};
  std::copy(range.begin(), range.end(), edid.begin() + 90);
  edid[111] = 0x10;
  if (cta) {
    edid[126] = 1;
    auto* extension = edid.data() + 128;
    // The selected mode is native. VIC 1 is the required HDMI baseline.
    // Include HDMI identification and selectable RGB quantization.
    const std::array<uint8_t, 18> blocks{
        2, 3, 18, 0xb0,
        0x42, static_cast<uint8_t>(0x80 | mode.vic), 1,
        0x67, 0x03, 0x0c, 0x00, 0x00, 0x00, 0x00, 30,
        0xe2, 0x00, 0x6a};
    std::copy(blocks.begin(), blocks.end(), extension);
    finish_block(extension);
  }
  finish_block(edid.data());
  return edid;
}

template <typename Payload>
inline int call(int fd, unsigned long request, Payload* data) {
  int result;
  do {
    result = ioctl(fd, request, data);
  } while (result < 0 && errno == EINTR);
  return result;
}

enum class ApplyResult { Unchanged, Written, Failed };

inline ApplyResult apply(int fd, const Mode& mode) {
  for (const bool cta : {true, false}) {
    const unsigned blocks = cta ? 2 : 1;
    auto wanted = edid_for(mode, cta);
    Edid current{};
    v4l2_edid request{};
    request.blocks = blocks;
    request.edid = current.data();
    if (call(fd, VIDIOC_SUBDEV_G_EDID, &request) == 0 && request.blocks == blocks &&
        current == wanted)
      return ApplyResult::Unchanged;

    request = {};
    request.blocks = blocks;
    request.edid = wanted.data();
    if (call(fd, VIDIOC_SUBDEV_S_EDID, &request) == 0) return ApplyResult::Written;
    // Some receivers only expose room for the 128-byte base block.
    if (errno != E2BIG) break;
  }
  return ApplyResult::Failed;
}

inline bool matches(const v4l2_dv_timings& detected, const Mode& mode,
                    uint64_t max_pixel_clock) {
  const auto& bt = detected.bt;
  const uint64_t pixels = static_cast<uint64_t>(bt.width + bt.hfrontporch +
                          bt.hsync + bt.hbackporch) *
                          (bt.height + bt.vfrontporch + bt.vsync + bt.vbackporch);
  return detected.type == V4L2_DV_BT_656_1120 && !bt.interlaced &&
         bt.width == static_cast<unsigned>(mode.width) &&
         bt.height == static_cast<unsigned>(mode.height) && pixels &&
         (bt.pixelclock + pixels / 2) / pixels == static_cast<unsigned>(mode.fps) &&
         bt.pixelclock <= max_pixel_clock;
}

inline bool wait_for_input(int fd, const Mode& mode, uint64_t max_pixel_clock,
                           bool renegotiating) {
  using namespace std::chrono_literals;
  // Let the HPD pulse clear the old signal before querying the new mode.
  const auto deadline = std::chrono::steady_clock::now() + 2s;
  if (renegotiating) std::this_thread::sleep_for(250ms);
  do {
    if (v4l2_dv_timings detected{};
        call(fd, VIDIOC_SUBDEV_QUERY_DV_TIMINGS, &detected) == 0 &&
        matches(detected, mode, max_pixel_clock) &&
        call(fd, VIDIOC_SUBDEV_S_DV_TIMINGS, &detected) == 0)
      return true;
    if (!renegotiating) break;
    std::this_thread::sleep_for(100ms);
  } while (std::chrono::steady_clock::now() < deadline);
  return false;
}

}  // namespace openhd::hdmi_edid
