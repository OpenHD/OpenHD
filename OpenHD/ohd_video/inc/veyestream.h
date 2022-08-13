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

// Unfortunately, we do not have an rpicamsrc equivalent for the veye -
// This is a hacky (perhaps temporary) workaround
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
};

#endif //OPENHD_OPENHD_OHD_VIDEO_INC_VEYESTREAM_H_
