//
// Created by consti10 on 04.01.23.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HOTSPOT_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HOTSPOT_H_

#include <openhd_external_device.hpp>
#include <openhd_settings_imp.hpp>
#include <string>
#include <thread>

/**
 * This class exists to expose the following (quite specific, but proven to be popular) functionality of
 * configuring the ground station to act as a DHCP provider (Hotspot) on the ethernet port
 * and then detecting if a device is connected via ethernet - this device then becomes a classic "external device"
 * regarding video and telemetry forwarding.
 * NOTE: Enabling / disabling this feature requires a reboot of the system (this stuff is just way too "dirty" to do it any other way) and we try and avoid
 * touching the networking of the host device from the OpenHD main executable for users that run OpenHD / QOpenHD on their own ubuntu installation.
 */
class EthernetHotspot{
 public:
  explicit EthernetHotspot(std::shared_ptr<openhd::ExternalDeviceManager> external_device_manager,std::string  device);
  ~EthernetHotspot();
  void enable();
  static void cleanup();
 private:
  std::shared_ptr<spdlog::logger> m_console;
  std::shared_ptr<openhd::ExternalDeviceManager> m_external_device_manager;
  const std::string m_device;
  std::unique_ptr<std::thread> m_check_connection_thread;
  std::atomic<bool> m_check_connection_thread_stop =false;
  void loop_infinite();
  // simple pattern: Wait for device to become available
  // forward connected event via ext devices manager
  // then wait for it to disconnect (might never happen) or openhd terminates
  // forward disconnected event via ext devices manager
  void discover_device_once();
};

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HOTSPOT_H_
