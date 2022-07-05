//
// Created by consti10 on 03.05.22.
//

#ifndef OPENHD_VIDEO_OHDVIDEO_H
#define OPENHD_VIDEO_OHDVIDEO_H

#include "camerastream.h"
#include "openhd-platform.hpp"

#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

/**
 * Main entry point for OpenHD video streaming.
 * NOTE: This module only needs to be run on air pi, so to say it is a "Video
 * stream camera wrapper".
 * TODO: We need to feed mavlink commands coming from telemetry into here & send
 * them to the according streams.
 */
class OHDVideo {
 public:
  OHDVideo(OHDPlatform platform1,DiscoveredCameraList cameras);
  /**
   * Create a verbose debug string about the current state of OHDVideo, doesn't
   * print to stdout.
   * @return a verbose debug string.
   */
  [[nodiscard]] std::string createDebug() const;
  /**
   * This should be called in regular intervals by the OpenHD main thread to
   * restart any camera stream if it has unexpectedly stopped.
   */
  void restartIfStopped();
  // can return an empty pointer if stream does not exist
  std::shared_ptr<CameraStream> get_stream_by_index(int idx);

  // This performs the following steps:
  // 1) Discover all the connected camera(s)
  // 2) If no cameras are detected and add_dummy=true, add a dummy camera
  // 3) Create a OHDVideo instance if there are any detected cameras, otherwise, return nullptr
  //static std::shared_ptr<OHDVideo> discover_and_create(const bool add_dummy){
  //}
  std::vector<std::shared_ptr<openhd::XSettingsComponent>> get_setting_components(){
    std::vector<std::shared_ptr<openhd::XSettingsComponent>> ret;
    for(auto& stream: m_camera_streams){
      ret.push_back(stream->_camera_holder);
    }
    return ret;
  }
 private:
  const OHDPlatform platform;
 private:
  // These members are what used to be in camera microservice
  // All the created camera streams
  std::vector<std::shared_ptr<CameraStream>> m_camera_streams;
  void configure(std::shared_ptr<CameraHolder> camera);
};

#endif  // OPENHD_VIDEO_OHDVIDEO_H
