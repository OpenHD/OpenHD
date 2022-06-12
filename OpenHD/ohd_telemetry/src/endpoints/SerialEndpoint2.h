//
// Created by consti10 on 12.06.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_SERIALENDPOINT2_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_SERIALENDPOINT2_H_

#include <utility>
#include <chrono>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>

#include "MEndpoint.hpp"

/**
 *  Similar to serial endpoint, but uses mavsdk instead of boost.
 */
class SerialEndpoint2 : public MEndpoint {
 public:
  struct HWOptions {
	std::string linux_filename; // the linux file name,for example /dev/tty..
	int baud_rate = 0; // manual baud rate, set to 0 to leave untouched
  };
 public:
  /**
   * @param serial_port the serial port linux name (dev/.. ) for this serial port
   */
  explicit SerialEndpoint2(std::string TAG, HWOptions options,bool enableDebug=false);
  void sendMessage(const MavlinkMessage &message) override;
 private:
  const HWOptions m_options;
  const bool m_enable_debug;
 public:
  std::shared_ptr<mavsdk::Mavsdk> mavsdk=nullptr;
  // the fc system once discovered
  std::shared_ptr<mavsdk::System> systemFc=nullptr;
  // util to send and receive ALL messages to/from, once discovered
  std::shared_ptr<mavsdk::MavlinkPassthrough> passtrougFC=nullptr;
};

#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_SERIALENDPOINT2_H_
