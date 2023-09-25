//
// Created by consti10 on 03.05.22.
//

#ifndef OPENHD_VIDEO_OHDVIDEO_H
#define OPENHD_VIDEO_OHDVIDEO_H

#include <string>

#include "camerastream.h"
#include "ohd_video_air_generic_settings.h"
#include "openhd_platform.h"
#include "openhd_spdlog.h"
// Dirty
#include "openhd-rpi-os-configure-vendor-cam.hpp"
#include "openhd_external_device.hpp"
#include "../../lib/wifibroadcast/src/HelperSources/SocketHelper.hpp"
#include "openhd_link.hpp"

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
  ~OHDVideoAir();
  OHDVideoAir(const OHDVideoAir&)=delete;
  OHDVideoAir(const OHDVideoAir&&)=delete;
  /**
   * Discover connected cameras at run time. CSI and USB cameras generally can be auto-detected,
   * IP Cameras (and/or really "specific" setups) not. Automatic discovery can be completely overridden by editing the .config file.
   * @return the camera(s) openhd should use, guaranteed to be at least one (in case no cam is found,
   * a dummy sw camera is created)
   */
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
  // react to dynamically connecting / disconnecting external device(s)
  void set_ext_devices_manager(std::shared_ptr<openhd::ExternalDeviceManager> ext_device_manager);
  void update_arming_state(bool armed);
 private:
  const OHDPlatform m_platform;
  // All the created camera streams
  std::vector<std::shared_ptr<CameraStream>> m_camera_streams;
  std::shared_ptr<spdlog::logger> m_console;
  std::shared_ptr<openhd::ActionHandler> m_opt_action_handler;
  std::shared_ptr<OHDLink> m_link_handle;
  // r.n only for multi camera support
  std::unique_ptr<AirCameraGenericSettingsHolder> m_generic_settings;
  // dirty / annoying. Interacts heavily with the OS aka can break QOpenHD
  std::unique_ptr<openhd::rpi::os::ConfigChangeHandler> m_rpi_os_change_config_handler=nullptr;
 private:
  // Add a CameraStream for a discovered camera.
  void configure(const std::shared_ptr<CameraHolder>& camera);
  // propagate a bitrate change request to the CameraStream implementation(s)
  void handle_change_bitrate_request(openhd::ActionHandler::LinkBitrateInformation lb);
  // Called every time an encoded frame was generated
  void on_video_data(int stream_index,const openhd::FragmentedVideoFrame& fragmented_video_frame);
  // NOTE: On air, by default, we do not forward video via UDP to save precious cpu time -
  // but we allow user(s) to connect to the air unit via mavlink TCP directly,
  // in which case we start forwarding of video data to the device.
  void start_stop_forwarding_external_device(openhd::ExternalDevice external_device,bool connected);
  std::unique_ptr<SocketHelper::UDPMultiForwarder> m_primary_video_forwarder= nullptr;
  std::unique_ptr<SocketHelper::UDPMultiForwarder> m_secondary_video_forwarder= nullptr;
  // Optimization for 0 overhead on air when not enabled
  std::atomic_bool m_has_localhost_forwarding_enabled=false;
};

#endif  // OPENHD_VIDEO_OHDVIDEO_H
