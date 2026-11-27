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

#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include "openhd_util_filesystem.h"

// Initial default paths
static char* CONFIG_BASE_PATH = nullptr;
static char* VIDEO_PATH = nullptr;

const char* getConfigBasePath() {
  return CONFIG_BASE_PATH && *CONFIG_BASE_PATH ? CONFIG_BASE_PATH : "/Config/";
}

bool hasMountedVideoPartition() {
  struct stat root {}, video {};
  struct statvfs volume {};
  return stat("/", &root) == 0 && stat("/Video", &video) == 0 &&
         root.st_dev != video.st_dev && statvfs("/Video", &volume) == 0 &&
         (volume.f_flag & ST_RDONLY) == 0;
}

const char* getVideoPath() {
  // The camera worker can inherit an explicit recording path.
  if (VIDEO_PATH && *VIDEO_PATH) return VIDEO_PATH;
  if (hasMountedVideoPartition()) return "/Video/";

  // Legacy removable-storage mount points.
  if (OHDFilesystemUtil::exists("/Videos/external_video_part.txt")) {
    return "/Videos/";
  }
  if (OHDFilesystemUtil::exists("/external/Videos/external_video_part.txt")) {
    return "/external/Videos/";
  }
  if (!OHDFilesystemUtil::exists("/Video/")) {
    OHDFilesystemUtil::create_directories("/Video/");
  }
  return "/Video/";
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
