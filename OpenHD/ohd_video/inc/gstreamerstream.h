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
//#include "gst_recorder.h"

// Implementation of OHD CameraStream for pretty much everything, using
// gstreamer.
// NOTE: What we are doing here essentially is creating a big gstreamer pipeline string and then
// executing this pipeline. This makes development easy (since you can just test the pipeline(s) manually
// using gst-launch and add settings and more this way) but you are encouraged to use other approach(es) if they
// better fit your needs (see CameraStream.h)
class GStreamerStream : public CameraStream {
 public:
  GStreamerStream(PlatformType platform,std::shared_ptr<CameraHolder> camera_holder,
                  openhd::ON_ENCODE_FRAME_CB out_cb,
                  std::shared_ptr<openhd::ActionHandler> opt_action_handler=nullptr);
  ~GStreamerStream();
  void setup() override;
 private:
  // Build (parts) of the gstreamer pipeline for all the different cameras we support.
  // If params (like for example resolution, framerate, exposure,...) are changeable on the camera type,
  // it sets them accordingly by reading them from @CameraStream::m_camera_holder
  void setup_raspberrypi_mmal_csi();
  void setup_raspberrypi_veye_v4l2();
  void setup_raspberrypi_libcamera();
  void setup_jetson_csi();
  void setup_rockchip_hdmi();
  void setup_rockchip_csi();
  void setup_allwinner_csi();
  void setup_usb_uvc();
  void setup_usb_uvch264();
  void setup_ip_camera();
  void setup_sw_dummy_camera();
  void setup_custom_unmanaged_camera();
  /**
   * Stop and cleanup the pipeline (if running), then re-build and re-start it.
   */
  void stop_cleanup_restart();
  // Utils when settings are changed (most of them require a full restart of the pipeline)
  void restart_after_new_setting();
  void restartIfStopped() override;
  void handle_change_bitrate_request(openhd::ActionHandler::LinkBitrateInformation lb) override;
  // this is called when the FC reports itself as armed / disarmed
  void handle_update_arming_state(bool armed) override;
 public:
  // Set gst state to PLAYING
  void start() override;
  // Set gst state to PAUSED
  void stop() override;
  // Set gst state to GST_STATE_NULL and properly cleanup the pipeline.
  void cleanup_pipe();
  std::string createDebug() override;
 private:
  // We cannot create the debug state while performing a restart
  std::mutex m_pipeline_mutex;
  // points to a running gst pipeline instance (unless in stopped & cleaned up state)
  GstElement *m_gst_pipeline = nullptr;
  // not supported by all camera(s).
  // for dynamically changing the bitrate
  std::optional<GstBitrateControlElement> m_bitrate_ctrl_element=std::nullopt;
  // The pipeline that is started in the end
  std::stringstream m_pipeline_content;
  // If a pipeline is started with air recording enabled, the file name the recording is written to is stored here
  // otherwise, it is set to std::nullopt
  std::optional<std::string> m_opt_curr_recording_filename=std::nullopt;
  // To reduce the time on the param callback(s) - they need to return immediately to not block the param server
  void restart_async();
  std::mutex m_async_thread_mutex;
  std::unique_ptr<std::thread> m_async_thread =nullptr;
  std::shared_ptr<spdlog::logger> m_console;
  std::chrono::steady_clock::time_point m_stream_creation_time=std::chrono::steady_clock::now();
  // This boolean indicates we should record
  bool m_armed_enable_air_recording= false;
 private:
  // Change the bitrate without re-starting the whole pipeline if supported by the camera.
  // This is needed for variable rf link bitrate(s)
  // returns true on success, false otherwise
  bool try_dynamically_change_bitrate(int bitrate_kbits);
  std::atomic<int> m_curr_dynamic_bitrate_kbits =-1;
 private:
  // The stuff here is to pull the data out of the gstreamer pipeline, such that we can forward it to the WB link
  void on_new_rtp_frame_fragment(std::shared_ptr<std::vector<uint8_t>> fragment,uint64_t dts);
  std::vector<std::shared_ptr<std::vector<uint8_t>>> m_frame_fragments;
  void on_new_rtp_fragmented_frame(std::vector<std::shared_ptr<std::vector<uint8_t>>> frame_fragments);
  // pull samples (fragments) out of the gstreamer pipeline
  GstElement *m_app_sink_element = nullptr;
  bool m_pull_samples_run=false;
  std::unique_ptr<std::thread> m_pull_samples_thread;
  void loop_pull_samples();
  std::shared_ptr<openhd::ActionHandler> m_opt_action_handler=nullptr;
 private:
  // Not working yet, keep the old approach
  //std::unique_ptr<GstVideoRecorder> m_gst_video_recorder=nullptr;
};

#endif
