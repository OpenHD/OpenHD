/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#include "config_paths.h"

#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>

#include "openhd_platform.h"
#include "openhd_util_filesystem.h"

// Initial default paths
static char* CONFIG_BASE_PATH = nullptr;
static char* VIDEO_PATH = nullptr;

const char* getConfigBasePath() {
  const auto platform_debug = OHDPlatform::instance();
  if (platform_debug.is_rock()) {
    return "/config/openhd/";
  } else if (platform_debug.is_x20()) {
    return "/config/openhd/";
  } else {
    return "/boot/openhd/";
  }
}

const char* getVideoPath() {
  static std::string cachedPath;
  static const char* CACHE_FILE = "/usr/local/share/openhd/recording.txt";
  static const char* FILENAME1 = "/Videos/external_video_part.txt";
  static const char* FILENAME2 = "/external/Videos/external_video_part.txt";

  if (!cachedPath.empty()) {
    return cachedPath.c_str();
  }

  if (OHDFilesystemUtil::exists(CACHE_FILE)) {
    std::ifstream infile(CACHE_FILE);
    if (infile) {
      std::getline(infile, cachedPath);
      return cachedPath.c_str();
    }
  }

  const char* selectedPath = nullptr;

  if (OHDFilesystemUtil::exists(FILENAME1)) {
    selectedPath = VIDEO_PATH ? VIDEO_PATH : "/Videos/";
    OHDFilesystemUtil::remove_if_existing(FILENAME1);
  } else if (OHDFilesystemUtil::exists(FILENAME2)) {
    selectedPath = VIDEO_PATH ? VIDEO_PATH : "/external/Videos/";
    OHDFilesystemUtil::remove_if_existing(FILENAME2);
  } else {
    selectedPath = "/home/openhd/Videos/";
    OHDFilesystemUtil::create_directories(selectedPath);
  }

  cachedPath = selectedPath;
  OHDFilesystemUtil::create_directories("/usr/local/share/openhd/");
  std::ofstream outfile(CACHE_FILE);
  if (outfile) {
    outfile << cachedPath;
  }

  return cachedPath.c_str();
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
