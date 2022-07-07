//
// Created by consti10 on 13.04.22.
//

#ifndef OPENHD_TELEMETRY_GROUNDTELEMETRY_H
#define OPENHD_TELEMETRY_GROUNDTELEMETRY_H

#include "endpoints/SerialEndpoint.h"
#include "endpoints/TCPEndpoint.h"
#include "endpoints/UDPEndpoint.h"
#include "endpoints/UDPEndpoint2.h"
#include "internal/OHDMainComponent.h"
#include "mavlink_settings/XSettingsComponent.h"

/**
 * OpenHD Ground telemetry. Assumes a air instance running on the air pi.
 */
class GroundTelemetry :public MavlinkSystem{
 public:
  GroundTelemetry();
  /**
   * Telemetry will run infinite in its own threads until an error occurs.
   * @param enableExtendedLogging be really verbose on logging.
   */
  [[noreturn]] void loopInfinite(bool enableExtendedLogging = false);
  [[nodiscard]] std::string createDebug()const;
  // add a mavlink parameter server that allows the user to change parameters
  void add_settings_component(int comp_id,std::shared_ptr<openhd::XSettingsComponent> glue);
 private:
  // called every time a message from the air pi is received
  void onMessageAirPi(MavlinkMessage &message);
  // send a message to the air pi
  void sendMessageAirPi(const MavlinkMessage &message);
  // called every time a message is received from any of the clients connected to the Ground Station (For Example QOpenHD)
  void onMessageGroundStationClients(MavlinkMessage &message);
  // send a message to all clients connected to the ground station, for example QOpenHD
  void sendMessageGroundStationClients(const MavlinkMessage &message);
 private:
  static constexpr auto M_SYS_ID = OHD_SYS_ID_GROUND;
  std::unique_ptr<TCPEndpoint> tcpGroundCLient = nullptr;
  //std::unique_ptr<UDPEndpoint> udpGroundClient = nullptr;
  std::unique_ptr<UDPEndpoint2> udpGroundClient = nullptr;
  // We rely on another service for starting the rx/tx links
  std::unique_ptr<UDPEndpoint> udpWifibroadcastEndpoint;
  std::shared_ptr<OHDMainComponent> _ohd_main_component;
  std::vector<std::shared_ptr<MavlinkComponent>> components;
};

#endif //OPENHD_TELEMETRY_GROUNDTELEMETRY_H
