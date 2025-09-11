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

#include "mpp_stream.h"

#include <fmt/format.h>

#include <cstdio>

#include "camera_holder.h"
#include "nalu/fragment_helper.h"

namespace {
// Helper to find next start code in a buffer. Returns -1 if none found.
int find_start_code(const std::vector<uint8_t>& data, int offset) {
  for (size_t i = offset; i + 3 < data.size(); ++i) {
    if (data[i] == 0x00 && data[i + 1] == 0x00 && data[i + 2] == 0x00 &&
        data[i + 3] == 0x01) {
      return static_cast<int>(i);
    }
  }
  return -1;
}
}  // namespace

MPPStream::MPPStream(std::shared_ptr<CameraHolder> camera_holder,
                     openhd::ON_ENCODE_FRAME_CB out_cb)
    : CameraStream(std::move(camera_holder), std::move(out_cb)) {
  m_console = openhd::log::create_or_get("mpp_stream");
}

MPPStream::~MPPStream() { terminate_looping(); }

void MPPStream::start_looping() {
  const auto camera = m_camera_holder->get_camera();
  const bool is_h265 =
      m_camera_holder->get_settings().streamed_video_format.videoCodec ==
      VideoCodec::H265;
  m_rtp_helper = std::make_shared<openhd::RTPHelper>(is_h265);
  m_rtp_helper->set_out_cb([this, camera](auto fragments) {
    openhd::FragmentedVideoFrame frame{};
    frame.rtp_fragments = std::move(fragments);
    frame.enable_ultra_secure_encryption =
        m_camera_holder->get_settings().enable_ultra_secure_encryption;
    m_output_cb(camera.index, frame);
  });
  m_keep_looping = true;
  m_thread = std::make_unique<std::thread>(&MPPStream::loop, this);
}

void MPPStream::terminate_looping() {
  m_keep_looping = false;
  if (m_thread && m_thread->joinable()) {
    m_thread->join();
  }
}

void MPPStream::handle_change_bitrate_request(
    openhd::LinkActionHandler::LinkBitrateInformation lb) {
  // Basic implementation – the external encoder would need to be restarted with
  // the new bitrate. For now we simply log the request.
  m_console->info("bitrate change request {} kbit/s", lb.recommended_bitrate);
}

void MPPStream::handle_update_arming_state(bool /*armed*/) {
  // No-op for now. Recording based on arming state is not implemented for MPP
  // mode yet.
}

void MPPStream::loop() {
  const auto settings = m_camera_holder->get_settings();
  const int width = settings.streamed_video_format.width;
  const int height = settings.streamed_video_format.height;
  const int fps = settings.streamed_video_format.framerate;
  const int bitrate = settings.h26x_bitrate_kbits;

  // Command to spawn external MPP encoder. The "mpp_stream" tool is expected to
  // output a raw H26x byte stream with start codes to stdout.
  const std::string cmd = fmt::format(
      "mpp_stream --width {} --height {} --fps {} --bitrate {} --output -",
      width, height, fps, bitrate);

  m_console->info("Starting MPP encoder: {}", cmd);
  FILE* pipe = popen(cmd.c_str(), "r");
  if (!pipe) {
    m_console->error("Failed to start mpp_stream process");
    return;
  }

  std::vector<uint8_t> buffer(64 * 1024);
  std::vector<uint8_t> pending;
  while (m_keep_looping) {
    size_t bytes = fread(buffer.data(), 1, buffer.size(), pipe);
    if (bytes == 0) {
      break;
    }
    pending.insert(pending.end(), buffer.begin(), buffer.begin() + bytes);
    int start = find_start_code(pending, 0);
    while (start >= 0) {
      int next = find_start_code(pending, start + 4);
      if (next > start) {
        m_rtp_helper->feed_nalu(pending.data() + start, next - start);
        pending.erase(pending.begin(), pending.begin() + next);
        start = find_start_code(pending, 0);
      } else {
        // Keep remaining bytes for next read
        if (start > 0) {
          pending.erase(pending.begin(), pending.begin() + start);
        }
        break;
      }
    }
  }
  pclose(pipe);
  m_console->info("MPP encoder terminated");
}
