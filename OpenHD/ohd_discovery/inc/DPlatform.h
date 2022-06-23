#ifndef PLATFORM_H
#define PLATFORM_H

#include <array>
#include <chrono>

#include "openhd-platform.hpp"
#include "openhd-discoverable.hpp"

/**
 * Discover the platform we are running on.
 * Note: One should not use a instance of this class for anything else than discovery, to pass around the discovered
 * data use the struct from ohd_platform.
 */
class DPlatform {
 public:
  DPlatform() = default;
  virtual ~DPlatform() = default;
  static std::shared_ptr<OHDPlatform> discover();
 private:
  static std::optional<std::pair<PlatformType,BoardType>> detect_raspberrypi();
  static std::optional<std::pair<PlatformType,BoardType>> detect_jetson();
  static std::pair<PlatformType,BoardType> detect_pc();
};

#endif
