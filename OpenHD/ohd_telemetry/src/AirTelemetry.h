//
// Created by consti10 on 13.04.22.
//

#ifndef OPENHD_TELEMETRY_AIRTELEMETRY_H
#define OPENHD_TELEMETRY_AIRTELEMETRY_H

#include <string>
#include "endpoints/SerialEndpoint.h"
#include "endpoints/UDPEndpoint.h"
#include "internal/OHDMainComponent.h"
#include "mavlink_settings/ISettingsComponent.hpp"
#include "openhd-platform.hpp"
#include "openhd-link-statistics.hpp"
#include "routing/MavlinkSystem.hpp"
//
#include "mavsdk_temporary/XMavlinkParamProvider.h"
#include "AirTelemetrySettings.h"
#include "openhd-action-handler.hpp"
#include "openhd-spdlog.hpp"
// Dirty
#include "openhd-rpi-os-configure-vendor-cam.hpp"

/**
 * OpenHD Air telemetry. Assumes a Ground instance running on the ground pi.
 */
class AirTelemetry : public MavlinkSystem{
 public:
  explicit AirTelemetry(OHDPlatform platform,std::shared_ptr<openhd::ActionHandler> opt_action_handler=nullptr);
  AirTelemetry(const AirTelemetry&)=delete;
  AirTelemetry(const AirTelemetry&&)=delete;
  /**
   * Telemetry will run infinite in its own threads until an error occurs.
   * @param enableExtendedLogging be really verbose on logging.
   */
  void loopInfinite(bool& terminate,bool enableExtendedLogging = false);
  [[nodiscard]] std::string createDebug();
  // add settings to the generic mavlink parameter server
  // changes are propagated back through the settings instances
  void add_settings_generic(const std::vector<openhd::Setting>& settings);
  // call once all settings have been added, this is needed to avoid an invariant parameter set
  void settings_generic_ready();
  // We have a unique component id / param server per camera
  void add_camera_component(int camera_index,const std::vector<openhd::Setting>& settings);
 private:
  const OHDPlatform _platform;
  std::unique_ptr<openhd::telemetry::air::SettingsHolder> _airTelemetrySettings;
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
  std::unique_ptr<UDPEndpoint> wifibroadcastEndpoint;
  // shared because we also push it onto our components list
  std::shared_ptr<OHDMainComponent> m_ohd_main_component;
  std::mutex components_lock;
  std::vector<std::shared_ptr<MavlinkComponent>> components;
  std::shared_ptr<XMavlinkParamProvider> generic_mavlink_param_provider;
  // R.N only on air, and only FC uart settings
  std::vector<openhd::Setting> get_all_settings();
  void setup_uart();
  std::unique_ptr<openhd::rpi::os::ConfigChangeHandler> m_rpi_os_change_config_handler=nullptr;
  std::shared_ptr<spdlog::logger> m_console;
};

#endif //OPENHD_TELEMETRY_AIRTELEMETRY_H
