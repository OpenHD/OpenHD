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

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_NETWORKING_SETTINGS_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_NETWORKING_SETTINGS_H_

#include <cstdint>

#include "openhd_settings_directories.h"
#include "openhd_settings_persistent.h"

// On by default, disabled when the FC is armed,
// re-enabled if the FC is disarmed.
static constexpr auto WIFI_HOTSPOT_AUTO = 0;
static constexpr auto WIFI_HOTSPOT_ALWAYS_OFF = 1;
static constexpr auto WIFI_HOTSPOT_ALWAYS_ON = 2;

// OpenHD does not touch the ethernet
static constexpr auto ETHERNET_OPERATING_MODE_UNTOUCHED = 0;
// OpenHD configures the ethernet, such that it acts as a 'hotspot'
// In hotspot mode, the IP of the ground station is always fixed and a unlimited
// amount of devices can connect to it.
static constexpr auto ETHERNET_OPERATING_MODE_HOTSPOT = 1;
// OpenHD does not touch the ethernet, but it starts forwarding data to whoever
// provides internet. a bit complicated :/
static constexpr auto ETHERNET_OPERATING_MODE_EXTERNAL_DEVICE = 2;

// Networking related settings, separate from wb_link
struct NetworkingSettings {
  // Only used if a wifi hotspot card has been found
  int wifi_hotspot_mode = WIFI_HOTSPOT_AUTO;
  // Ethernet operating mode (changes networking,might require reboot)
  int ethernet_operating_mode = ETHERNET_OPERATING_MODE_UNTOUCHED;
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
