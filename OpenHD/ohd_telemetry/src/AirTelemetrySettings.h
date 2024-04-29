//
// Created by consti10 on 31.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_AIRTELEMETRYSETTINGS_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_AIRTELEMETRYSETTINGS_H_

#include <map>

#include "openhd_platform.h"
#include "openhd_settings_directories.h"
#include "openhd_settings_persistent.h"

// Settings for telemetry, only valid on an air pi (since only on the air pi we
// connect the FC) Note that we do not have any telemetry settings r.n for the
// ground (since forwarding is a task ov ohd_interface, not ohd_telemetry)
namespace openhd::telemetry::air {

// Default for ardupilot and more
static constexpr int DEFAULT_UART_BAUDRATE = 115200;
// We use an empty string for "serial disabled"
static constexpr auto UART_CONNECTION_TYPE_DISABLE = "";

static constexpr auto DEFAULT_UART_CONNECTION =
    UART_CONNECTION_TYPE_DISABLE;  // Default to UART disabled (FC)

struct Settings {
  // Mapped in QOpenHD - "" means disabled
  // In general, we do not really validate the user input - if the fd path
  // doesn't exist for example, the Serial endpoint implementation will cry a
  // warning in regular intervals anyways. 1: RPI UART0 (/dev/serial0) 2: RPI
  // UART1 (/dev/serial1) 3: UART USB ADAPTER (/dev/ttyUSB0)
  //  My ardupilot shows up as either /dev/ttyACM0 or /dev/ttyACM1 if I connect
  //  it via
  // usb cable (micro usb on FC to rpi USB port)
  // 4: /dev/ttyACM0
  // 5: /dev/ttyACM1
  // 6: Rock5B UART7_M2 (/dev/ttyS7)
  std::string fc_uart_connection_type = DEFAULT_UART_CONNECTION;
  int fc_uart_baudrate = DEFAULT_UART_BAUDRATE;
  bool fc_uart_flow_control = false;
  // DANG ardupilot why do we have to make this an extra param ...
  // 0 means not configured (do not use)
  int fc_battery_n_cells = 0;
};

// 16 chars limit !
static constexpr auto FC_UART_CONNECTION_TYPE = "FC_UART_CONN";
static constexpr auto FC_UART_BAUD_RATE = "FC_UART_BAUD";
static constexpr auto FC_UART_FLOW_CONTROL = "FC_UART_FLWCTL";
static constexpr auto FC_BATT_N_CELLS = "FC_BATT_N_CELLS";

class SettingsHolder : public openhd::PersistentSettings<Settings> {
 public:
  explicit SettingsHolder()
      : openhd::PersistentSettings<Settings>(
            openhd::get_telemetry_settings_directory()) {
    init();
  }

 private:
  [[nodiscard]] std::string get_unique_filename() const override {
    return "air_settings.json";
  }
  [[nodiscard]] Settings create_default() const override {
    Settings ret{};
    // Default telemetry serial (for this platform)
    ret.fc_uart_connection_type = "DEFAULT";
    return ret;
  }
  std::optional<Settings> impl_deserialize(
      const std::string& file_as_string) const override;
  std::string imp_serialize(const Settings& data) const override;
};

}  // namespace openhd::telemetry::air

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_AIRTELEMETRYSETTINGS_H_
