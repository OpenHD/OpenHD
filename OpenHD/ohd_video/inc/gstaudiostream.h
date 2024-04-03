//
// Created by consti10 on 27.02.24.
//

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
