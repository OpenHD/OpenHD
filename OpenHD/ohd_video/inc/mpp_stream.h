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

#ifndef OPENHD_MPP_STREAM_H
#define OPENHD_MPP_STREAM_H

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include "camerastream.h"
#include "openhd_rtp.h"
#include "openhd_spdlog.h"

/**
 * Basic CameraStream implementation that uses the Rockchip MPP encoder instead
 * of gstreamer. The encoder is spawned as an external process and its output is
 * parsed into NAL units which are then packetised into RTP fragments.
 *
 * The implementation is intentionally minimal and serves as a starting point
 * for platforms where gstreamer is not desired. The external encoder command
 * must be available on the system and accessible as "mpp_stream".
 */
class MPPStream : public CameraStream {
 public:
  MPPStream(std::shared_ptr<CameraHolder> camera_holder,
            openhd::ON_ENCODE_FRAME_CB out_cb);
  ~MPPStream() override;

  void start_looping() override;
  void terminate_looping() override;
  void handle_change_bitrate_request(
      openhd::LinkActionHandler::LinkBitrateInformation lb) override;
  void handle_update_arming_state(bool armed) override;

 private:
  void loop();
  std::shared_ptr<openhd::RTPHelper> m_rtp_helper;
  std::atomic_bool m_keep_looping{false};
  std::unique_ptr<std::thread> m_thread;
  std::shared_ptr<spdlog::logger> m_console;
};

#endif  // OPENHD_MPP_STREAM_H
