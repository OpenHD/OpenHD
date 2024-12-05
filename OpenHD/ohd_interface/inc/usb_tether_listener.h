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

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_USBHOTSPOT_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_USBHOTSPOT_H_

#include <openhd_external_device.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <utility>

#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

/**
 * USB hotspot (USB Tethering).
 * Since the USB tethering is always initiated by the user (when he switches USB
 * Tethering on on his phone/tablet) we don't need any settings or similar, and
 * checking once every second barely uses any CPU resources. This was created by
 * translating the tether_functions.sh script from wifibroadcast-scripts into
 * c++. This class configures and forwards the connect and disconnect event(s)
 * for a USB tethering device, such that we can start/stop forwarding to the
 * device's ip address. Only supports one USB tethering device connected at the
 * same time. Also, assumes that the usb tethering device always shows up under
 * /sys/class/net/usb0. Note that we do not have to perform any setup action(s)
 * here - network manager does that for us We really only listen to the event's
 * device connected / device disconnected and forward them.
 */
class USBTetherListener {
 public:
  /**
   * Creates a new USB tether listener which notifies the upper level with the
   * IP address of a connected or disconnected USB tether device.
   * @param external_device_manager connect / disconnect events are forwarded
   * using this handle
   */
  explicit USBTetherListener();
  ~USBTetherListener();

 private:
  std::shared_ptr<spdlog::logger> m_console;
  std::unique_ptr<std::thread> m_check_connection_thread;
  std::atomic<bool> m_check_connection_thread_stop = false;
  /**
   * Continuously checks for connected or disconnected USB tether devices.
   * Does not return as long as there is no fatal error or a stop is requested.
   * Use startLooping() to not block the calling thread.
   */
  void loopInfinite();
  /**
   * @brief simple state-based method that performs the following sequential
   * steps: 1) Wait until a tethering device is connected 2) Get the IP
   * -> if success, forward the IP address of the connected device.
   * 3) Wait until the device disconnects
   * 4) forward the now disconnected IP address.
   * Nr. 3) might never become true during run time as long as the user does not
   * disconnect his tethering device.
   */
  void connectOnce();
};

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_USBHOTSPOT_H_
