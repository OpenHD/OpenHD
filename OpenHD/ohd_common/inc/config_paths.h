//
// Created by Raphael on 7/15/24.
// Made to hold several variables that can be used within OpenHD
//
#ifndef CONFIG_H
#define CONFIG_H

// Function declarations
void setConfigBasePath(const char* path);
void setVideoPath(const char* path);
const char* getConfigBasePath();
const char* getVideoPath();

#endif  // CONFIG_H
