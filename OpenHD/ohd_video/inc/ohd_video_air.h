//
// Created by consti10 on 03.05.22.
//

#ifndef OPENHD_VIDEO_OHDVIDEO_H
#define OPENHD_VIDEO_OHDVIDEO_H

#include <string>

#include "camerastream.h"
#include "ohd_video_air_generic_settings.hpp"
#include "openhd_platform.h"
#include "openhd_spdlog.h"
// Dirty
#include "openhd-rpi-os-configure-vendor-cam.hpp"

/**
 * Main entry point for OpenHD video streaming for discovered cameras on the air unit.
 * NOTE: Camera(s) and camera settings are local on the air unit, the ground unit does not need to know anything about that -
 * it just "stupidly" forwards received video data. Therefore, we only create an instance of this class on the air unit.
 * See the Readme.md and camerastream.h for more information.
 */
class OHDVideoAir {
 public:
  /**
   * Creates a video stream for each of the discovered cameras given in @param cameras. You have to provide at least one camera -
   * if there is no camera found, use a dummy camera.
   * @param opt_action_handler openhd global handler for communication between different ohd modules.
   * @param link_handle handle for sending video data over the (currently only wb) link between air and ground
   */
  OHDVideoAir(OHDPlatform platform1,std::vector<Camera> cameras,
           std::shared_ptr<openhd::ActionHandler> opt_action_handler,
           std::shared_ptr<OHDLink> link_handle);
  OHDVideoAir(const OHDVideoAir&)=delete;
  OHDVideoAir(const OHDVideoAir&&)=delete;
  static std::vector<Camera> discover_cameras(const OHDPlatform& platform);
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
   * In ohd-telemetry, we create a mavlink settings component for each of the camera(s),instead of using one generic settings component
   * like for the rest of the settings.
   * Get all the settings for the discovered cameras. Settings for Camera0 are the first element, other cameras
   * (if existing) follow.
   */
  std::vector<std::shared_ptr<openhd::ISettingsComponent>> get_all_camera_settings();
  // r.n only for debugging
  std::vector<openhd::Setting> get_generic_settings();
  // r.n limited to primary and secondary camera
  static constexpr auto MAX_N_CAMERAS=2;
 private:
  const OHDPlatform m_platform;
  // All the created camera streams
  std::vector<std::shared_ptr<CameraStream>> m_camera_streams;
  std::shared_ptr<spdlog::logger> m_console;
  std::shared_ptr<openhd::ActionHandler> m_opt_action_handler;
  std::shared_ptr<OHDLink> m_link_handle;
 private:
  // Add a CameraStream for a discovered camera.
  void configure(const std::shared_ptr<CameraHolder>& camera);
  // propagate a bitrate change request to the CameraStream implementation(s)
  void handle_change_bitrate_request(openhd::ActionHandler::LinkBitrateInformation lb);
 private:
  // r.n only for multi camera support
  std::unique_ptr<AirCameraGenericSettingsHolder> m_generic_settings;
  // dirty / annoying. Interacts heavily with the OS aka can break QOpenHD
  std::unique_ptr<openhd::rpi::os::ConfigChangeHandler> m_rpi_os_change_config_handler=nullptr;
};

#endif  // OPENHD_VIDEO_OHDVIDEO_H
