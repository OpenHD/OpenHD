/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#include "AirTelemetry.h"

#include "openhd_secondary_telemetry.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "mav_helper.h"
#include "mavsdk_temporary/XMavlinkParamProvider.h"
#include "openhd_telemetry_recorder.h"
#include "openhd_plugin_manager.h"
#include "openhd_temporary_air_or_ground.h"
#include "openhd_util.h"
#include "openhd_util_time.h"

namespace {

constexpr float kHalfPi = 1.57079632679489661923F;
constexpr float kRadiansToDegrees = 57.295779513082320876F;

// MAV_CMD_USER_1 subcommands used between QOpenHD and the OpenHD air unit.
// Standard MAVLink camera commands remain in use wherever one exists.
constexpr int kCameraPluginImageType = 1;
constexpr int kCameraPluginThermalPalette = 2;
constexpr int kCameraPluginGimbalMode = 3;
constexpr int kCameraPluginRollRate = 4;
constexpr int kCameraPluginCalibrate = 5;

bool is_camera_component(uint8_t component) {
  return component >= MAV_COMP_ID_CAMERA && component <= MAV_COMP_ID_CAMERA6;
}

bool is_gimbal_component(uint8_t component) {
  return component == MAV_COMP_ID_GIMBAL ||
         (component >= MAV_COMP_ID_GIMBAL2 && component <= MAV_COMP_ID_GIMBAL6);
}

uint32_t camera_index_for_component(uint8_t component) {
  return is_camera_component(component) ? component - MAV_COMP_ID_CAMERA : 0;
}

MavlinkMessage camera_command_ack(uint8_t source_system,
                                  uint8_t source_component,
                                  uint16_t command, bool accepted) {
  MavlinkMessage response{};
  mavlink_msg_command_ack_pack(
      OHD_SYS_ID_AIR, MAV_COMP_ID_CAMERA, &response.m, command,
      accepted ? MAV_RESULT_ACCEPTED : MAV_RESULT_UNSUPPORTED, 255, 0,
      source_system, source_component);
  return response;
}

void record_mavlink_messages(const std::vector<MavlinkMessage>& messages,
                             const std::string& direction,
                             const std::string& source,
                             const std::string& destination) {
  auto& recorder = openhd::TelemetryRecorder::instance();
  for (const auto& msg : messages) {
    const mavlink_message_t& mav_msg = msg.m;
    const auto* payload =
        reinterpret_cast<const uint8_t*>(_MAV_PAYLOAD(&mav_msg));
    recorder.record_mavlink_message(direction, source, destination,
                                    mav_msg.sysid, mav_msg.compid,
                                    mav_msg.msgid, mav_msg.seq, payload,
                                    mav_msg.len);
  }
}

}  // namespace

AirTelemetry::AirTelemetry(bool ignoreSerial)
    : MavlinkSystem(OHD_SYS_ID_AIR), m_ignoreSerial(ignoreSerial) {
  m_console = openhd::log::create_or_get("air_tele");
  assert(m_console);
  m_air_settings = std::make_unique<openhd::telemetry::air::SettingsHolder>();
  openhd::TelemetryRecorder::instance().set_enabled(
      m_air_settings->get_settings().telemetry_logging_enabled);
  m_fc_serial = std::make_unique<SerialEndpointManager>();
  m_openhd_uart_serial = std::make_unique<SerialEndpointManager>();
  m_sbus_output = std::make_unique<SbusOutput>();
  m_ohd_main_component = std::make_shared<OHDMainComponent>(_sys_id, true);
  m_ohd_main_component->set_fc_sys_id(
      static_cast<uint8_t>(m_air_settings->get_settings().fc_sys_id));
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
  if (m_ignoreSerial) {
    m_console->info("Serial setup disabled by CLI");
  } else {
    setup_uart();
    setup_openhd_uart_telemetry();
    setup_sbus_output();
  }
  m_console->debug("Created AirTelemetry");
}

AirTelemetry::~AirTelemetry() {
  openhd::SecondaryTelemetryStatus::instance().set_configured("UART", false);
}

void AirTelemetry::send_messages_fc(std::vector<MavlinkMessage>& messages) {
  if (!m_fc_serial) return;
  auto [generic, local_only] =
      split_into_generic_and_local_only(messages, OHD_SYS_ID_AIR);
  // NOTE: Remember there is a hack in place for rc channels override in regards
  // to the sender sys id
  record_mavlink_messages(generic, "sent", "openhd_air", "flight_controller");
  m_fc_serial->send_messages_if_enabled(generic);
}

void AirTelemetry::send_messages_ground_unit(
    std::vector<MavlinkMessage>& messages) {
  record_mavlink_messages(messages, "sent", "openhd_air", "ground_unit");
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
  if (m_openhd_uart_serial) {
    const auto prioritized_messages = m_uart_prioritizer.sort_by_priority(
        messages, get_openhd_uart_priority_profile());
    m_openhd_uart_serial->send_messages_if_enabled(prioritized_messages);
  }
}

void AirTelemetry::on_messages_fc(std::vector<MavlinkMessage>& messages) {
  // openhd::log::get_default()->debug("on_messages_fc {}",messages.size());
  // debugMavlinkMessage(message.m,"AirTelemetry::onMessageFC");
  //  Note: No OpenHD component ever talks to the FC, FC is completely passed
  //  through
  // debugMavlinkMessages(messages,"FC");
  MavlinkHelpers::maybe_sync_system_time_from_gps(messages);
  record_mavlink_messages(messages, "received", "flight_controller",
                          "openhd_air");
  send_messages_ground_unit(messages);
  m_ohd_main_component->check_fc_messages_for_actions(messages);
}

void AirTelemetry::on_messages_ground_unit(
    std::vector<MavlinkMessage>& messages) {
  // m_console->debug("on_messages_ground_unit {}", messages.size());
  record_mavlink_messages(messages, "received", "ground_unit", "openhd_air");
  if (m_sbus_output) {
    for (const auto& msg : messages) {
      if (msg.m.msgid == MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE) {
        mavlink_rc_channels_override_t rc_override;
        mavlink_msg_rc_channels_override_decode(&msg.m, &rc_override);
        std::array<uint16_t, 18> channels{};
        channels[0] = rc_override.chan1_raw;
        channels[1] = rc_override.chan2_raw;
        channels[2] = rc_override.chan3_raw;
        channels[3] = rc_override.chan4_raw;
        channels[4] = rc_override.chan5_raw;
        channels[5] = rc_override.chan6_raw;
        channels[6] = rc_override.chan7_raw;
        channels[7] = rc_override.chan8_raw;
        channels[8] = rc_override.chan9_raw;
        channels[9] = rc_override.chan10_raw;
        channels[10] = rc_override.chan11_raw;
        channels[11] = rc_override.chan12_raw;
        channels[12] = rc_override.chan13_raw;
        channels[13] = rc_override.chan14_raw;
        channels[14] = rc_override.chan15_raw;
        channels[15] = rc_override.chan16_raw;
        channels[16] = rc_override.chan17_raw;
        channels[17] = rc_override.chan18_raw;
        m_sbus_output->update_channels(channels);
      }
    }
  }
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
  auto plugin_responses = process_plugin_camera_controls(messages);
  send_messages_ground_unit(plugin_responses);
  for (auto& component : m_components) {
    std::vector<MavlinkMessage> responses{};
    OHDUtil::vec_append(responses,
                        component->process_mavlink_messages(messages));
    send_messages_ground_unit(responses);
  }
}

std::vector<MavlinkMessage> AirTelemetry::process_plugin_camera_controls(
    const std::vector<MavlinkMessage>& messages) {
  std::vector<MavlinkMessage> responses;
  const auto send_control = [](uint32_t camera_index, int32_t action,
                               float value1 = 0.0F, float value2 = 0.0F,
                               uint32_t flags = 0U) {
    const openhd_plugin_camera_control_event event{
        sizeof(openhd_plugin_camera_control_event), camera_index, action,
        value1, value2, flags};
    return openhd::PluginManager::instance().notify_camera_control(event);
  };

  for (const auto& wrapped : messages) {
    const auto& msg = wrapped.m;
    if (msg.msgid == MAVLINK_MSG_ID_GIMBAL_MANAGER_SET_MANUAL_CONTROL) {
      mavlink_gimbal_manager_set_manual_control_t control{};
      mavlink_msg_gimbal_manager_set_manual_control_decode(&msg, &control);
      if (control.target_system != OHD_SYS_ID_AIR ||
          !(control.target_component == 0 ||
            is_gimbal_component(control.target_component))) continue;
      const float pitch =
          std::isfinite(control.pitch_rate)
              ? control.pitch_rate
              : (std::isfinite(control.pitch) ? control.pitch : 0.0F);
      const float yaw = std::isfinite(control.yaw_rate)
                            ? control.yaw_rate
                            : (std::isfinite(control.yaw) ? control.yaw : 0.0F);
      const uint32_t flags =
          (control.flags & GIMBAL_MANAGER_FLAGS_YAW_LOCK) != 0
              ? OPENHD_PLUGIN_CAMERA_CONTROL_YAW_LOCK
              : 0U;
      send_control(0, OPENHD_PLUGIN_GIMBAL_RATE,
                   std::clamp(pitch, -1.0F, 1.0F),
                   std::clamp(yaw, -1.0F, 1.0F), flags);
      continue;
    }
    if (msg.msgid == MAVLINK_MSG_ID_GIMBAL_MANAGER_SET_PITCHYAW) {
      mavlink_gimbal_manager_set_pitchyaw_t control{};
      mavlink_msg_gimbal_manager_set_pitchyaw_decode(&msg, &control);
      if (control.target_system != OHD_SYS_ID_AIR ||
          !(control.target_component == 0 ||
            is_gimbal_component(control.target_component))) continue;
      const uint32_t flags =
          (control.flags & GIMBAL_MANAGER_FLAGS_YAW_LOCK) != 0
              ? OPENHD_PLUGIN_CAMERA_CONTROL_YAW_LOCK
              : 0U;
      if (std::isfinite(control.pitch) || std::isfinite(control.yaw)) {
        const float pitch = std::isfinite(control.pitch) ? control.pitch : 0.0F;
        const float yaw = std::isfinite(control.yaw) ? control.yaw : 0.0F;
        send_control(0, OPENHD_PLUGIN_GIMBAL_ANGLE,
                     pitch * kRadiansToDegrees,
                     yaw * kRadiansToDegrees, flags);
      } else {
        const float pitch_rate = std::isfinite(control.pitch_rate)
                                     ? control.pitch_rate / kHalfPi : 0.0F;
        const float yaw_rate = std::isfinite(control.yaw_rate)
                                   ? control.yaw_rate / kHalfPi : 0.0F;
        send_control(0, OPENHD_PLUGIN_GIMBAL_RATE,
                     std::clamp(pitch_rate, -1.0F, 1.0F),
                     std::clamp(yaw_rate, -1.0F, 1.0F), flags);
      }
      continue;
    }
    if (msg.msgid != MAVLINK_MSG_ID_COMMAND_LONG) continue;

    mavlink_command_long_t command{};
    mavlink_msg_command_long_decode(&msg, &command);
    if (command.target_system != OHD_SYS_ID_AIR ||
        !(command.target_component == 0 ||
          is_camera_component(command.target_component) ||
          is_gimbal_component(command.target_component))) continue;

    const uint32_t camera_index =
        camera_index_for_component(command.target_component);
    bool recognized = true;
    bool accepted = false;
    switch (command.command) {
      case MAV_CMD_DO_GIMBAL_MANAGER_PITCHYAW: {
        const uint32_t flags =
            (static_cast<uint32_t>(command.param5) &
             GIMBAL_MANAGER_FLAGS_YAW_LOCK) != 0
                ? OPENHD_PLUGIN_CAMERA_CONTROL_YAW_LOCK
                : 0U;
        if ((static_cast<uint32_t>(command.param5) &
             GIMBAL_MANAGER_FLAGS_NEUTRAL) != 0) {
          accepted = send_control(camera_index,
                                  OPENHD_PLUGIN_GIMBAL_CENTER);
        } else if (std::isfinite(command.param1) ||
                   std::isfinite(command.param2)) {
          accepted = send_control(
              camera_index, OPENHD_PLUGIN_GIMBAL_ANGLE,
              std::isfinite(command.param1) ? command.param1 : 0.0F,
              std::isfinite(command.param2) ? command.param2 : 0.0F, flags);
        } else {
          accepted = send_control(
              camera_index, OPENHD_PLUGIN_GIMBAL_RATE,
              std::clamp((std::isfinite(command.param3) ? command.param3
                                                        : 0.0F) /
                             90.0F,
                         -1.0F, 1.0F),
              std::clamp((std::isfinite(command.param4) ? command.param4
                                                        : 0.0F) /
                             90.0F,
                         -1.0F, 1.0F), flags);
        }
        break;
      }
      case MAV_CMD_DO_MOUNT_CONTROL:
        if (static_cast<int>(command.param7) == MAV_MOUNT_MODE_NEUTRAL) {
          accepted = send_control(camera_index,
                                  OPENHD_PLUGIN_GIMBAL_CENTER);
        } else {
          accepted = send_control(camera_index, OPENHD_PLUGIN_GIMBAL_ANGLE,
                                  command.param1, command.param3);
        }
        break;
      case MAV_CMD_SET_CAMERA_ZOOM: {
        const int zoom_type = static_cast<int>(command.param1);
        if (zoom_type == ZOOM_TYPE_CONTINUOUS) {
          accepted = send_control(camera_index,
                                  OPENHD_PLUGIN_CAMERA_ZOOM_RATE,
                                  command.param2);
        } else if (zoom_type == ZOOM_TYPE_RANGE && command.param2 >= 0.0F &&
                   command.param2 <= 100.0F) {
          accepted = send_control(camera_index,
                                  OPENHD_PLUGIN_CAMERA_ZOOM_PERCENT,
                                  command.param2);
        }
        break;
      }
      case MAV_CMD_SET_CAMERA_FOCUS: {
        const int focus_type = static_cast<int>(command.param1);
        if (focus_type == FOCUS_TYPE_CONTINUOUS) {
          accepted = send_control(camera_index,
                                  OPENHD_PLUGIN_CAMERA_FOCUS_RATE,
                                  command.param2);
        } else if (focus_type == FOCUS_TYPE_AUTO ||
                   focus_type == FOCUS_TYPE_AUTO_SINGLE) {
          accepted = send_control(camera_index,
                                  OPENHD_PLUGIN_CAMERA_AUTO_FOCUS);
        }
        break;
      }
      case MAV_CMD_IMAGE_START_CAPTURE:
        if (command.param3 == 1.0F) {
          accepted = send_control(camera_index,
                                  OPENHD_PLUGIN_CAMERA_TAKE_PHOTO);
        }
        break;
      case MAV_CMD_VIDEO_START_CAPTURE:
        accepted = send_control(camera_index,
                                OPENHD_PLUGIN_CAMERA_RECORD_START);
        break;
      case MAV_CMD_VIDEO_STOP_CAPTURE:
        accepted = send_control(camera_index,
                                OPENHD_PLUGIN_CAMERA_RECORD_STOP);
        break;
      case MAV_CMD_USER_1: {
        const int subcommand = static_cast<int>(command.param1);
        if (subcommand == kCameraPluginImageType) {
          accepted = send_control(camera_index,
                                  OPENHD_PLUGIN_CAMERA_IMAGE_TYPE,
                                  command.param2);
        } else if (subcommand == kCameraPluginThermalPalette) {
          accepted = send_control(camera_index,
                                  OPENHD_PLUGIN_CAMERA_THERMAL_PALETTE,
                                  command.param2);
        } else if (subcommand == kCameraPluginGimbalMode) {
          accepted = send_control(camera_index, OPENHD_PLUGIN_GIMBAL_MODE,
                                  command.param2);
        } else if (subcommand == kCameraPluginRollRate) {
          accepted = send_control(
              camera_index, OPENHD_PLUGIN_GIMBAL_ROLL_RATE,
              std::clamp(command.param2, -1.0F, 1.0F));
        } else if (subcommand == kCameraPluginCalibrate) {
          accepted = send_control(camera_index,
                                  OPENHD_PLUGIN_GIMBAL_CALIBRATE);
        }
        break;
      }
      default:
        recognized = false;
        break;
    }
    if (recognized) {
      responses.push_back(camera_command_ack(msg.sysid, msg.compid,
                                             command.command, accepted));
    }
  }
  return responses;
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

UartPriorityProfile AirTelemetry::get_openhd_uart_priority_profile() const {
  const auto& settings = m_air_settings->get_settings();
  UartPriorityProfile profile{};
  profile.rc_priority = settings.openhd_uart_priority_rc;
  profile.openhd_priority = settings.openhd_uart_priority_openhd;
  profile.flight_controller_priority = settings.openhd_uart_priority_fc;
  profile.fc_sys_id = settings.fc_sys_id;
  return profile;
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
  auto c_fc_sys_id = [this](std::string, int value) {
    if (value < 0 || value > 254 || value == OHD_SYS_ID_GROUND ||
        value == OHD_SYS_ID_AIR || value == QOPENHD_SYS_ID) {
      return false;
    }
    m_air_settings->unsafe_get_settings().fc_sys_id = value;
    m_air_settings->persist(false);
    if (m_ohd_main_component) {
      m_ohd_main_component->set_fc_sys_id(static_cast<uint8_t>(value));
    }
    return true;
  };
  auto c_openhd_uart_conn = [this](std::string, std::string value) {
    m_air_settings->unsafe_get_settings().openhd_uart_telemetry_connection =
        value;
    m_air_settings->persist();
    setup_openhd_uart_telemetry();
    return true;
  };
  auto c_openhd_uart_enable = [this](std::string, int value) {
    if (!openhd::validate_yes_or_no(value)) return false;
    m_air_settings->unsafe_get_settings().openhd_uart_telemetry_enabled = value;
    m_air_settings->persist();
    setup_openhd_uart_telemetry();
    return true;
  };
  auto c_openhd_uart_baud = [this](std::string, int value) {
    if (!SerialEndpoint::is_valid_linux_baudrate(value)) return false;
    m_air_settings->unsafe_get_settings().openhd_uart_telemetry_baudrate =
        value;
    m_air_settings->persist();
    setup_openhd_uart_telemetry();
    return true;
  };
  auto c_openhd_uart_flow = [this](std::string, int value) {
    if (!openhd::validate_yes_or_no(value)) return false;
    m_air_settings->unsafe_get_settings().openhd_uart_telemetry_flow_control =
        value;
    m_air_settings->persist();
    setup_openhd_uart_telemetry();
    return true;
  };
  auto c_openhd_uart_prio_rc = [this](std::string, int value) {
    if (!UartPrioritizer::valid_priority_value(value)) return false;
    m_air_settings->unsafe_get_settings().openhd_uart_priority_rc = value;
    m_air_settings->persist(false);
    return true;
  };
  auto c_openhd_uart_prio_ohd = [this](std::string, int value) {
    if (!UartPrioritizer::valid_priority_value(value)) return false;
    m_air_settings->unsafe_get_settings().openhd_uart_priority_openhd = value;
    m_air_settings->persist(false);
    return true;
  };
  auto c_openhd_uart_prio_fc = [this](std::string, int value) {
    if (!UartPrioritizer::valid_priority_value(value)) return false;
    m_air_settings->unsafe_get_settings().openhd_uart_priority_fc = value;
    m_air_settings->persist(false);
    return true;
  };
  auto c_telemetry_logging = [this](std::string, int value) {
    if (!openhd::validate_yes_or_no(value)) return false;
    m_air_settings->unsafe_get_settings().telemetry_logging_enabled = value;
    m_air_settings->persist(false);
    openhd::TelemetryRecorder::instance().set_enabled(value != 0);
    return true;
  };
  auto c_sbus_enable = [this](std::string, int value) {
    if (!openhd::validate_yes_or_no(value)) return false;
    m_air_settings->unsafe_get_settings().sbus_out_enabled = value;
    m_air_settings->persist();
    setup_sbus_output();
    return true;
  };
  auto c_sbus_device = [this](std::string, std::string value) {
    m_air_settings->unsafe_get_settings().sbus_uart_device = value;
    m_air_settings->persist();
    setup_sbus_output();
    return true;
  };
  auto c_sbus_rate = [this](std::string, int value) {
    if (value < 1 || value > 200) return false;
    m_air_settings->unsafe_get_settings().sbus_update_rate_hz = value;
    m_air_settings->persist();
    setup_sbus_output();
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
  ret.push_back(openhd::Setting{
      air::FC_SYS_ID_PARAM,
      openhd::IntSetting{
          static_cast<int>(m_air_settings->get_settings().fc_sys_id),
          c_fc_sys_id}});
  ret.push_back(openhd::Setting{
      air::OPENHD_UART_TELEMETRY_PARAM,
      openhd::StringSetting{
          m_air_settings->get_settings().openhd_uart_telemetry_connection,
          c_openhd_uart_conn}});
  ret.push_back(openhd::Setting{
      air::OPENHD_UART_TELEMETRY_ENABLE_PARAM,
      openhd::IntSetting{
          static_cast<int>(
              m_air_settings->get_settings().openhd_uart_telemetry_enabled),
          c_openhd_uart_enable}});
  ret.push_back(openhd::Setting{
      air::OPENHD_UART_TELEMETRY_BAUD_PARAM,
      openhd::IntSetting{
          static_cast<int>(
              m_air_settings->get_settings().openhd_uart_telemetry_baudrate),
          c_openhd_uart_baud}});
  ret.push_back(openhd::Setting{
      air::OPENHD_UART_TELEMETRY_FLOW_PARAM,
      openhd::IntSetting{
          static_cast<int>(m_air_settings->get_settings()
                               .openhd_uart_telemetry_flow_control),
          c_openhd_uart_flow}});
  ret.push_back(openhd::Setting{
      air::OPENHD_UART_PRIORITY_RC_PARAM,
      openhd::IntSetting{
          static_cast<int>(
              m_air_settings->get_settings().openhd_uart_priority_rc),
          c_openhd_uart_prio_rc}});
  ret.push_back(openhd::Setting{
      air::OPENHD_UART_PRIORITY_OHD_PARAM,
      openhd::IntSetting{
          static_cast<int>(
              m_air_settings->get_settings().openhd_uart_priority_openhd),
          c_openhd_uart_prio_ohd}});
  ret.push_back(openhd::Setting{
      air::OPENHD_UART_PRIORITY_FC_PARAM,
      openhd::IntSetting{
          static_cast<int>(
              m_air_settings->get_settings().openhd_uart_priority_fc),
          c_openhd_uart_prio_fc}});
  ret.push_back(openhd::Setting{
      air::TELEMETRY_LOGGING_PARAM,
      openhd::IntSetting{
          static_cast<int>(
              m_air_settings->get_settings().telemetry_logging_enabled),
          c_telemetry_logging}});
  ret.push_back(openhd::Setting{
      air::SBUS_OUT_ENABLE_PARAM,
      openhd::IntSetting{
          static_cast<int>(m_air_settings->get_settings().sbus_out_enabled),
          c_sbus_enable}});
  ret.push_back(openhd::Setting{
      air::SBUS_OUT_DEVICE_PARAM,
      openhd::StringSetting{
          m_air_settings->get_settings().sbus_uart_device, c_sbus_device}});
  ret.push_back(openhd::Setting{
      air::SBUS_OUT_RATE_PARAM,
      openhd::IntSetting{
          static_cast<int>(m_air_settings->get_settings().sbus_update_rate_hz),
          c_sbus_rate}});
  ret.push_back(openhd::create_read_only_int(
      "SIYI_ACTIVE",
      openhd::PluginManager::instance().is_plugin_loaded("siyi") ? 1 : 0));
  ret.push_back(openhd::create_read_only_int(
      "TOPOTEK_ACTIVE",
      openhd::PluginManager::instance().is_plugin_loaded("topotek") ? 1 : 0));
  ret.push_back(openhd::create_read_only_int(
      "CAM_CTRL_ACTIVE",
      (openhd::PluginManager::instance().is_plugin_loaded("siyi") ||
       openhd::PluginManager::instance().is_plugin_loaded("topotek"))
          ? 1
          : 0));
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
  if (m_ignoreSerial) {
    if (m_fc_serial) m_fc_serial->disable();
    return;
  }
  assert(m_air_settings);
  using namespace openhd::telemetry;
  const auto uart_linux_fd = serial_openhd_param_to_linux_fd(
      m_air_settings->get_settings().fc_uart_connection_type,
      SerialPortRole::Flight);
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

void AirTelemetry::setup_openhd_uart_telemetry() {
  if (m_ignoreSerial) {
    openhd::SecondaryTelemetryStatus::instance().set_configured("UART", false);
    if (m_openhd_uart_serial) m_openhd_uart_serial->disable();
    return;
  }
  if (!m_openhd_uart_serial) return;
  const auto& settings = m_air_settings->get_settings();
  if (!settings.openhd_uart_telemetry_enabled) {
    openhd::SecondaryTelemetryStatus::instance().set_configured("UART", false);
    m_openhd_uart_serial->disable();
    return;
  }
  const auto uart_linux_fd = serial_openhd_param_to_linux_fd(
      settings.openhd_uart_telemetry_connection, SerialPortRole::OpenHD);
  if (!uart_linux_fd.has_value()) {
    openhd::SecondaryTelemetryStatus::instance().set_configured("UART", false);
    m_openhd_uart_serial->disable();
    return;
  }
  SerialEndpoint::HWOptions options{};
  options.linux_filename = uart_linux_fd.value();
  options.baud_rate = settings.openhd_uart_telemetry_baudrate;
  options.flow_control = settings.openhd_uart_telemetry_flow_control;
  options.enable_reading = true;
  openhd::SecondaryTelemetryStatus::instance().set_configured("UART", true);
  m_openhd_uart_serial->configure(
      options, "openhd_uart",
      [this](const std::vector<MavlinkMessage> messages) {
        if (!messages.empty()) {
          openhd::SecondaryTelemetryStatus::instance().note_received("UART");
        }
        auto filtered = m_uart_deduplicator.filter_and_mark(messages);
        if (!filtered.empty()) {
          this->on_messages_ground_unit(filtered);
        }
      });
}

void AirTelemetry::setup_sbus_output() {
  if (m_ignoreSerial) {
    if (m_sbus_output) m_sbus_output->configure(SbusOutput::Options{});
    return;
  }
  if (!m_sbus_output) return;
  const auto& settings = m_air_settings->get_settings();
  SbusOutput::Options options{};
  options.enabled = settings.sbus_out_enabled;
  options.device = OHDUtil::str_equal(settings.sbus_uart_device, "DEFAULT")
                       ? "/dev/Sbus"
                       : settings.sbus_uart_device;
  options.update_rate_hz = settings.sbus_update_rate_hz;
  m_sbus_output->configure(options);
}

void AirTelemetry::configure_openhd_uart_telemetry(
    const std::optional<std::string>& device_path) {
  if (!device_path.has_value()) {
    return;
  }
  if (m_ignoreSerial) {
    m_console->info("Ignoring OpenHD UART telemetry CLI override because serial setup is disabled");
    return;
  }
  m_console->info("CLI override for OpenHD UART telemetry: {}",
                  device_path.value());
  m_air_settings->unsafe_get_settings().openhd_uart_telemetry_connection =
      device_path.value();
  m_air_settings->unsafe_get_settings().openhd_uart_telemetry_enabled = true;
  m_air_settings->persist();
  setup_openhd_uart_telemetry();
}

void AirTelemetry::set_link_handle(std::shared_ptr<OHDLink> link) {
  m_wb_endpoint = std::make_unique<WBEndpoint>(link, "wb_tx");
  m_wb_endpoint->registerCallback([this](std::vector<MavlinkMessage> messages) {
    auto filtered = m_uart_deduplicator.filter_and_mark(messages);
    if (!filtered.empty()) {
      on_messages_ground_unit(filtered);
    }
  });
}
