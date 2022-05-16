//
// Created by consti10 on 03.05.22.
//

#ifndef OPENHD_VIDEO_OHDVIDEO_H
#define OPENHD_VIDEO_OHDVIDEO_H

#include <fstream>

#include <iostream>
#include <iterator>
#include <exception>

#include "openhd-platform.hpp"
#include "openhd-profile.hpp"
#include "openhd-log.hpp"

#include "camerastream.h"
#include "openhd-camera.hpp"

#include <string>

/**
 * Main entry point for OpenHD video streaming.
 * NOTE: This module only needs to be run on air pi, so to say it is a "Video stream camera wrapper".
 * TODO: We need to feed mavlink commands coming from telemetry into here & send them to the according streams.
 */
class OHDVideo {
 public:
  /**
   * Reads the detected cameras (from ohd_system & json) then creates an according stream for each of them.
   * @param is_air MUST ALWAYS RUN ON AIR ONLY, there are no cameras on the ground pi and the transmission
   * (and optionally forwarding) of the generated video stream(s) is done by ohd_interface.
   * @param unit_id stephen
   * @param platform_type the platform we are running on.
   */
  OHDVideo(const OHDPlatform &platform, const OHDProfile &profile);
  // Debug stuff LOL :)
  void debug() const;
 private:
  const OHDPlatform &platform;
  const OHDProfile &profile;
 private:
  // These members are what used to be in camera microservice
  // All the created camera streams
  std::vector<std::unique_ptr<CameraStream>> m_camera_streams;
  // each camera stream already links camera, is this duplicated ??!
  std::vector<Camera> m_cameras;
  // these methods are from camera microservice
  void setup();
  void process_manifest();
  void configure(Camera &camera);
};

#endif //OPENHD_VIDEO_OHDVIDEO_H
