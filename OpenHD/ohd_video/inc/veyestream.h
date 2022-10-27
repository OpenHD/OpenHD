//
// Created by consti10 on 13.08.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_VEYESTREAM_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_VEYESTREAM_H_

#include <array>
#include <vector>
#include <memory>
#include <thread>

#include "camerastream.h"
#include "openhd-camera.hpp"
#include "openhd-platform.hpp"
#include "openhd-spdlog.hpp"

// Unfortunately, we do not have an rpicamsrc equivalent for the veye -
// This is a hacky (perhaps temporary) workaround
// NOTE: The problem here is that we cannot just run a gstreamer pipe from c++ via gst_parse_launch() but rather
// run veye_raspivid and pipe its output into a gstreamer instance.
// R.n I have this one setup really similar to how video was done in EZ-WB and more, and honestly, i quite like it
// Since it is that simple. A disadvantage though is that debugging is much more complicated, since you now don't debug
// a c++ application, but rather some processes started by a c++ application.
class VEYEStream :public CameraStream {
 public:
  VEYEStream(PlatformType platform,std::shared_ptr<CameraHolder> camera_holder,
			 uint16_t video_udp_port);
  void setup() override;
 public:
  void start() override;
  void stop() override;
  std::string createDebug() override;
 private:
  void restartIfStopped() override;
  void restart_async();
 private:
  std::string pipeline="";
  //std::unique_ptr<std::thread> _veye_thread=nullptr;
  std::shared_ptr<spdlog::logger> m_console;
};

#endif //OPENHD_OPENHD_OHD_VIDEO_INC_VEYESTREAM_H_
