//
// Created by consti10 on 13.04.22.
//

#ifndef OPENHD_TELEMETRY_GROUNDTELEMETRY_H
#define OPENHD_TELEMETRY_GROUNDTELEMETRY_H

#include "endpoints/UDPEndpoint.h"
#include "endpoints/UDPEndpoint2.h"
#include "internal/OHDMainComponent.h"
#include "mavlink_settings/ISettingsComponent.hpp"
#include "mavsdk_temporary//XMavlinkParamProvider.h"
#include "openhd-action-handler.hpp"
//#include "rc/JoystickReader.h"

/**
 * OpenHD Ground telemetry. Assumes a air instance running on the air pi.
 */
class GroundTelemetry :public MavlinkSystem{
 public:
  explicit GroundTelemetry(OHDPlatform platform,std::shared_ptr<openhd::ActionHandler> opt_action_handler=nullptr);
  GroundTelemetry(const GroundTelemetry&)=delete;
  GroundTelemetry(const GroundTelemetry&&)=delete;
  /**
   * Telemetry will run infinite in its own threads until an error occurs.
   * @param enableExtendedLogging be really verbose on logging.
   */
  [[noreturn]] void loopInfinite(bool enableExtendedLogging = false);
  [[nodiscard]] std::string createDebug()const;
  // add settings to the generic mavlink parameter server
  // changes are propagated back through the settings instances
  void add_settings_generic(const std::vector<openhd::Setting>& settings);
  // call once all settings have been added, this is needed to avoid an invariant parameter set
  void settings_generic_ready();
  void set_link_statistics(openhd::link_statistics::AllStats stats);
  // Add the IP of another Ground station client, to start forwarding telemetry data there
  void add_external_ground_station_ip(const std::string& ip_openhd,const std::string& ip_dest_device);
  void remove_external_ground_station_ip(const std::string& ip_openhd,const std::string& ip_dest_device);
 private:
  const OHDPlatform _platform;
  // called every time a message from the air pi is received
  void onMessageAirPi(MavlinkMessage &message);
  // send a message to the air pi
  void sendMessageAirPi(const MavlinkMessage &message);
  // called every time a message is received from any of the clients connected to the Ground Station (For Example QOpenHD)
  void onMessageGroundStationClients(MavlinkMessage &message);
  // send a message to all clients connected to the ground station, for example QOpenHD
  void sendMessageGroundStationClients(const MavlinkMessage &message);
 private:
  //std::unique_ptr<UDPEndpoint> udpGroundClient = nullptr;
  std::unique_ptr<UDPEndpoint2> udpGroundClient = nullptr;
  // We rely on another service for starting the rx/tx links
  std::unique_ptr<UDPEndpoint> udpWifibroadcastEndpoint;
  std::shared_ptr<OHDMainComponent> _ohd_main_component;
  std::mutex components_lock;
  std::vector<std::shared_ptr<MavlinkComponent>> components;
  std::shared_ptr<XMavlinkParamProvider> generic_mavlink_param_provider;
  // telemetry to / from external ground stations (e.g. not the QOpenHD instance running on the device itself (localhost)
  std::mutex other_udp_ground_stations_lock;
  std::map<std::string,std::shared_ptr<UDPEndpoint2>> _other_udp_ground_stations{};
  //
  //std::unique_ptr<JoystickReader> m_joystick_reader;
};

#endif //OPENHD_TELEMETRY_GROUNDTELEMETRY_H
