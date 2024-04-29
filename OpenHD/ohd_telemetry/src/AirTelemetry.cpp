//
// Created by consti10 on 13.04.22.
//

#include "AirTelemetry.h"

#include <chrono>

#include "mav_helper.h"
#include "mavsdk_temporary/XMavlinkParamProvider.h"
#include "openhd_temporary_air_or_ground.h"
#include "openhd_util.h"
#include "openhd_util_time.h"

AirTelemetry::AirTelemetry() : MavlinkSystem(OHD_SYS_ID_AIR) {
  m_console = openhd::log::create_or_get("air_tele");
  assert(m_console);
  m_air_settings = std::make_unique<openhd::telemetry::air::SettingsHolder>();
  m_fc_serial = std::make_unique<SerialEndpointManager>();
  m_ohd_main_component = std::make_shared<OHDMainComponent>(_sys_id, true);
  m_components.push_back(m_ohd_main_component);
  //
  m_generic_mavlink_param_provider = std::make_shared<XMavlinkParamProvider>(
      _sys_id, MAV_COMP_ID_ONBOARD_COMPUTER);
  if (OHDPlatform::instance().is_rpi()) {
    m_opt_gpio_control =
        std::make_unique<openhd::telemetry::rpi::GPIOControl>();
  }
  // NOTE: We don't call set ready yet, since we have to wait until other
  // modules have provided all their paramters.
  m_generic_mavlink_param_provider->add_params(get_all_settings());
  m_components.push_back(m_generic_mavlink_param_provider);
  m_tcp_server = std::make_unique<TCPEndpoint>(
      openhd::TCPServer::Config{TCPEndpoint::DEFAULT_PORT});  // 1445
  if (m_tcp_server) {
    m_tcp_server->registerCallback(
        [this](std::vector<MavlinkMessage> messages) {
          // Technically not correct, but works
          on_messages_ground_unit(messages);
        });
  }
  setup_uart();
  m_console->debug("Created AirTelemetry");
}

AirTelemetry::~AirTelemetry() {}

void AirTelemetry::send_messages_fc(std::vector<MavlinkMessage>& messages) {
  auto [generic, local_only] =
      split_into_generic_and_local_only(messages, OHD_SYS_ID_AIR);
  // NOTE: Remember there is a hack in place for rc channels override in regards
  // to the sender sys id
  m_fc_serial->send_messages_if_enabled(generic);
}

void AirTelemetry::send_messages_ground_unit(
    std::vector<MavlinkMessage>& messages) {
  if (m_wb_endpoint) {
    // Optimization: Increase reliability of responding to mavlink (extended)
    // parameter set responses
    for (auto& msg : messages) {
      const auto msg_id = msg.m.msgid;
      if (msg_id == MAVLINK_MSG_ID_PARAM_EXT_VALUE ||
          msg_id == MAVLINK_MSG_ID_PARAM_VALUE) {
        msg.recommended_n_injections = 2;
      }
    }
    m_wb_endpoint->sendMessages(messages);
  }
  // Not technically correct, but works
  if (m_tcp_server) {
    m_tcp_server->sendMessages(messages);
  }
}

void AirTelemetry::on_messages_fc(std::vector<MavlinkMessage>& messages) {
  // openhd::log::get_default()->debug("on_messages_fc {}",messages.size());
  // debugMavlinkMessage(message.m,"AirTelemetry::onMessageFC");
  //  Note: No OpenHD component ever talks to the FC, FC is completely passed
  //  through
  // debugMavlinkMessages(messages,"FC");
  send_messages_ground_unit(messages);
  m_ohd_main_component->check_fc_messages_for_actions(messages);
}

void AirTelemetry::on_messages_ground_unit(
    std::vector<MavlinkMessage>& messages) {
  // m_console->debug("on_messages_ground_unit {}", messages.size());
  //   filter out heartbeats from the openhd ground unit,we do not need to send
  //   them to the FC
  std::vector<MavlinkMessage> filtered_messages_fc;
  for (const auto& msg : messages) {
    const mavlink_message_t& m = msg.m;
    if (static_cast<int>(m.msgid) == MAVLINK_MSG_ID_HEARTBEAT &&
        m.sysid == OHD_SYS_ID_GROUND)
      continue;
    filtered_messages_fc.push_back(msg);
  }
  send_messages_fc(filtered_messages_fc);
  // any data created by an OpenHD component on the air pi only needs to be sent
  // to the ground pi, the FC cannot do anything with it anyways.
  std::lock_guard<std::mutex> guard(m_components_lock);
  for (auto& component : m_components) {
    std::vector<MavlinkMessage> responses{};
    OHDUtil::vec_append(responses,
                        component->process_mavlink_messages(messages));
    send_messages_ground_unit(responses);
  }
}

void AirTelemetry::loop_infinite(bool& terminate,
                                 const bool enableExtendedLogging) {
  const auto log_intervall = std::chrono::seconds(5);
  const auto loop_intervall = std::chrono::milliseconds(100);
  auto last_log = std::chrono::steady_clock::now();
  while (!terminate) {
    const auto loopBegin = std::chrono::steady_clock::now();
    if (std::chrono::steady_clock::now() - last_log >= log_intervall) {
      // State debug logging
      last_log = std::chrono::steady_clock::now();
      // m_console->debug("AirTelemetry::loopInfinite()");
      //  for debugging, check if any of the endpoints is not alive
      if (enableExtendedLogging && m_wb_endpoint) {
        m_console->debug(m_wb_endpoint->createInfo());
      }
    }
    // send messages to the ground pi in regular intervals, includes heartbeat.
    // everything else is handled by the callbacks and their threads
    {
      // NOTE: No component on the air unit ever needs to talk to the FC himself
      std::lock_guard<std::mutex> guard(m_components_lock);
      for (auto& component : m_components) {
        auto messages = component->generate_mavlink_messages();
        send_messages_ground_unit(messages);
      }
    }
    const auto loopDelta = std::chrono::steady_clock::now() - loopBegin;
    if (loopDelta > loop_intervall) {
      // We can't keep up with the wanted loop interval
      m_console->debug(
          "Warning AirTelemetry cannot keep up with the wanted loop interval. "
          "Took {}",
          openhd::util::time_readable(loopDelta));
    } else {
      const auto sleepTime = loop_intervall - loopDelta;
      // send out in X second intervals
      std::this_thread::sleep_for(loop_intervall);
    }
  }
}

std::string AirTelemetry::create_debug() {
  std::stringstream ss;
  // ss<<"AT:\n";
  if (m_wb_endpoint) {
    ss << m_wb_endpoint->createInfo();
  }
  return ss.str();
}

void AirTelemetry::add_settings_generic(
    const std::vector<openhd::Setting>& settings) {
  std::lock_guard<std::mutex> guard(m_components_lock);
  m_generic_mavlink_param_provider->add_params(settings);
  m_console->debug("Added parameter component");
}

void AirTelemetry::settings_generic_ready() {
  m_generic_mavlink_param_provider->set_ready();
}

void AirTelemetry::add_settings_camera_component(
    int camera_index, const std::vector<openhd::Setting>& settings) {
  assert(camera_index >= 0 && camera_index < 2);
  const auto cam_comp_id = MAV_COMP_ID_CAMERA + camera_index;
  auto param_server = std::make_shared<XMavlinkParamProvider>(
      _sys_id, cam_comp_id, std::chrono::seconds(1));
  param_server->add_params(settings);
  param_server->set_ready();
  std::lock_guard<std::mutex> guard(m_components_lock);
  m_components.push_back(param_server);
  m_console->debug("Added camera component");
}

std::vector<openhd::Setting> AirTelemetry::get_all_settings() {
  std::vector<openhd::Setting> ret{};
  using namespace openhd::telemetry;
  auto c_fc_uart_connection_type = [this](std::string, std::string value) {
    // We just accept anything
    m_air_settings->unsafe_get_settings().fc_uart_connection_type = value;
    m_air_settings->persist();
    setup_uart();
    return true;
  };
  auto c_fc_uart_baudrate = [this](std::string, int value) {
    if (!SerialEndpoint::is_valid_linux_baudrate(value)) return false;
    m_air_settings->unsafe_get_settings().fc_uart_baudrate = value;
    m_air_settings->persist();
    setup_uart();
    return true;
  };
  auto c_fc_uart_flow_control = [this](std::string, int value) {
    if (!openhd::validate_yes_or_no(value)) {
      return false;
    }
    m_air_settings->unsafe_get_settings().fc_uart_flow_control = value;
    m_air_settings->persist();
    setup_uart();
    return true;
  };
  auto c_fc_battery_n_cells = [this](std::string, int value) {
    if (value < 0) return false;
    m_air_settings->unsafe_get_settings().fc_battery_n_cells = value;
    m_air_settings->persist(false);
    return true;
  };
  ret.push_back(openhd::Setting{
      air::FC_UART_CONNECTION_TYPE,
      openhd::StringSetting{
          m_air_settings->get_settings().fc_uart_connection_type,
          c_fc_uart_connection_type}});
  ret.push_back(openhd::Setting{
      air::FC_UART_BAUD_RATE,
      openhd::IntSetting{
          static_cast<int>(m_air_settings->get_settings().fc_uart_baudrate),
          c_fc_uart_baudrate}});
  ret.push_back(openhd::Setting{
      air::FC_UART_FLOW_CONTROL,
      openhd::IntSetting{
          static_cast<int>(m_air_settings->get_settings().fc_uart_flow_control),
          c_fc_uart_flow_control}});
  ret.push_back(openhd::Setting{
      air::FC_BATT_N_CELLS,
      openhd::IntSetting{
          static_cast<int>(m_air_settings->get_settings().fc_battery_n_cells),
          c_fc_battery_n_cells}});
  // and this allows an advanced user to change its air unit to a ground unit
  // only expose this setting if OpenHD uses the file workaround to figure out
  // air or ground.
  if (openhd::tmp::file_air_or_ground_exists()) {
    auto c_config_boot_as_air = [](std::string, int value) {
      return openhd::tmp::handle_telemetry_change(value);
    };
    ret.push_back(openhd::Setting{"CONFIG_BOOT_AIR",
                                  openhd::IntSetting{1, c_config_boot_as_air}});
  }
  if (m_opt_gpio_control != nullptr) {
    OHDUtil::vec_append(ret, m_opt_gpio_control->get_all_settings());
  }
  openhd::testing::append_dummy_if_empty(ret);
  return ret;
}

// Every time the UART configuration changes, we just re-start the UART (if it
// was already started) This properly handles all the cases, e.g cleaning up an
// existing uart connection if set.
void AirTelemetry::setup_uart() {
  assert(m_air_settings);
  using namespace openhd::telemetry;
  const auto uart_linux_fd = serial_openhd_param_to_linux_fd(
      m_air_settings->get_settings().fc_uart_connection_type);
  if (uart_linux_fd.has_value()) {
    SerialEndpoint::HWOptions options{};
    options.linux_filename = uart_linux_fd.value();
    options.baud_rate = m_air_settings->get_settings().fc_uart_baudrate;
    options.flow_control = m_air_settings->get_settings().fc_uart_flow_control;
    options.enable_reading = true;
    m_fc_serial->configure(options, "fc_ser",
                           [this](std::vector<MavlinkMessage> messages) {
                             this->on_messages_fc(messages);
                           });
  } else {
    m_fc_serial->disable();
  }
}

void AirTelemetry::set_link_handle(std::shared_ptr<OHDLink> link) {
  m_wb_endpoint = std::make_unique<WBEndpoint>(link, "wb_tx");
  m_wb_endpoint->registerCallback([this](std::vector<MavlinkMessage> messages) {
    on_messages_ground_unit(messages);
  });
}
