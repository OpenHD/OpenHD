//
// Created by consti10 on 04.01.23.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_NETWORKING_SETTINGS_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_NETWORKING_SETTINGS_H_

#include <cstdint>

#include "openhd_settings_directories.hpp"
#include "openhd_settings_persistent.h"

// On by default, disabled when the FC is armed,
// re-enabled if the FC is disarmed.
static constexpr auto WIFI_HOTSPOT_AUTO = 0;
static constexpr auto WIFI_HOTSPOT_ALWAYS_OFF = 1;
static constexpr auto WIFI_HOTSPOT_ALWAYS_ON = 2;

// Networking related settings, separate from wb_link
struct NetworkingSettings {
  // Only used if a wifi hotspot card has been found
  int wifi_hotspot_mode = WIFI_HOTSPOT_AUTO;
  // Ethernet hotspot (changes networking,might require reboot)
  bool ethernet_hotspot_enable = false;
  // passive listening for forwarding without hotspot functionality, can be
  // enabled / disabled at run time.
  bool ethernet_nonhotspot_enable_auto_forwarding = false;
};

static bool is_valid_wifi_hotspot_mode(int mode) {
  return mode == 0 || mode == 1 || mode == 2;
}

class NetworkingSettingsHolder
    : public openhd::PersistentSettings<NetworkingSettings> {
 public:
  NetworkingSettingsHolder()
      : openhd::PersistentSettings<NetworkingSettings>(
            openhd::get_interface_settings_directory()) {
    init();
  }

 private:
  [[nodiscard]] std::string get_unique_filename() const override {
    return "networking_settings.json";
  }
  [[nodiscard]] NetworkingSettings create_default() const override {
    return NetworkingSettings{};
  }
  std::optional<NetworkingSettings> impl_deserialize(
      const std::string& file_as_string) const override;
  std::string imp_serialize(const NetworkingSettings& data) const override;
};

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_NETWORKING_SETTINGS_H_
