//
// Created by Raphael on 7/15/24.
//
// config.cpp

#include "config.h"
#include <cstring>
#include <cstdlib>

// Initial default paths
static char* CONFIG_BASE_PATH = nullptr;
static char* VIDEO_PATH = nullptr;

const char* getConfigBasePath() {
    return CONFIG_BASE_PATH ? CONFIG_BASE_PATH : "/home/openhd/Videos/";
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
