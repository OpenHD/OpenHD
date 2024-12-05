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

#ifndef OPENHD_GSTAUDIOSTREAM_H
#define OPENHD_GSTAUDIOSTREAM_H

#include <gst/gst.h>

#include "openhd_link.hpp"

/**
 * Similar to gstreamerstream
 * only for audio ;)
 * provides rtp audio data from autoaudiosrc
 */
class GstAudioStream {
 public:
  explicit GstAudioStream();
  ~GstAudioStream();
  void set_link_cb(openhd::ON_AUDIO_TX_DATA_PACKET cb);
  void start_looping();
  void stop_looping();
  bool openhd_enable_audio_test = false;

 private:
  void loop_infinite();
  void stream_once();
  void on_audio_packet(std::shared_ptr<std::vector<uint8_t>> packet);
  std::string create_pipeline();
  std::shared_ptr<spdlog::logger> m_console;
  std::atomic_bool m_keep_looping = false;
  std::unique_ptr<std::thread> m_loop_thread = nullptr;
  openhd::ON_AUDIO_TX_DATA_PACKET m_cb = nullptr;

 private:
  // points to a running gst pipeline instance
  GstElement* m_gst_pipeline = nullptr;
  // pull samples (fragments) out of the gstreamer pipeline
  GstElement* m_app_sink_element = nullptr;
};

#endif  // OPENHD_GSTAUDIOSTREAM_H
