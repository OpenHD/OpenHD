//
// Created by consti10 on 13.04.22.
//

#ifndef OPENHD_TELEMETRY_AIRTELEMETRY_H
#define OPENHD_TELEMETRY_AIRTELEMETRY_H

#include <string>
#include "endpoints/SerialEndpoint.h"
#include "endpoints/UDPEndpoint.h"
#include "internal/OHDMainComponent.h"
#include "mavlink_settings/ISettingsComponent.h"
#include "openhd-platform.hpp"
#include "openhd-link-statistics.hpp"
#include "routing/MavlinkSystem.hpp"
//
#include "mavsdk_temporary/XMavlinkParamProvider.h"
#include "AirTelemetrySettings.h"

/**
 * OpenHD Air telemetry. Assumes a Ground instance running on the ground pi.
 */
class AirTelemetry : public MavlinkSystem{
 public:
  explicit AirTelemetry(OHDPlatform platform);
  AirTelemetry(const AirTelemetry&)=delete;
  AirTelemetry(const AirTelemetry&&)=delete;
  /**
   * Telemetry will run infinite in its own threads until an error occurs.
   * @param enableExtendedLogging be really verbose on logging.
   */
  [[noreturn]] void loopInfinite(bool enableExtendedLogging = false);
  [[nodiscard]] std::string createDebug()const;
  // add a mavlink parameter server that allows the user to change parameters.
  // changes in the parameter set are propagated back up by the "glue".
  void add_settings_generic(const std::vector<openhd::Setting>& settings);
  void settings_generic_ready();
  void add_camera_component(int camera_index,const std::vector<openhd::Setting>& settings);
  void set_link_statistics(openhd::link_statistics::AllStats stats);
 private:
  const OHDPlatform _platform;
  std::unique_ptr<openhd::AirTelemetrySettingsHolder> _airTelemetrySettings;
  // send a mavlink message to the flight controller connected to the air unit via UART, if connected.
  void sendMessageFC(const MavlinkMessage &message);
  // send a mavlink message to the ground pi, system cannot know if this message actually makes it.
  void sendMessageGroundPi(const MavlinkMessage &message);
  // called every time a message from the flight controller is received
  void onMessageFC(MavlinkMessage &message);
  // called every time a message from the ground pi is received
  void onMessageGroundPi(MavlinkMessage &message);
 private:
  std::mutex _serialEndpointMutex;
  std::unique_ptr<SerialEndpoint> serialEndpoint;
  // For now, use UDP endpoint and rely on another service for starting the rx/tx links
  //std::unique_ptr<WBEndpoint> wifibroadcastEndpoint;
  std::unique_ptr<UDPEndpoint> wifibroadcastEndpoint;
  std::shared_ptr<OHDMainComponent> _ohd_main_component;
  std::mutex components_lock;
  std::vector<std::shared_ptr<MavlinkComponent>> components;
  std::shared_ptr<XMavlinkParamProvider> generic_mavlink_param_provider;
  // R.N only on air, and only FC uart settings
  std::vector<openhd::Setting> get_all_settings();
  void setup_uart();
};

#endif //OPENHD_TELEMETRY_AIRTELEMETRY_H
