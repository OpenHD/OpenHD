//
// Created by consti10 on 13.04.22.
//

#ifndef OPENHD_TELEMETRY_AIRTELEMETRY_H
#define OPENHD_TELEMETRY_AIRTELEMETRY_H

#include <string>

#include "endpoints/SerialEndpoint.h"
#include "endpoints/UDPEndpoint.h"
#include "internal/OHDMainComponent.h"
#include "mavlink_settings/XSettingsComponent.h"
#include "openhd-platform.hpp"
#include "routing/MavlinkSystem.hpp"

/**
 * OpenHD Air telemetry. Assumes a Ground instance running on the ground pi.
 */
class AirTelemetry : public MavlinkSystem{
 public:
  explicit AirTelemetry(std::string fcSerialPort);
  /**
   * Telemetry will run infinite in its own threads until an error occurs.
   * @param enableExtendedLogging be really verbose on logging.
   */
  [[noreturn]] void loopInfinite(bool enableExtendedLogging = false);
  [[nodiscard]] std::string createDebug()const;
  // add a mavlink parameter server that allows the user to change parameters
  void add_settings_component(int comp_id,std::shared_ptr<openhd::XSettingsComponent> glue);
 private:
  // send a mavlink message to the flight controller connected to the air unit via UART, if connected.
  void sendMessageFC(const MavlinkMessage &message);
  // send a mavlink message to the ground pi, system cannot know if this message actually makes it.
  void sendMessageGroundPi(const MavlinkMessage &message);
  // called every time a message from the flight controller is received
  void onMessageFC(MavlinkMessage &message);
  // called every time a message from the ground pi is received
  void onMessageGroundPi(MavlinkMessage &message);
 private:
  std::unique_ptr<SerialEndpoint> serialEndpoint;
  // For now, use UDP endpoint and rely on another service for starting the rx/tx links
  //std::unique_ptr<WBEndpoint> wifibroadcastEndpoint;
  std::unique_ptr<UDPEndpoint> wifibroadcastEndpoint;
  std::shared_ptr<OHDMainComponent> _ohd_main_component;
  std::vector<std::shared_ptr<MavlinkComponent>> components;
};

#endif //OPENHD_TELEMETRY_AIRTELEMETRY_H
