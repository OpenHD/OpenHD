//
// Created by consti10 on 03.05.22.
//

#ifndef OPENHD_VIDEO_OHDVIDEO_H
#define OPENHD_VIDEO_OHDVIDEO_H

#include "camerastream.h"
#include "openhd-platform.hpp"

#include <string>

#include "openhd-spdlog.hpp"

/**
 * Main entry point for OpenHD video streaming for discovered cameras.
 * NOTE: Camera(s) and camera settings are local on the air unit, the ground unit does not need to know anything about that -
 * it just "stupidly" forwards received video data. Therefore, we only create an instance of this class on the air unit.
 * See the Readme.md and camerastream.h for more information.
 */
class OHDVideo {
 public:
  /**
   * Creates a video stream for each of the discovered cameras given in @param cameras. You have to provide at least one camera -
   * if there is no camera found, use a dummy camera.
   * @param opt_action_handler openhd global handler for communication between different ohd modules.
   */
  OHDVideo(OHDPlatform platform1,const std::vector<Camera>& cameras,std::shared_ptr<openhd::ActionHandler> opt_action_handler);
  OHDVideo(const OHDVideo&)=delete;
  OHDVideo(const OHDVideo&&)=delete;
  /**
   * Create a verbose debug string about the current state of OHDVideo.
   * @return a verbose debug string.
   */
  [[nodiscard]] std::string createDebug() const;
  /**
   * This should be called in regular intervals by the OpenHD main thread to
   * restart any camera stream if it has unexpectedly stopped.
   */
  void restartIfStopped();
  /**
   * Get all the settings for the discovered cameras. Settings for Camera0 are the first element, other cameras
   * (if existing) follow.
   */
  std::vector<std::shared_ptr<openhd::ISettingsComponent>> get_setting_components();
 private:
  const OHDPlatform m_platform;
  // All the created camera streams
  std::vector<std::shared_ptr<CameraStream>> m_camera_streams;
  // Add a CameraStream for a discovered camera.
  void configure(std::shared_ptr<CameraHolder> camera);
  std::shared_ptr<spdlog::logger> m_console;
  // r.n limited to primary and secondary camera
  static constexpr auto MAX_N_CAMERAS=2;
  std::shared_ptr<openhd::ActionHandler> m_opt_action_handler;
 private:
  void handle_change_bitrate_request(openhd::ActionHandler::LinkBitrateInformation lb);
};

#endif  // OPENHD_VIDEO_OHDVIDEO_H
