//
// Created by consti10 on 13.04.22.
//

#ifndef OPENHD_TELEMETRY_GROUNDTELEMETRY_H
#define OPENHD_TELEMETRY_GROUNDTELEMETRY_H

#include "endpoints/TCPEndpoint.h"
#include "endpoints/UDPEndpoint.h"
#include "endpoints/UDPEndpoint2.h"
#include "endpoints/SerialEndpoint.h"
#include "internal/InternalTelemetry.h"

/**
 * OpenHD Ground telemetry. Assumes a air instance running on the air pi.
 */
class GroundTelemetry {
 public:
  GroundTelemetry();
  /**
   * Telemetry will run infinite in its own threads until an error occurs.
   * @param enableExtendedLogging be really verbose on logging.
   */
  void loopInfinite(bool enableExtendedLogging = false);
  [[nodiscard]] std::string createDebug()const;
 private:
  // called every time a message from the air pi is received
  void onMessageAirPi(MavlinkMessage &message);
  // send a message to the air pi
  void sendMessageAirPi(MavlinkMessage &message);
  // called every time a message is received from any of the clients connected to the Ground Station (For Example QOpenHD)
  void onMessageGroundStationClients(MavlinkMessage &message);
  // send a message to all clients connected to the ground station, for example QOpenHD
  void sendMessageGroundStationClients(MavlinkMessage &message);
 private:
  static constexpr auto M_SYS_ID = OHD_SYS_ID_GROUND;
  std::unique_ptr<TCPEndpoint> tcpGroundCLient = nullptr;
  //std::unique_ptr<UDPEndpoint> udpGroundClient = nullptr;
  std::unique_ptr<UDPEndpoint2> udpGroundClient = nullptr;
  // We rely on another service for starting the rx/tx links
  std::unique_ptr<UDPEndpoint> udpWifibroadcastEndpoint;
  InternalTelemetry ohdTelemetryGenerator{false};
};

#endif //OPENHD_TELEMETRY_GROUNDTELEMETRY_H
