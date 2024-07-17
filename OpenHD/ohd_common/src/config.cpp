//
// Created by Raphael on 7/15/24.
//
#include "config.h"
#include "openhd_platform.h"

const char* CONFIG_BASE_PATH;
const char* VIDEO_PATH;

void set_paths() {
  const auto& platform = OHDPlatform::instance();
  switch (platform.platform_type) {
    case X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W:
    case X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_A:
    case X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_B:
    case X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_CM3:
      CONFIG_BASE_PATH = "/config/openhd/";
      VIDEO_PATH = "/home/openhd/Videos/";
      break;
    case X_PLATFORM_TYPE_ALWINNER_X20:
      CONFIG_BASE_PATH = "/external/openhd/";
      VIDEO_PATH = "/home/openhd/Videos/";
      break;
    default:
      CONFIG_BASE_PATH = "/boot/openhd/";
      VIDEO_PATH = "/home/openhd/Videos/";
      break;
  }
}
