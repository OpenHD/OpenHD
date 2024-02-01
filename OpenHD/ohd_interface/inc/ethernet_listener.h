//
// Created by consti10 on 03.01.23.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_LISTENER_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_LISTENER_H_

#include <openhd_external_device.h>

#include <atomic>
#include <memory>
#include <thread>

#include "openhd_spdlog.h"

// Same/Similar pattern as usb_tether_listener.h
// For automatically forwarding data to device(s) connected via Ethernet when
// the Ethernet is NOT a hotspot, but rather waits for someone to provide
// internet / dhcpcd. Waits for someone to give the pi an ip / internet via
// ethernet, and start / stop automatic video and telemetry forwarding. Not
// really recommended - the ethernet hotspot functionality is much more popular
// and easier to implement.
class EthernetListener {
 public:
  explicit EthernetListener(std::string device = "eth0");
  EthernetListener(const EthernetListener&) = delete;
  EthernetListener(const EthernetListener&&) = delete;
  ~EthernetListener();
  void start();
  void stop();
  void set_enabled(bool enable);

 private:
  const std::string m_device;
  std::shared_ptr<spdlog::logger> m_console;
  std::unique_ptr<std::thread> m_check_connection_thread = nullptr;
  std::atomic<bool> m_check_connection_thread_stop = false;
  std::mutex m_enable_disable_mutex;
  void loop_infinite();
  void connect_once();
};

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_LISTENER_H_
