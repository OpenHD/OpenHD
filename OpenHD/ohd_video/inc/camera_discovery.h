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

#ifndef CAMERA_H
#define CAMERA_H

#include <array>
#include <chrono>
#include <vector>

#include "camera_holder.h"
#include "openhd_platform.h"
#include "openhd_spdlog.h"

/**
 * We used to try and also discover CSI cameras, but with the current state of
 * the main platform (RPI) this is just not feasible / bugged. Therefore, only
 * discover USB cameras, for the rest, be lazy and rely on the user setting the
 * camera.
 */
class DCameras {
 public:
  struct DiscoveredUSBCamera {
    std::string bus;
    int v4l2_device_number;
  };
  static std::vector<DiscoveredUSBCamera> detect_usb_cameras(
      std::shared_ptr<spdlog::logger>& m_console, bool debug = false);

  // NOTE: IP cameras cannot be auto detected !
};

#endif
