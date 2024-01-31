#ifndef CAMERA_H
#define CAMERA_H

#include <array>
#include <chrono>
#include <vector>

#include "camera_holder.h"
#include "openhd_platform.h"
#include "openhd_spdlog.h"

/**
 * We used to try and also discover CSI cameras, but with the current state of
 * the main platform (RPI) this is just not feasible / bugged. Therefore, only
 * discover USB cameras, for the rest, be lazy and rely on the user setting the
 * camera.
 */
class DCameras {
 public:
  struct DiscoveredUSBCamera {
    std::string bus;
    std::string device_name;
  };
  static std::vector<DiscoveredUSBCamera> detect_usb_cameras(
      const OHDPlatform& platform, std::shared_ptr<spdlog::logger>& m_console);

  // NOTE: IP cameras cannot be auto detected !
};

#endif
