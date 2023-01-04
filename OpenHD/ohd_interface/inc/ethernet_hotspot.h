//
// Created by consti10 on 04.01.23.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HOTSPOT_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HOTSPOT_H_

#include <string>
#include "ethernet_hotspot_settings.h"

class EthernetHotspot{
 public:
  explicit EthernetHotspot(std::string  device);
 private:
  const std::string m_device;
  std::unique_ptr<EthernetHotspotSettingsHolder> m_settings;
};

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HOTSPOT_H_
