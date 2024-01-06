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
#include "nalu/CodecConfigFinder.hpp"

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
  void start_looping() override;
  void terminate_looping() override;
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
  void setup();
  // Set gst state to PLAYING
  void start();
  // Set gst state to PAUSED
  void stop();
  // Set gst state to GST_STATE_NULL and properly cleanup the pipeline.
  void cleanup_pipe();
  void handle_change_bitrate_request(openhd::ActionHandler::LinkBitrateInformation lb) override;
  // this is called when the FC reports itself as armed / disarmed
  void handle_update_arming_state(bool armed) override;
  void loop_infinite();
  void stream_once();
  // To reduce the time on the param callback(s) - they need to return immediately to not block the param server
  void request_restart();
 private:
  // points to a running gst pipeline instance
  GstElement *m_gst_pipeline = nullptr;
  // pull samples (fragments) out of the gstreamer pipeline
  GstElement *m_app_sink_element = nullptr;
  // not supported by all camera(s).
  // for dynamically changing the bitrate
  std::optional<GstBitrateControlElement> m_bitrate_ctrl_element=std::nullopt;
  // The pipeline that is started in the end
  std::stringstream m_pipeline_content;
  // If a pipeline is started with air recording enabled, the file name the recording is written to is stored here
  // otherwise, it is set to std::nullopt
  std::optional<std::string> m_opt_curr_recording_filename=std::nullopt;
  std::shared_ptr<spdlog::logger> m_console;
  // Set to true if armed, used for auto record on arm
  bool m_armed_enable_air_recording= false;
  std::atomic<int> m_curr_dynamic_bitrate_kbits =-1;
  std::shared_ptr<openhd::ActionHandler> m_opt_action_handler=nullptr;
  // Not working yet, keep the old approach
  //std::unique_ptr<GstVideoRecorder> m_gst_video_recorder=nullptr;
  std::atomic_bool m_request_restart= false;
  std::atomic_bool m_keep_looping=false;
  std::unique_ptr<std::thread> m_loop_thread=nullptr;
 private:
  // The stuff here is to pull the data out of the gstreamer pipeline, such that we can forward it to the WB link
  void on_new_rtp_frame_fragment(std::shared_ptr<std::vector<uint8_t>> fragment,uint64_t dts);
  void on_new_rtp_fragmented_frame(std::vector<std::shared_ptr<std::vector<uint8_t>>> frame_fragments);
  void on_new_raw_frame(std::shared_ptr<std::vector<uint8_t>> frame,uint64_t dts);
  std::vector<std::shared_ptr<std::vector<uint8_t>>> m_frame_fragments;
  bool dirty_use_raw= false;
  void on_gst_nalu_buffer(const uint8_t* data,int data_len);
  void on_new_nalu(const uint8_t* data,int data_len);
  void on_new_nalu_frame(const uint8_t* data,int data_len);
  void forward_video_frame(std::shared_ptr<std::vector<uint8_t>> frame);
  CodecConfigFinder m_config_finder;
};

#endif
