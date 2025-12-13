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

#pragma once

#include <optional>
#include <string>

#include "openhd_spdlog.h"

namespace openhd::nxp {

struct NxpEncoderNode {
  std::string device_path;
  bool supports_cir{false};
};

// Enumerate video devices and return the first mem2mem encoder that advertises
// H.264/H.265 capture formats.
std::optional<NxpEncoderNode> find_v4l2_encoder_node(bool needs_hevc);

// Translate RPi-style intra refresh period (frames) to V4L2 MB-per-frame value.
int calculate_cyclic_intra_refresh_mb(int width_px, int height_px,
                                      int period_frames);

// Set V4L2_CID_MPEG_VIDEO_CYCLIC_INTRA_REFRESH_MB if supported. Logs once when
// unsupported. Returns true on success.
bool set_cyclic_intra_refresh(const NxpEncoderNode& node, int width_px,
                              int height_px, int period_frames,
                              const std::shared_ptr<spdlog::logger>& logger);

// Emit a small set of sample calculations for debug builds / unit-ish
// validation.
void log_cir_examples(const std::shared_ptr<spdlog::logger>& logger);

}  // namespace openhd::nxp

