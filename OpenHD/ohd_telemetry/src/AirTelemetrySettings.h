//
// Created by consti10 on 31.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_AIRTELEMETRYSETTINGS_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_AIRTELEMETRYSETTINGS_H_

// Settings for telemetry, only valid on an air pi (since only on the air pi we connect the FC)

#include "openhd-settings.hpp"
#include "openhd-settings2.hpp"

namespace openhd{

static const std::string TELEMETRY_SETTINGS_DIRECTORY=std::string(BASE_PATH)+std::string("telemetry/");

struct AirTelemetrySettings{
  // 0: RPI UART0 (/dev/serial0)
  // 1: RPI UART1 (/dev/serial1)
  // 2: UART USB ADAPTER (/dev/ttyUSB0)
  //  My ardupilot shows up as either /dev/ttyACM0 or /dev/ttyACM1 if I connect it via
  // usb cable (micro usb on FC to rpi USB port)
  // 3: /dev/ttyACM0
  // 4: /dev/ttyACM1
  int uart_connection_type=0;
  int uart_baudrate=0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AirTelemetrySettings,uart_connection_type,uart_baudrate);

class AirTelemetrySettingsHolder:public openhd::settings::PersistentSettings<AirTelemetrySettings>{
 public:
  AirTelemetrySettingsHolder():
	  openhd::settings::PersistentSettings<AirTelemetrySettings>(TELEMETRY_SETTINGS_DIRECTORY){
	init();
  }
 private:
  [[nodiscard]] std::string get_unique_filename()const override{
	std::stringstream ss;
	ss<<"fc_uart.json";
	return ss.str();
  }
  [[nodiscard]] AirTelemetrySettings create_default()const override{
	return AirTelemetrySettings{};
  }
};

}

#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_AIRTELEMETRYSETTINGS_H_
