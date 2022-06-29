//
// Created by consti10 on 03.05.22.
//

#ifndef OPENHD_VIDEO_OHDVIDEO_H
#define OPENHD_VIDEO_OHDVIDEO_H

#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "camerastream.h"
#include "openhd-camera.hpp"
#include "openhd-log.hpp"
#include "openhd-platform.hpp"
#include "openhd-profile.hpp"

/**
 * Main entry point for OpenHD video streaming.
 * NOTE: This module only needs to be run on air pi, so to say it is a "Video
 * stream camera wrapper".
 * TODO: We need to feed mavlink commands coming from telemetry into here & send
 * them to the according streams.
 */
class OHDVideo {
 public:
  /**
   * Reads the detected cameras (from ohd_system & json) then creates an
   * according stream for each of them.
   * @param is_air MUST ALWAYS RUN ON AIR ONLY, there are no cameras on the
   * ground pi and the transmission (and optionally forwarding) of the generated
   * video stream(s) is done by ohd_interface.
   * @param unit_id stephen
   * @param platform_type the platform we are running on.
   */
  OHDVideo(const OHDPlatform &platform, const OHDProfile &profile,
           DiscoveredCameraList cameras);
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
  // Experimental for now
  bool set_video_format(int stream_idx, const VideoFormat &video_format);
  std::shared_ptr<CameraStream> get_stream_by_index(int idx);

 public:
 private:
  const OHDPlatform &platform;
  const OHDProfile &profile;

 private:
  // These members are what used to be in camera microservice
  // All the created camera streams
  std::vector<std::shared_ptr<CameraStream>> m_camera_streams;
  // each camera stream already links camera, is this duplicated ??!
  DiscoveredCameraList m_cameras;
  void setup();
  void configure(Camera &camera);
};

#endif  // OPENHD_VIDEO_OHDVIDEO_H
