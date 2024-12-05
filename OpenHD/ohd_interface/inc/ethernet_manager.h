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

#ifndef OPENHD_ETHERNET_MANAGER_H
#define OPENHD_ETHERNET_MANAGER_H

#include <string>

#include "openhd_spdlog_include.h"

//
// See networking_settings for more info
//
class EthernetManager {
 public:
  explicit EthernetManager();

  void async_initialize(int operating_mode);

  void stop();

 private:
  void loop(int operating_mode);
  void configure(int operating_mode, const std::string& device_name);

  std::shared_ptr<spdlog::logger> m_console;
  std::shared_ptr<std::thread> m_thread;
  std::atomic_bool m_terminate = false;

 private:
  // Same/Similar pattern as usb_tether_listener.h
  // For automatically forwarding data to device(s) connected via Ethernet when
  // the Ethernet is NOT a hotspot, but rather waits for someone to provide
  // internet / dhcpcd. Waits for someone to give the pi an ip / internet via
  // ethernet, and start / stop automatic video and telemetry forwarding. Not
  // really recommended - the ethernet hotspot functionality is much more
  // popular and easier to implement.
  void loop_ethernet_external_device_listener(const std::string& device_name);
};

#endif  // OPENHD_ETHERNET_MANAGER_H
