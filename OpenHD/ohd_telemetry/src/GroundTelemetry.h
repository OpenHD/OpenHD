//
// Created by consti10 on 13.04.22.
//

#ifndef OPENHD_TELEMETRY_GROUNDTELEMETRY_H
#define OPENHD_TELEMETRY_GROUNDTELEMETRY_H

#include "GroundTelemetrySettings.h"
#include "endpoints/SerialEndpoint.h"
#include "endpoints/TCPEndpoint.h"
#include "endpoints/UDPEndpoint.h"
#include "endpoints/WBEndpoint.h"
#include "internal/OHDMainComponent.h"
#include "mavsdk_temporary/XMavlinkParamProvider.h"
#include "openhd_action_handler.h"
#include "openhd_external_device.h"
#include "openhd_link.hpp"
#include "openhd_settings_imp.h"
#include "openhd_spdlog.h"

#ifdef OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
#include "rc/JoystickReader.h"
#include "rc/RcJoystickSender.h"
#endif

/**
 * OpenHD Ground telemetry. Assumes a air instance running on the air pi.
 */
class GroundTelemetry : public MavlinkSystem {
 public:
  explicit GroundTelemetry();
  GroundTelemetry(const GroundTelemetry&) = delete;
  GroundTelemetry(const GroundTelemetry&&) = delete;
  ~GroundTelemetry();
  /**
   * Telemetry will run infinite in its own threads until terminate is set to
   * true
   * @param enableExtendedLogging be really verbose on logging.
   */
  void loop_infinite(bool& terminate, bool enableExtendedLogging = false);
  /**
   * @return verbose string about the current state, for debugging
   */
  [[nodiscard]] std::string create_debug() const;
  /**
   * add settings to the generic mavlink parameter server
   * changes are propagated back through the settings instances
   * @param settings the settings to add
   */
  void add_settings_generic(const std::vector<openhd::Setting>& settings);
  /**
   * must be called once all settings have been added, this is needed to avoid
   * an invariant parameter set
   */
  void settings_generic_ready();
  // Add the IP of another Ground station client, to start forwarding telemetry
  // data there
  void add_external_ground_station_ip(const openhd::ExternalDevice& ext_device);
  void remove_external_ground_station_ip(
      const openhd::ExternalDevice& ext_device);
  /**
   * The link handle can be set later after instantiation - until it is set,
   * messages from/to the air unit are just discarded.
   */
  void set_link_handle(std::shared_ptr<OHDLink> link);

 private:
  // called every time one or more messages from the air unit are received
  void on_messages_air_unit(const std::vector<MavlinkMessage>& messages);
  // send messages to the air unit, lossy
  void send_messages_air_unit(const std::vector<MavlinkMessage>& messages);
  // called every time one or more messages are received from any of the clients
  // connected to the Ground Station (For Example QOpenHD)
  void on_messages_ground_station_clients(
      const std::vector<MavlinkMessage>& messages);
  // send one or more messages to all clients connected to the ground station,
  // for example QOpenHD
  void send_messages_ground_station_clients(
      const std::vector<MavlinkMessage>& messages);
  std::vector<openhd::Setting> get_all_settings();
  void setup_uart();
#ifdef OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
  void enable_joystick();
  void disable_joystick();
#endif  // OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
 private:
  std::shared_ptr<spdlog::logger> m_console;
  std::unique_ptr<openhd::telemetry::ground::SettingsHolder> m_gnd_settings;
  // Mavlink to / from gcs station(s)
  std::unique_ptr<UDPEndpoint> m_gcs_endpoint = nullptr;
  // mavlink out via serial for tracker or similar
  std::unique_ptr<SerialEndpointManager> m_endpoint_tracker = nullptr;
  // EXP - always on TCP mavlink server
  std::unique_ptr<TCPEndpoint> m_tcp_server = nullptr;
  // send/receive data via wb
  std::unique_ptr<WBEndpoint> m_wb_endpoint;
  std::shared_ptr<OHDMainComponent> m_ohd_main_component;
  std::mutex m_components_lock;
  std::vector<std::shared_ptr<MavlinkComponent>> m_components;
  std::shared_ptr<XMavlinkParamProvider> m_generic_mavlink_param_provider;
  //
#ifdef OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
  std::unique_ptr<RcJoystickSender> m_rc_joystick_sender = nullptr;
#endif
};

#endif  // OPENHD_TELEMETRY_GROUNDTELEMETRY_H
