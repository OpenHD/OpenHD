#ifndef GSTREAMERSTREAM_H
#define GSTREAMERSTREAM_H

#include <gst/gst.h>

#include <array>
#include <memory>
#include <mutex>
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
  void setup();
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

 private:
  // points to a running gst pipeline instance
  GstElement* m_gst_pipeline = nullptr;
  // pull samples (fragments) out of the gstreamer pipeline
  GstElement* m_app_sink_element = nullptr;
  // not supported by all camera(s).
  // for dynamically changing the bitrate
  std::optional<GstBitrateControlElement> m_bitrate_ctrl_element = std::nullopt;
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
