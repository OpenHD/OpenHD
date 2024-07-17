//
// Created by Raphael on 7/15/24.
//

#include "config.h"

#ifdef X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W
const char* CONFIG_BASE_PATH = "/config/openhd/";
#elif X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_A
const char* CONFIG_BASE_PATH = "/config/openhd/";
#elif X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_B
const char* CONFIG_BASE_PATH = "/config/openhd/";
#elif X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_CM3
const char* CONFIG_BASE_PATH = "/config/openhd/";
#elif X_PLATFORM_TYPE_ALWINNER_X20
const char* CONFIG_BASE_PATH = "/external/openhd/";
#else
const char* CONFIG_BASE_PATH = "/boot/openhd/";
#endif

#ifdef X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W
const char* VIDEO_PATH = "/home/openhd/Videos/";
#else
const char* VIDEO_PATH = "/home/openhd/Videos/";
#endif