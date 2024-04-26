//
// Created by consti10 on 07.11.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_GROUNDTELEMETRYSETTINGS_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_GROUNDTELEMETRYSETTINGS_H_

#include <map>

#include "openhd_settings_directories.h"
#include "openhd_settings_persistent.h"

namespace openhd::telemetry::ground {

// We use an empty string for "serial disabled"
static constexpr auto UART_CONNECTION_TYPE_DISABLE = "";

struct Settings {
  bool enable_rc_over_joystick = false;
  int rc_over_joystick_update_rate_hz = 30;
  std::string rc_channel_mapping =
      "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18";
  // This is for outputting FC mavlink data via serial on the ground station
  std::string gnd_uart_connection_type = UART_CONNECTION_TYPE_DISABLE;
  int gnd_uart_baudrate = 115200;
};

static bool valid_joystick_update_rate(int value) {
  return value >= 1 && value <= 150;
}

class SettingsHolder : public openhd::PersistentSettings<Settings> {
 public:
  SettingsHolder()
      : openhd::PersistentSettings<Settings>(
            openhd::get_telemetry_settings_directory()) {
    init();
  }

 private:
  [[nodiscard]] std::string get_unique_filename() const override {
    return "ground_settings.json";
  }
  [[nodiscard]] Settings create_default() const override { return Settings{}; }
  std::optional<Settings> impl_deserialize(
      const std::string& file_as_string) const override;
  std::string imp_serialize(const Settings& data) const override;
};

}  // namespace openhd::telemetry::ground

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_GROUNDTELEMETRYSETTINGS_H_
