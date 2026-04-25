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

#ifndef GSTREAMERSTREAM_H
#define GSTREAMERSTREAM_H

#include <gst/gst.h>

#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "camera_settings.hpp"
#include "camerastream.h"
#include "gst_bitrate_controll_wrapper.hpp"
#include "openhd_platform.h"
#include "openhd_spdlog.h"
// #include "gst_recorder.h"
#include "nalu/CodecConfigFinder.hpp"
#include "openhd_rtp.h"

// Implementation of OHD CameraStream for pretty much everything, using
// gstreamer.
// NOTE: What we are doing here essentially is creating a big gstreamer pipeline
// string and then executing this pipeline. This makes development easy (since
// you can just test the pipeline(s) manually using gst-launch and add settings
// and more this way) but you are encouraged to use other approach(es) if they
// better fit your needs (see CameraStream.h)
class GStreamerStream : public CameraStream {
 public:
  GStreamerStream(std::shared_ptr<CameraHolder> camera_holder,
                  openhd::ON_ENCODE_FRAME_CB out_cb);
  ~GStreamerStream();
  void start_looping() override;
  void terminate_looping() override;

 private:
  // Creates a valid gstreamer pipeline for the given camera,
  // including the source and encoder, not including appsink
  std::string create_source_encode_pipeline(const CameraHolder& cam_holder);
  bool setup();
  // Set gst state to PLAYING
  void start();
  // Set gst state to PAUSED
  void stop();
  // Set gst state to GST_STATE_NULL and properly cleanup the pipeline.
  void cleanup_pipe();
  void handle_change_bitrate_request(
      openhd::LinkActionHandler::LinkBitrateInformation lb) override;
  // this is called when the FC reports itself as armed / disarmed
  void handle_update_arming_state(bool armed) override;
  void loop_infinite();
  void stream_once();
  // To reduce the time on the param callback(s) - they need to return
  // immediately to not block the param server
  void request_restart();
  static GstBusSyncReply on_gst_bus_message(GstBus* bus, GstMessage* message,
                                            gpointer user_data);
  void handle_gst_message(GstMessage* message);
  bool setup_perf_element();
  void cleanup_perf_element();
  bool handle_perf_info_message(const char* info_text);
  void check_required_perf_telemetry(int64_t now_ms, int64_t first_frame_ms);
  void report_required_perf_problem(const std::string& code,
                                    const std::string& description);
  bool should_skip_runtime_bitrate_update() const;

 private:
  // points to a running gst pipeline instance
  GstElement* m_gst_pipeline = nullptr;
  // pull samples (fragments) out of the gstreamer pipeline
  GstElement* m_app_sink_element = nullptr;
  // not supported by all camera(s).
  // for dynamically changing the bitrate
  std::optional<GstBitrateControlElement> m_bitrate_ctrl_element = std::nullopt;
  // Required gst-perf element that reports encoder-side bitrate/fps.
  GstElement* m_perf_element = nullptr;
  GstBus* m_gst_bus = nullptr;
  // If a pipeline is started with air recording enabled, the file name the
  // recording is written to is stored here otherwise, it is set to std::nullopt
  std::optional<std::string> m_opt_curr_recording_filename = std::nullopt;
  std::shared_ptr<spdlog::logger> m_console;
  // Set to true if armed, used for auto record on arm
  bool m_armed_enable_air_recording = false;
  std::atomic<int> m_curr_dynamic_bitrate_kbits = -1;
  // Not working yet, keep the old approach
  // std::unique_ptr<GstVideoRecorder> m_gst_video_recorder=nullptr;
  std::atomic_bool m_request_restart = false;
  std::atomic_bool m_keep_looping = false;
  std::unique_ptr<std::thread> m_loop_thread = nullptr;
  std::atomic_bool m_loop_exited = true;
  std::mutex m_loop_mutex;
  std::condition_variable m_loop_cv;
  std::chrono::steady_clock::time_point m_last_log_skip_dynamic_bitrate =
      std::chrono::steady_clock::now();
  std::atomic<int64_t> m_last_perf_message_ms = 0;
  std::atomic<int64_t> m_last_required_perf_problem_ms = 0;
  int64_t m_last_perf_warning_ms = 0;

 private:
  // The stuff here is to pull the data out of the gstreamer pipeline, such that
  // we can forward it to the WB link
  void on_new_rtp_frame_fragment(std::shared_ptr<std::vector<uint8_t>> fragment,
                                 uint64_t dts);
  void on_new_rtp_fragmented_frame();
  std::vector<std::shared_ptr<std::vector<uint8_t>>> m_frame_fragments;

  void x_on_new_rtp_fragmented_frame(
      std::vector<std::shared_ptr<std::vector<uint8_t>>> frame_fragments);
  bool m_last_fu_s_idr = false;
  bool dirty_use_raw = false;
  std::chrono::steady_clock::time_point m_last_log_streaming_disabled =
      std::chrono::steady_clock::now();

 private:
  std::shared_ptr<openhd::RTPHelper> m_rtp_helper;
};

#endif
