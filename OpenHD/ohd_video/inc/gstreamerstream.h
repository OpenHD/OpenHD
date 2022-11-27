#ifndef GSTREAMERSTREAM_H
#define GSTREAMERSTREAM_H

#include <gst/gst.h>

#include <array>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>

#include "camerastream.h"
#include "camera_settings.hpp"
#include "openhd-platform.hpp"

#include "openhd-spdlog.hpp"

// Implementation of OHD CameraStream for pretty much everything, using
// gstreamer.

class GStreamerStream : public CameraStream {
 public:
  GStreamerStream(PlatformType platform,std::shared_ptr<CameraHolder> camera_holder,
                  uint16_t video_udp_port);
  ~GStreamerStream();
  void setup() override;
 private:
  void setup_raspberrypi_csi();
  void setup_libcamera();
  void setup_jetson_csi();
  void setup_rockchip_hdmi();
  void setup_usb_uvc();
  void setup_usb_uvch264();
  void setup_ip_camera();
  void setup_sw_dummy_camera();
  void restart_after_new_setting();
  void restartIfStopped() override;
  void handle_change_bitrate_request(openhd::ActionHandler::LinkBitrateInformation lb) override;
 public:
  void start() override;
  void stop() override;
  void cleanup_pipe();
  std::string createDebug() override;
 private:
  // We cannot create the debug state while performing a restart
  std::mutex m_pipeline_mutex;
  // points to a running gst pipeline instance (unless in stopped & cleaned up state)
  GstElement *m_gst_pipeline = nullptr;
  // not supported by all camera(s). The element in the pipeline that has a bitrate property
  // for dynamically changing the bitrate
  GstElement *m_bitrate_ctrl_element= nullptr;
  // some encoders take the bitrate as kBit/s, some take it as bits per second
  bool m_bitrate_ctrl_element_takes_kbit=false;
  // The pipeline that is started in the end
  std::stringstream m_pipeline_content;
  // To reduce the time on the param callback(s) - they need to return immediately to not block the param server
  void restart_async();
  std::mutex m_async_thread_mutex;
  std::unique_ptr<std::thread> m_async_thread =nullptr;
  std::shared_ptr<spdlog::logger> m_console;
 private:
  // Change the bitrate without re-starting the whole pipeline if supported by the camera.
  // This is needed for variable rf link bitrate(s)
  // returns true on success, false otherwise
  bool try_dynamically_change_bitrate(uint32_t bitrate_kbits);
  uint32_t m_curr_dynamic_bitrate_kbits =-1;
};

#endif
