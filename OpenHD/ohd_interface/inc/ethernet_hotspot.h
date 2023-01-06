//
// Created by consti10 on 04.01.23.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HOTSPOT_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HOTSPOT_H_

#include <ISettingsComponent.hpp>
#include <string>

#include "ethernet_hotspot_settings.h"

class EthernetHotspot{
 public:
  explicit EthernetHotspot(std::string  device);
  ~EthernetHotspot();
  std::vector<openhd::Setting> get_all_settings();
 private:
  void start();
  void stop();
  void start_async();
  void stop_async();
  std::shared_ptr<spdlog::logger> m_console;
  const std::string m_device;
  std::unique_ptr<EthernetHotspotSettingsHolder> m_settings;
};

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HOTSPOT_H_
