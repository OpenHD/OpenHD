//
// Created by Raphael on 7/15/24.
//
#include "config.h"


#ifdef X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W
const char* CONFIG_BASE_PATH = "/config/openhd/";
#else
const char* CONFIG_BASE_PATH = "/boot/openhd/";
#endif

#ifdef X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W
const char* VIDEO_PATH = "/home/openhd/Videos/";
#else
const char* VIDEO_PATH = "/home/openhd/Videos/";
#endif