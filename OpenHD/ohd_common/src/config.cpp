//
// Created by Raphael on 7/15/24.
//
// config.cpp

#include "config.h"
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <iostream>
#include <memory>
#include "openhd_platform.h"


// Initial default paths
static char* CONFIG_BASE_PATH = nullptr;
static char* VIDEO_PATH = nullptr;

const char* getConfigBasePath() {
    const auto platform_debug = OHDPlatform::instance();
    if (platform.is_rock()) {
            std::cerr << "Setting paths for rock platform: " << platform_debug.to_string() << std::endl;
        return "/config/openhd/";
    } else if (platform.is_radxa_cm3()) {
        std::cerr << "Setting paths for rock cm3 platform: " << platform.to_string() << std::endl;
        return "/config/openhd/";
    } else {
            std::cerr << "Setting paths for normal platform: " << platform_debug.to_string() << std::endl;
        return "/boot/openhd/";
    }
}

const char* getVideoPath() {
    return VIDEO_PATH ? VIDEO_PATH : "/home/openhd/Videos/";
}

void setConfigBasePath(const char* path) {
    if (CONFIG_BASE_PATH) {
        free(CONFIG_BASE_PATH);
    }
    CONFIG_BASE_PATH = static_cast<char*>(malloc(strlen(path) + 1));
    strcpy(CONFIG_BASE_PATH, path);
}

void setVideoPath(const char* path) {
    if (VIDEO_PATH) {
        free(VIDEO_PATH);
    }
    VIDEO_PATH = static_cast<char*>(malloc(strlen(path) + 1));
    strcpy(VIDEO_PATH, path);
}
