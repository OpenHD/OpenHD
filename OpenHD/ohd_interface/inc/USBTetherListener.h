//
// Created by consti10 on 19.05.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_USBHOTSPOT_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_USBHOTSPOT_H_

#include "openhd-util-filesystem.hpp"
#include "openhd-util.hpp"
#include <thread>
#include <chrono>
#include <utility>
#include <mutex>
#include <atomic>


/**
 * USB hotspot (USB Tethering).
 * Since the USB tethering is always initiated by the user (when he switches USB Tethering on on his phone/tablet)
 * we don't need any settings or similar, and checking once every second barely uses any CPU resources.
 * This was created by translating the tether_functions.sh script from wifibroadcast-scripts into c++.
 * This class configures and forwards the connect and disconnect event(s) for a USB tethering device, such that
 * we can start/stop forwarding to the device's ip address.
 * Only supports one USB tethering device connected at the same time.
 * Also, assumes that the usb tethering device always shows up under /sys/class/net/usb0.
 */
class USBTetherListener{
 public:
  /**
   * Callback to be called when a new device has been connected/disconnected.
   * @param removed: true if the device has been removed (upper level should stop formwarding)
   * false if the device has been added (upper leved should start forwarding).
   */
  typedef std::function<void(bool removed,std::string ip)> IP_CALLBACK;
  /**
   * Creates a new USB tether listener which notifies the upper level with the IP address of a connected or
   * disconnected USB tether device.
   * @param ip_callback the callback to notify the upper level.
   */
  explicit USBTetherListener(IP_CALLBACK ip_callback):ip_callback(std::move(ip_callback)){}
  /**
   * Continuously checks for connected or disconnected USB tether devices.
   * Does not return as long as there is no fatal error or a stop is requested.
   * Use startLooping() to not block the calling thread.
   */
  void loopInfinite();
  /**
   * start looping in a new thread.
   */
  void startLooping();
  /**
   * stop looping.
   */
   void stopLooping();
  /**
   * Can be called safely from any thread.
   * @return the valid ip address of the connected USB tether device if there is one. Empty if there is currently no device deteced.
   */
  [[nodiscard]] std::vector<std::string> getConnectedTetherIPsLocked();
 private:
  const IP_CALLBACK ip_callback;
  // protects against simultaneous read/write of the device_ip variable.
  std::mutex device_ip_mutex;
  std::string device_ip;
  std::unique_ptr<std::thread> loopThread;
  std::atomic<bool> loopThreadStop=false;
  // write the device ip, protected by mutex.
  void setDeviceIpLocked(std::string newDeviceIp);
  /**
   * @brief simple state-based method that performs the following sequential steps:
   * 1) Wait until a tethering device is connected
   * 2) Configure the connected device
   * -> if success, forward the IP address of the connected device.
   * 3) Wait until the device disconnects
   * 4) forward the now disconnected IP address.
   * Nr. 3) might never become true during run time as long as the user does not disconnect his tethering device.
   */
  void connectOnce();
};

#endif //OPENHD_OPENHD_OHD_INTERFACE_INC_USBHOTSPOT_H_
