//
// Created by consti10 on 13.04.22.
//

#include "GroundTelemetry.h"

#include <chrono>
#include <iostream>

#include "mav_helper.h"
#include "openhd_temporary_air_or_ground.h"
#include "openhd_util.h"
#include "openhd_util_time.h"

GroundTelemetry::GroundTelemetry() : MavlinkSystem(OHD_SYS_ID_GROUND) {
  m_console = openhd::log::create_or_get("ground_tele");
  assert(m_console);
  m_gnd_settings =
      std::make_unique<openhd::telemetry::ground::SettingsHolder>();
  m_endpoint_tracker = std::make_unique<SerialEndpointManager>();
  m_gcs_endpoint = std::make_unique<UDPEndpoint>(
      "GroundStationUDP", OHD_GROUND_CLIENT_UDP_PORT_OUT,
      OHD_GROUND_CLIENT_UDP_PORT_IN,
      // We send data to localhost::14550 and any other external device IPs
      "127.0.0.1",
      // and we accept udp data from anybody on 14551
      "0.0.0.0");
  m_gcs_endpoint->registerCallback(
      [this](std::vector<MavlinkMessage> messages) {
        on_messages_ground_station_clients(messages);
      });
  m_tcp_server = std::make_unique<TCPEndpoint>(
      openhd::TCPServer::Config{TCPEndpoint::DEFAULT_PORT});  // 1445
  // m_tcp_server= nullptr;
  if (m_tcp_server) {
    m_tcp_server->registerCallback(
        [this](std::vector<MavlinkMessage> messages) {
          on_messages_ground_station_clients(messages);
        });
  }
  m_ohd_main_component = std::make_shared<OHDMainComponent>(_sys_id, false);
  m_components.push_back(m_ohd_main_component);
#ifdef OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
  if (m_gnd_settings->get_settings().enable_rc_over_joystick) {
    enable_joystick();
  } else {
    m_console->info("Joystick disabled");
  }
#else
  m_console->info("No Joystick support");
#endif
  //
  // NOTE: We don't call set ready yet, since we have to wait until other
  // modules have provided all their parameters.
  m_generic_mavlink_param_provider = std::make_shared<XMavlinkParamProvider>(
      _sys_id, MAV_COMP_ID_ONBOARD_COMPUTER);
  m_generic_mavlink_param_provider->add_params(get_all_settings());
  m_components.push_back(m_generic_mavlink_param_provider);
  setup_uart();
  openhd::ExternalDeviceManager::instance().register_listener(
      [this](openhd::ExternalDevice external_device, bool connected) {
        if (!external_device.discovered_by_mavlink_tcp_server) {
          if (connected) {
            add_external_ground_station_ip(external_device);
          } else {
            remove_external_ground_station_ip(external_device);
          }
        }
      });
  m_console->debug("Created GroundTelemetry");
}

GroundTelemetry::~GroundTelemetry() {
  // first, stop all the endpoints that have their own threads
  m_wb_endpoint = nullptr;
  m_gcs_endpoint = nullptr;
  if (m_gcs_endpoint) {
    m_gcs_endpoint = nullptr;
  }
  if (m_tcp_server) {
    m_tcp_server = nullptr;
  }
}

void GroundTelemetry::on_messages_air_unit(
    const std::vector<MavlinkMessage>& messages) {
  // All messages we get from the Air pi (they might come from the AirPi itself
  // or the FC connected to the air pi) get forwarded straight to all the
  // client(s) connected to the ground station.
  send_messages_ground_station_clients(messages);
  // Note: No OpenHD component ever talks to another OpenHD component or the FC,
  // so we do not need to do anything else here. tracker serial out - we are
  // only interested in message(s) coming from the FC
  // 17.April: One exception - timesync
  for (const auto& msg : messages) {
    if (msg.m.msgid == MAVLINK_MSG_ID_TIMESYNC) {
      m_ohd_main_component->handle_timesync_message(msg);
    }
  }
  if (m_endpoint_tracker != nullptr) {
    auto msges_from_fc = filter_by_source_sys_id(messages, OHD_SYS_ID_FC);
    if (msges_from_fc.empty()) {
      msges_from_fc =
          filter_by_source_sys_id(messages, OHD_SYS_ID_FC_BETAFLIGHT);
    }
    if (!msges_from_fc.empty()) {
      m_endpoint_tracker->send_messages_if_enabled(msges_from_fc);
    }
  }
  m_ohd_main_component->check_fc_messages_for_actions(messages);
}

void GroundTelemetry::on_messages_ground_station_clients(
    const std::vector<MavlinkMessage>& messages) {
  // debugMavlinkMessages(messages,"GSC");
  //  All messages from the ground station(s) are forwarded to the air unit,
  //  unless they have a target sys id of the ohd ground unit itself
  auto [generic, local_only] =
      split_into_generic_and_local_only(messages, OHD_SYS_ID_GROUND);
  for (auto& msg_generic : generic) {
    // In general, since the uplink suffers that much from over-talking by the
    // video from the air unit, send each message twice by default - we do not
    // send much mavlink to the air unit anyway. We do this for all messages
    // unless it's a heartbeat.
    const auto msg_id = msg_generic.m.msgid;
    if (msg_id == MAVLINK_MSG_ID_HEARTBEAT) {
      msg_generic.recommended_n_injections = 1;
    } else {
      msg_generic.recommended_n_injections = 2;
    }
    // optimization: The telemetry link is quite lossy, here we help QOpenHD (or
    // anybody else) on special message(s). WB link makes sure duplicates are
    // discarded
    if (msg_id == MAVLINK_MSG_ID_PARAM_EXT_SET  // Param protocol
        || msg_id == MAVLINK_MSG_ID_PARAM_EXT_REQUEST_READ ||
        msg_id == MAVLINK_MSG_ID_PARAM_EXT_REQUEST_LIST ||
        msg_id == MAVLINK_MSG_ID_PARAM_SET ||
        msg_id == MAVLINK_MSG_ID_PARAM_REQUEST_READ ||
        msg_id == MAVLINK_MSG_ID_PARAM_REQUEST_LIST
        // command protocol
        || msg_id == MAVLINK_MSG_ID_COMMAND_LONG ||
        msg_id == MAVLINK_MSG_ID_COMMAND_INT
        // mission protocol
        || msg_id == MAVLINK_MSG_ID_MISSION_REQUEST_LIST ||
        msg_id == MAVLINK_MSG_ID_MISSION_REQUEST_INT) {
      msg_generic.recommended_n_injections = 4;
    }
  }
  send_messages_air_unit(generic);
  // OpenHD components running on the ground station don't need to talk to the
  // air unit. This is not exactly following the mavlink routing standard, but
  // saves a lot of bandwidth.
  std::lock_guard<std::mutex> guard(m_components_lock);
  for (auto& component : m_components) {
    const auto responses = component->process_mavlink_messages(messages);
    // for now, send to the ground station clients only
    send_messages_ground_station_clients(responses);
  }
}

void GroundTelemetry::send_messages_ground_station_clients(
    const std::vector<MavlinkMessage>& messages) {
  if (m_gcs_endpoint) {
    m_gcs_endpoint->sendMessages(messages);
  }
  if (m_tcp_server) {
    m_tcp_server->sendMessages(messages);
  }
}

void GroundTelemetry::send_messages_air_unit(
    const std::vector<MavlinkMessage>& messages) {
  // transmit via wb / the abstract link we use for sending message(s) to the
  // air unit
  if (m_wb_endpoint) {
    m_wb_endpoint->sendMessages(messages);
  }
}

void GroundTelemetry::loop_infinite(bool& terminate,
                                    const bool enableExtendedLogging) {
  const auto log_intervall = std::chrono::seconds(5);
  const auto loop_intervall = std::chrono::milliseconds(100);
  auto last_log = std::chrono::steady_clock::now();
  while (!terminate) {
    const auto loopBegin = std::chrono::steady_clock::now();
    if (std::chrono::steady_clock::now() - last_log >= log_intervall) {
      last_log = std::chrono::steady_clock::now();
      // m_console->debug("GroundTelemetry::loopInfinite()");
      //  for debugging, check if any of the endpoints is not alive
      if (enableExtendedLogging && m_wb_endpoint) {
        m_console->debug(m_wb_endpoint->createInfo());
      }
      if (enableExtendedLogging && m_gcs_endpoint) {
        m_console->debug(m_gcs_endpoint->createInfo());
      }
    }
    // send messages to the ground station in regular intervals, includes
    // heartbeat. everything else is handled by the callbacks and their threads
    {
      // NOTE: No component from the ground station ever needs to talk to the
      // air unit / FC itself
      std::lock_guard<std::mutex> guard(m_components_lock);
      for (auto& component : m_components) {
        assert(component);
        const auto messages = component->generate_mavlink_messages();
        send_messages_ground_station_clients(messages);
        // exception: timesync
        for (const auto& msg : messages) {
          if (msg.m.msgid == MAVLINK_MSG_ID_TIMESYNC) {
            m_console->debug("Sending timesync to air");
            send_messages_air_unit({msg});
          }
        }
      }
    }
    const auto loopDelta = std::chrono::steady_clock::now() - loopBegin;
    if (loopDelta > loop_intervall) {
      // We can't keep up with the wanted loop interval
      // We can't keep up with the wanted loop interval
      m_console->debug(
          "Warning GroundTelemetry cannot keep up with the wanted loop "
          "interval. Took {}",
          openhd::util::time_readable(loopDelta));
    } else {
      const auto sleepTime = loop_intervall - loopDelta;
      // send out in X second intervals
      std::this_thread::sleep_for(loop_intervall);
    }
  }
}

std::string GroundTelemetry::create_debug() const {
  std::stringstream ss;
  // ss<<"GT:\n";
  if (m_wb_endpoint) {
    ss << m_wb_endpoint->createInfo();
  }
  if (m_gcs_endpoint) {
    ss << m_gcs_endpoint->createInfo();
  }
  return ss.str();
}

void GroundTelemetry::add_settings_generic(
    const std::vector<openhd::Setting>& settings) {
  std::lock_guard<std::mutex> guard(m_components_lock);
  m_generic_mavlink_param_provider->add_params(settings);
  m_console->debug("Added parameter component");
}

void GroundTelemetry::settings_generic_ready() {
  m_generic_mavlink_param_provider->set_ready();
}

void GroundTelemetry::add_external_ground_station_ip(
    const openhd::ExternalDevice& ext_device) {
  m_console->debug("add_external_ground_station_ip {}", ext_device.to_string());
  if (m_gcs_endpoint) {
    m_gcs_endpoint->addAnotherDestIpAddress(ext_device.external_device_ip);
  }
}

void GroundTelemetry::remove_external_ground_station_ip(
    const openhd::ExternalDevice& ext_device) {
  m_console->debug("remove_external_ground_station_ip {}",
                   ext_device.to_string());
  if (m_gcs_endpoint) {
    m_gcs_endpoint->removeAnotherDestIpAddress(ext_device.external_device_ip);
  }
}

std::vector<openhd::Setting> GroundTelemetry::get_all_settings() {
  std::vector<openhd::Setting> ret{};
  // and this allows an advanced user to change its air unit to a ground unit
  // only expose this setting if OpenHD uses the file workaround to figure out
  // air or ground.
  if (openhd::tmp::file_air_or_ground_exists()) {
    auto c_config_boot_as_air = [](std::string, int value) {
      return openhd::tmp::handle_telemetry_change(value);
    };
    ret.push_back(openhd::Setting{"CONFIG_BOOT_AIR",
                                  openhd::IntSetting{0, c_config_boot_as_air}});
  }
#ifdef OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
  if (true) {
    auto c_config_enable_joystick = [this](std::string, int value) {
      if (!openhd::validate_yes_or_no(value)) return false;
      m_gnd_settings->unsafe_get_settings().enable_rc_over_joystick = value;
      m_gnd_settings->persist();
      if (m_gnd_settings->unsafe_get_settings().enable_rc_over_joystick) {
        enable_joystick();
      } else {
        disable_joystick();
      }
      return true;
    };
    ret.push_back(openhd::Setting{
        "ENABLE_JOY_RC",
        openhd::IntSetting{
            static_cast<int>(
                m_gnd_settings->get_settings().enable_rc_over_joystick),
            c_config_enable_joystick}});
    // We always expose these params, regardless if joy is enabled or disabled
    auto c_rc_over_joystick_update_rate_hz = [this](std::string, int value) {
      if (!openhd::telemetry::ground::valid_joystick_update_rate(value))
        return false;
      m_gnd_settings->unsafe_get_settings().rc_over_joystick_update_rate_hz =
          value;
      m_gnd_settings->persist();
      if (m_rc_joystick_sender) {
        m_rc_joystick_sender->change_update_rate(value);
      }
      return true;
    };
    ret.push_back(openhd::Setting{
        "RC_UPDATE_HZ",
        openhd::IntSetting{
            static_cast<int>(
                m_gnd_settings->get_settings().rc_over_joystick_update_rate_hz),
            c_rc_over_joystick_update_rate_hz}});
    auto c_rc_over_joystick_channel_mapping = [this](std::string,
                                                     std::string value) {
      m_console->debug("Change channel mapping {}", value);
      const auto parsed = openhd::convert_string_to_channel_mapping(value);
      if (parsed == std::nullopt) {
        m_console->warn("Not a valid channel mapping");
        return false;
      }
      m_gnd_settings->unsafe_get_settings().rc_channel_mapping = value;
      m_gnd_settings->persist();
      if (m_rc_joystick_sender) {
        m_rc_joystick_sender->update_channel_mapping(parsed.value());
      }
      return true;
    };
    ret.push_back(openhd::Setting{
        "RC_CHAN_MAP",
        openhd::StringSetting{m_gnd_settings->get_settings().rc_channel_mapping,
                              c_rc_over_joystick_channel_mapping}});
  }
#endif
  if (true) {
    auto c_gnd_uart_connection_type = [this](std::string, std::string value) {
      if (!value.empty() && !OHDFilesystemUtil::exists(value)) {
        m_console->warn("{} is not a valid serial", value);
      }
      m_gnd_settings->unsafe_get_settings().gnd_uart_connection_type = value;
      m_gnd_settings->persist();
      setup_uart();
      return true;
    };
    ret.push_back(openhd::Setting{
        "TRACKER_UART_OUT",
        openhd::StringSetting{
            m_gnd_settings->get_settings().gnd_uart_connection_type,
            c_gnd_uart_connection_type}});
  }
  openhd::testing::append_dummy_if_empty(ret);
  return ret;
}

void GroundTelemetry::setup_uart() {
  assert(m_gnd_settings);
  using namespace openhd::telemetry;
  const auto uart_linux_fd = serial_openhd_param_to_linux_fd(
      m_gnd_settings->get_settings().gnd_uart_connection_type);
  if (uart_linux_fd.has_value()) {
    SerialEndpoint::HWOptions options{};
    options.linux_filename = uart_linux_fd.value();
    options.baud_rate = m_gnd_settings->get_settings().gnd_uart_baudrate;
    options.flow_control = false;
    options.enable_reading = false;
    m_endpoint_tracker->configure(options, "gnd_ser",
                                  [this](std::vector<MavlinkMessage> messages) {
                                    // We ignore any incoming messages here for
                                    // now, since it is only for mavlink out via
                                    // serial
                                  });
  } else {
    m_endpoint_tracker->disable();
  }
}

void GroundTelemetry::set_link_handle(std::shared_ptr<OHDLink> link) {
  // only call this once, we do not support changing the link handle at run time
  assert(m_wb_endpoint == nullptr);
  m_wb_endpoint = std::make_unique<WBEndpoint>(link, "wb_tx");
  m_wb_endpoint->registerCallback([this](std::vector<MavlinkMessage> messages) {
    on_messages_air_unit(messages);
  });
}

#ifdef OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
void GroundTelemetry::enable_joystick() {
  if (m_rc_joystick_sender != nullptr) {
    m_console->warn("Joy already enabled");
    return;
  }
  auto cb = [this](std::array<uint16_t, 18> channels) {
    // Dirty - we want the message both in the GCS station for debugging BUT
    // need to perform some annoying workaround for ARDUPILOT in regard to the
    // SYS id. Which is why we send the same data as 2 different messages to the
    // air unit(FC) and to the GCS stations, and the message for the air unit
    // has the "wrong" source sys id so to say See
    // https://github.com/ArduPilot/ardupilot/blob/master/libraries/GCS_MAVLink/GCS_Common.cpp#L3507
    // and https://github.com/ArduPilot/ardupilot/issues/1515
    auto msg_for_air =
        rc_channels_override_from_array(QOPENHD_SYS_ID, 1, channels, 0, 0);
    send_messages_air_unit({msg_for_air});
    // to the GCS stations
    auto msg_for_gcs =
        rc_channels_override_from_array(OHD_SYS_ID_GROUND, 0, channels, 0, 0);
    send_messages_ground_station_clients({msg_for_gcs});
  };
  auto mapping_parsed = openhd::convert_string_to_channel_mapping_or_default(
      m_gnd_settings->get_settings().rc_channel_mapping);
  m_rc_joystick_sender = std::make_unique<RcJoystickSender>(
      cb, m_gnd_settings->get_settings().rc_over_joystick_update_rate_hz,
      mapping_parsed);
  m_console->info("Joystick enabled");
}
void GroundTelemetry::disable_joystick() {
  if (m_rc_joystick_sender == nullptr) {
    m_console->warn("Joy already disabled");
    return;
  }
  // Destruction might block, which is not ideal, but hey
  m_console->debug("Disable joy begin");
  m_rc_joystick_sender = nullptr;
  m_console->debug("Disable joy end");
}
#endif
