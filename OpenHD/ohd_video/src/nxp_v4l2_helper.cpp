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

#include "nxp_v4l2_helper.h"

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cmath>
#include <set>
#include <tuple>
#include <vector>

#include <fmt/format.h>

namespace openhd::nxp {

namespace {

constexpr int MAX_V4L2_NODE = 63;

bool supports_codec(int fd, __u32 pixfmt) {
  v4l2_fmtdesc fmt{};
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  while (ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == 0) {
    if (fmt.pixelformat == pixfmt) return true;
    fmt.index++;
  }
  return false;
}

bool is_mem2mem_encoder(const v4l2_capability& caps) {
  const bool m2m = caps.device_caps & V4L2_CAP_VIDEO_M2M ||
                   caps.device_caps & V4L2_CAP_VIDEO_M2M_MPLANE;
  const bool capture = caps.device_caps & V4L2_CAP_VIDEO_CAPTURE ||
                       caps.device_caps & V4L2_CAP_VIDEO_CAPTURE_MPLANE;
  const bool output = caps.device_caps & V4L2_CAP_VIDEO_OUTPUT ||
                      caps.device_caps & V4L2_CAP_VIDEO_OUTPUT_MPLANE;
  return m2m && capture && output;
}

}  // namespace

std::optional<NxpEncoderNode> find_v4l2_encoder_node(bool needs_hevc) {
  std::set<int> seen;
  for (int idx = 0; idx <= MAX_V4L2_NODE; ++idx) {
    if (seen.count(idx) != 0) continue;
    const auto path = fmt::format("/dev/video{}", idx);
    const int fd = open(path.c_str(), O_RDWR | O_NONBLOCK);
    if (fd < 0) continue;
    seen.insert(idx);

    v4l2_capability caps{};
    if (ioctl(fd, VIDIOC_QUERYCAP, &caps) != 0) {
      close(fd);
      continue;
    }

    if (!is_mem2mem_encoder(caps)) {
      close(fd);
      continue;
    }

    const bool has_h264 = supports_codec(fd, V4L2_PIX_FMT_H264);
    const bool has_h265 = supports_codec(fd, V4L2_PIX_FMT_HEVC);
    if (!has_h264 && !has_h265) {
      close(fd);
      continue;
    }
    if (needs_hevc && !has_h265) {
      close(fd);
      continue;
    }

    v4l2_queryctrl qctrl{};
    qctrl.id = V4L2_CID_MPEG_VIDEO_CYCLIC_INTRA_REFRESH_MB;
    const bool supports_cir = ioctl(fd, VIDIOC_QUERYCTRL, &qctrl) == 0 &&
                              !(qctrl.flags & V4L2_CTRL_FLAG_DISABLED);
    close(fd);
    return NxpEncoderNode{path, supports_cir};
  }
  return std::nullopt;
}

int calculate_cyclic_intra_refresh_mb(int width_px, int height_px,
                                      int period_frames) {
  const int mb_w = (width_px + 15) / 16;
  const int mb_h = (height_px + 15) / 16;
  const int total_mbs = mb_w * mb_h;
  if (period_frames <= 0) return 0;
  int mbs_per_frame = static_cast<int>(
      std::ceil(static_cast<double>(total_mbs) / period_frames));
  if (mbs_per_frame < 1) mbs_per_frame = 1;
  if (mbs_per_frame > total_mbs) mbs_per_frame = total_mbs;
  return mbs_per_frame;
}

bool set_cyclic_intra_refresh(const NxpEncoderNode& node, int width_px,
                              int height_px, int period_frames,
                              const std::shared_ptr<spdlog::logger>& logger) {
  static bool logged_unsupported = false;
  if (!node.supports_cir) {
    if (!logged_unsupported && logger) {
      logger->warn("CIR not supported on this encoder");
    }
    logged_unsupported = true;
    return false;
  }

  const int value = calculate_cyclic_intra_refresh_mb(width_px, height_px,
                                                      period_frames);
  const int fd = open(node.device_path.c_str(), O_RDWR | O_NONBLOCK);
  if (fd < 0) return false;

  v4l2_control ctrl{};
  ctrl.id = V4L2_CID_MPEG_VIDEO_CYCLIC_INTRA_REFRESH_MB;
  ctrl.value = value;
  const bool ok = ioctl(fd, VIDIOC_S_CTRL, &ctrl) == 0;
  close(fd);

  if (logger) {
    const int mb_w = (width_px + 15) / 16;
    const int mb_h = (height_px + 15) / 16;
    const int total_mbs = mb_w * mb_h;
    if (value == 0) {
      logger->info("CIR disabled");
    } else {
      logger->info("CIR enabled: period={} frames => {} MB/frame (total={})",
                   period_frames, value, total_mbs);
    }
  }

  return ok;
}

void log_cir_examples(const std::shared_ptr<spdlog::logger>& logger) {
  if (!logger) return;
  const std::vector<std::tuple<int, int, int>> samples = {
      {1280, 720, 30}, {1920, 1080, 30}, {640, 480, 30}};
  for (const auto& sample : samples) {
    const auto mb = calculate_cyclic_intra_refresh_mb(
        std::get<0>(sample), std::get<1>(sample), std::get<2>(sample));
    logger->debug("{}x{} period={} -> {} MB/frame", std::get<0>(sample),
                  std::get<1>(sample), std::get<2>(sample), mb);
  }
}

}  // namespace openhd::nxp

