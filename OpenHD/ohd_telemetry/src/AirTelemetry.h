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
#include "endpoints/WBEndpoint.h"

/**
 * OpenHD Air telemetry. Assumes a Ground instance running on the ground pi.
 */
class AirTelemetry : public MavlinkSystem{
 public:
  explicit AirTelemetry(OHDPlatform platform,std::shared_ptr<openhd::ActionHandler> opt_action_handler=nullptr);
  AirTelemetry(const AirTelemetry&)=delete;
  AirTelemetry(const AirTelemetry&&)=delete;
  ~AirTelemetry();
  /**
   * Telemetry will run infinite in its own threads until terminate is set to true
   * @param enableExtendedLogging be really verbose on logging.
   */
  void loop_infinite(bool& terminate,bool enableExtendedLogging = false);
  /**
   * @return verbose string about the current state, for debugging
   */
  [[nodiscard]] std::string create_debug();
  /**
   * add settings to the generic mavlink parameter server
   * changes are propagated back through the settings instances
   * @param settings the settings to add
   */
  void add_settings_generic(const std::vector<openhd::Setting>& settings);
  /**
   * must be called once all settings have been added, this is needed to avoid an invariant parameter set
   */
  void settings_generic_ready();
  /**
   * On the air unit we use mavlink to change camera settings. We have exactly one mavlink param server per camera
   * @param camera_index 0 for primary camera, 1 for secondary camera, ...
   * @param settings the settings for this camera
   */
  void add_settings_camera_component(int camera_index,const std::vector<openhd::Setting>& settings);
  //
  void set_wb_tx_rx_handle(std::shared_ptr<openhd::ITransmitReceiveTelemetry> handle);
 private:
  const OHDPlatform _platform;
  std::unique_ptr<openhd::telemetry::air::SettingsHolder> _airTelemetrySettings;
  // send a mavlink message to the flight controller connected to the air unit via UART, if connected.
  void send_messages_fc(const std::vector<MavlinkMessage>& messages);
  // send mavlink messages to the ground unit, lossy
  void send_messages_ground_unit(const std::vector<MavlinkMessage>& messages);
  // called every time one or more messages from the flight controller are received
  void on_messages_fc(const std::vector<MavlinkMessage>& messages);
  // called every time one or more messages from the ground unit are received
  void on_messages_ground_unit(const std::vector<MavlinkMessage>& messages);
 private:
  std::mutex _serialEndpointMutex;
  std::unique_ptr<SerialEndpoint> serialEndpoint;
  // For now, use UDP endpoint and rely on another service for starting the rx/tx links
  //std::unique_ptr<UDPEndpoint> wifibroadcastEndpoint;
  std::unique_ptr<WBEndpoint> wifibroadcastEndpoint;
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
