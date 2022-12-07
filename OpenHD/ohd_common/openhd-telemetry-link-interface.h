//
// Created by consti10 on 07.12.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_TELEMETRY_LINK_INTERFACE_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_TELEMETRY_LINK_INTERFACE_H_

#include <vector>
#include <memory>

namespace openhd{

// Telemetry communication between air and ground is lossy, but bidirectional
// e.g both air and ground send and receive data (over wifibroadcast or perhaps something else in the future)
// Re-transmissions are done by mavlink, not the link itself
class ITransmitReceiveTelemetry{
  // Telemetry, bidirectional (receive and transmit each)
  // valid on both air and ground instance
  // send telemetry data to the ground if air unit and vice versa.
  virtual bool transmit_telemetry_data(std::shared_ptr<std::vector<uint8_t>> data)=0;

  // valid on both air and ground instance
  // called every time telemetry data is received
  virtual void on_receive_telemetry_data(std::shared_ptr<std::vector<uint8_t>> data)=0;
};
}
#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_TELEMETRY_LINK_INTERFACE_H_
