#ifndef PLATFORM_H
#define PLATFORM_H

#include <array>
#include <chrono>

#include "openhd-platform.hpp"
#include "openhd-discoverable.hpp"

/**
 * Discover the platform we are running on and write it to json.
 * Note: One should not use a instance of this class for anything else than discovery, to pass around the discovered
 * data use the struct from ohd_platform.
 */
class DPlatform {
 public:
  DPlatform() = default;
  virtual ~DPlatform() = default;
  std::shared_ptr<OHDPlatform> discover();
 private:
  void detect_raspberrypi();
  void detect_jetson();
  void detect_pc();
  PlatformType m_platform_type = PlatformTypeUnknown;
  BoardType m_board_type = BoardTypeUnknown;
};

#endif
