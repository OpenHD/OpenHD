//
// Created by consti10 on 02.05.22.
//

#ifndef OPENHD_OPENHD_INTERFACE_H
#define OPENHD_OPENHD_INTERFACE_H

#include <memory>
#include <utility>

#include "ethernet_hotspot.h"
#include "ethernet_listener.h"
#include "openhd_action_handler.hpp"
#include "openhd_external_device.hpp"
#include "openhd_led_codes.hpp"
#include "openhd_platform.h"
#include "openhd_profile.h"
#include "openhd_settings_imp.hpp"
#include "openhd_spdlog.h"
#include "usb_tether_listener.h"
#include "wb_link.h"
#include "wifi_hotspot.h"

/**
 * Takes care of everything networking related, like wifibroadcast, usb / tethering / WiFi-hotspot usw.
 * In openhd, there is an instance of this class on both air and ground with partially similar, partially
 * different functionalities.
 */
class OHDInterface :public openhd::ISettingsComponent{
 public:
  /**
   * @param platform platform we are running on
   * @param profile air or ground
   * @param opt_action_handler r.n used to propagate rate control from wb_link to ohd_video
   */
  explicit OHDInterface(OHDPlatform platform,OHDProfile profile,std::shared_ptr<openhd::ActionHandler> opt_action_handler=nullptr,
                        bool continue_without_wb_card=false);
  OHDInterface(const OHDInterface&)=delete;
  OHDInterface(const OHDInterface&&)=delete;
  // Verbose string about the current state.
  [[nodiscard]] std::string createDebug() const;
  // Get all (mavlink) settings ohd_interface exposes on the air or ground unit, respective
  std::vector<openhd::Setting> get_all_settings()override;
  // easy access without polluting the headers
  static void print_internal_fec_optimization_method();
  // Agnostic of the link, even though r.n we only have a wifibroadcast implementation (but this might change).
  std::shared_ptr<OHDLink> get_link_handle();
  // Both video and telemetry do the forwarding in their own way, this just provides the convenient methods to
  // start / stop forwarding if external device(s) are connected / disconnected.
  std::shared_ptr<openhd::ExternalDeviceManager> get_ext_devices_manager();
 private:
  const OHDProfile m_profile;
  const OHDPlatform m_platform;
  std::shared_ptr<spdlog::logger> m_console;
  std::shared_ptr<WBLink> m_wb_link;
  std::unique_ptr<USBTetherListener> m_usb_tether_listener;
  std::unique_ptr<EthernetListener> m_ethernet_listener;
  std::unique_ptr<EthernetHotspot> m_ethernet_hotspot;
  std::unique_ptr<WifiHotspot> m_wifi_hotspot;
  std::unique_ptr<openhd::LEDBlinker> m_error_blinker;
  std::shared_ptr<openhd::ExternalDeviceManager> m_external_devices_manager;
};

#endif //OPENHD_OPENHD_INTERFACE_H
