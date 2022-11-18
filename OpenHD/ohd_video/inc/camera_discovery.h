#ifndef CAMERA_H
#define CAMERA_H

#include <array>
#include <chrono>
#include <vector>

#include "camera_holder.hpp"
#include "openhd-platform.hpp"

#include "openhd-spdlog.hpp"

/**
 * Discover all connected cameras and expose their hardware capabilities to OpenHD.
 * Note that this class does not handle camera settings (like video width, height) - camera capabilities
 * and user set / default camera settings are seperated.
 */
class DCameras {
 public:
  explicit DCameras(OHDPlatform ohdPlatform);
  virtual ~DCameras() = default;
  /**
   * Discover all cameras connected to this system.
   * @returns A list of detected cameras, or an empty vector if no cameras have been found.
   * Note that at this point, we haven't performed the settings lookup for the Camera(s) - this just exposes the available cameras
   * and their capabilities.
   * @param ohdPlatform the platform we are running on, detection depends on the platform type.
   */
  static std::vector<Camera> discover(OHDPlatform ohdPlatform);
 private:
  DiscoveredCameraList discover_internal();
  void argh_cleanup();
 private:
  /*
   * These are for platform-specific camera access methods, most can also be
   * accessed through v4l2 but there are sometimes downsides to doing that. For
   * example on the Pi, v4l2 can have higher latency than going through the
   * broadcom API, and at preset bcm2835-v4l2 doesn't support all of the ISP
   * controls.
   *
   */
  /**
   * This is used when the gpu firmware is in charge of the camera, we have to
   * ask it.
   */
  void detect_raspberrypi_broadcom_csi();
  // hacky
  bool detect_raspberrypi_broadcom_veye();

  /*
   * Detecting via libcamera.
   * Actually all cameras in system available via libcamera.
   * Moreover libcamera cameras is v4l devices and can be used as usual.
   * But here we are using libcamera only for undetected cameras for compatability
   */
  void detect_raspberry_libcamera();

  /*
   * Detect all v4l2 cameras, that is cameras that show up as a v4l2 device
   * (/dev/videoXX)
   */
  void detect_v4l2();
  /**
   * Something something stephen.
   */
  void probe_v4l2_device(const std::string &device_node);
  /**
   * Something something stephen.
   */
  bool process_v4l2_node(const std::string &node, Camera &camera,
                         CameraEndpoint &endpoint);
  /**
   * TODO unimplemented.
   */
  void detect_ip();

  std::vector<Camera> m_cameras;
  std::vector<CameraEndpoint> m_camera_endpoints;

  int m_discover_index = 0;

  const OHDPlatform ohdPlatform;
  bool m_enable_debug;

 private:
  std::shared_ptr<spdlog::logger> m_console;
};

#endif
