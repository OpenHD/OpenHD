//
// Created by consti10 on 19.04.22.
//

#include "OHDMainComponent.h"

#include <iostream>
#include <openhd_global_constants.hpp>
#include <utility>

#include "OHDLinkStatisticsHelper.h"
#include "OnboardComputerStatusProvider.h"
#include "openhd_config.h"
#include "openhd_reboot_util.h"
#include "openhd_spdlog_include.h"
#include "openhd_util_time.h"

OHDMainComponent::OHDMainComponent(uint8_t parent_sys_id, bool runsOnAir)
    : RUNS_ON_AIR(runsOnAir),
      MavlinkComponent(parent_sys_id, MAV_COMP_ID_ONBOARD_COMPUTER),
      m_heartbeats_interval(RUNS_ON_AIR ? std::chrono::milliseconds(500)
                                        : std::chrono::milliseconds(200)),
      m_onboard_computer_status_interval(RUNS_ON_AIR
                                             ? std::chrono::milliseconds(500)
                                             : std::chrono::milliseconds(200)),
      m_wb_stats_interval(RUNS_ON_AIR ? std::chrono::milliseconds(500)
                                      : std::chrono::milliseconds(200)) {
  m_console = openhd::log::create_or_get("t_main_c");
  assert(m_console);
  m_onboard_computer_status_provider =
      std::make_unique<OnboardComputerStatusProvider>(true);
  const auto config = openhd::load_config();
  if (!RUNS_ON_AIR && config.GEN_ENABLE_LAST_KNOWN_POSITION) {
    m_last_known_position = std::make_unique<LastKnowPosition>();
  }
}

OHDMainComponent::~OHDMainComponent() {}

std::vector<MavlinkMessage> OHDMainComponent::generate_mavlink_messages() {
  // m_console->debug("InternalTelemetry::generate_mavlink_messages()");
  std::vector<MavlinkMessage> ret;
  auto opt_heartbeat = create_heartbeat_if_needed();
  if (opt_heartbeat.has_value()) {
    ret.push_back(opt_heartbeat.value());
  }
  auto broadcast_stats = create_broadcast_stats_if_needed();
  OHDUtil::vec_append(ret, broadcast_stats);
  // ret.push_back(generateOpenHDVersion());
  // ret.push_back(MExampleMessage::position(mSysId,mCompId));
  //_status_text_accumulator.manually_add_message(RUNS_ON_AIR ? "HelloAir" :
  //"HelloGround");
  const auto logs = generateLogMessages();
  OHDUtil::vec_append(ret, logs);
  // OHDUtil::vec_append(ret, perform_time_synchronisation());
  return ret;
}

std::vector<MavlinkMessage> OHDMainComponent::process_mavlink_messages(
    std::vector<MavlinkMessage> messages) {
  std::vector<MavlinkMessage> ret{};
  for (const auto& msg : messages) {
    switch (msg.m.msgid) {  // NOLINT(cppcoreguidelines-narrowing-conversions)
      case MAVLINK_MSG_ID_TIMESYNC: {
        // makes ping obsolete
        auto response = handle_timesync_message(msg);
        if (response.has_value()) {
          ret.push_back(response.value());
        }
      } break;
      case MAVLINK_MSG_ID_COMMAND_LONG: {
        mavlink_command_long_t command;
        mavlink_msg_command_long_decode(&msg.m, &command);
        if (command.target_system == m_sys_id &&
            command.target_component == m_comp_id) {
          process_command_self(command, msg.m.sysid, msg.m.compid, ret);
        }
        // TODO have an ack response.
      } break;
      case MAVLINK_MSG_ID_GLOBAL_POSITION_INT: {
        // Writes last known position to file(s) for crash recovery
        mavlink_global_position_int_t global_position_int;
        mavlink_msg_global_position_int_decode(&msg.m, &global_position_int);
        const double lat =
            static_cast<double>(global_position_int.lat) / 10000000.0;
        const double lon =
            static_cast<double>(global_position_int.lon) / 10000000.0;
        const double alt = global_position_int.relative_alt / 1000.0;
        if (m_last_known_position) {
          m_last_known_position->on_new_position(lat, lon, alt);
        }
      } break;
      default:
        break;
    }
  }
  return ret;
}

std::vector<MavlinkMessage> OHDMainComponent::generate_mav_wb_stats() {
  // m_console->debug("OHDMainComponent::generate_mav_wb_stats");
  const auto latest_stats =
      openhd::LinkActionHandler::instance().get_link_stats();
  if (!latest_stats.ready) {
    // Not yet updated
    return {};
  }
  std::vector<MavlinkMessage> ret;
  if (RUNS_ON_AIR != latest_stats.is_air) {
    // Happens when wb hasn't updated the stats the first time yet
    m_console->warn("Mismatch air/ground");
    return ret;
  }
  // stats for all the wifi card(s)
  int card_index = 0;
  for (const auto& card_stats : latest_stats.cards) {
    if (!card_stats.NON_MAVLINK_CARD_ACTIVE) {
      // skip non active cards
      continue;
    }
    MavlinkMessage msg = openhd::LinkStatisticsHelper::pack_card(
        m_sys_id, m_comp_id, card_index, card_stats);
    card_index++;
    ret.push_back(msg);
  }
  ret.push_back(openhd::LinkStatisticsHelper::pack_link_general(
      m_sys_id, m_comp_id, latest_stats.monitor_mode_link));
  ret.push_back(openhd::LinkStatisticsHelper::pack_tele(
      m_sys_id, m_comp_id, latest_stats.telemetry));
  if (RUNS_ON_AIR) {
    for (const auto& stats : latest_stats.stats_wb_video_air) {
      ret.push_back(openhd::LinkStatisticsHelper::pack_vid_air(
          m_sys_id, m_comp_id, stats));
    }
    ret.push_back(openhd::LinkStatisticsHelper::pack_vid_air_fec_performance(
        m_sys_id, m_comp_id, latest_stats.air_fec_performance));
  } else {
    for (const auto& ground_video : latest_stats.stats_wb_video_ground) {
      ret.push_back(openhd::LinkStatisticsHelper::pack_vid_gnd(
          m_sys_id, m_comp_id, ground_video));
    }
    ret.push_back(openhd::LinkStatisticsHelper::pack_vid_gnd_fec_performance(
        m_sys_id, m_comp_id, latest_stats.gnd_fec_performance));
    ret.push_back(
        openhd::LinkStatisticsHelper::
            pack_mavlink_openhd_wifbroadcast_gnd_operating_mode(
                m_sys_id, m_comp_id, latest_stats.gnd_operating_mode));
    if (openhd::LinkActionHandler::instance().wb_get_supported_channels !=
        nullptr) {
      auto channels =
          openhd::LinkActionHandler::instance().wb_get_supported_channels();
      ret.push_back(openhd::LinkStatisticsHelper::
                        generate_msg_openhd_wifibroadcast_supported_channels(
                            m_sys_id, m_comp_id, channels));
    }
    auto progress_x =
        openhd::LinkActionHandler::instance().get_analyze_results();
    for (auto& progress : progress_x) {
      ret.push_back(
          openhd::LinkStatisticsHelper::generate_msg_analyze_channels_progress(
              m_sys_id, m_comp_id, progress));
    }
    auto progress_y =
        openhd::LinkActionHandler::instance().get_scan_channels_progress();
    for (auto& progress : progress_y) {
      ret.push_back(
          openhd::LinkStatisticsHelper::generate_msg_scan_channels_progress(
              m_sys_id, m_comp_id, progress));
    }
  }
  return ret;
}

static mavlink_message_t create_mavlink_log_message(
    const openhd::log::MavlinkLogMessage& msg, uint8_t sys_id,
    uint8_t comp_id) {
  mavlink_message_t ret;
  mavlink_statustext_t mavlink_statustext;
  mavlink_statustext.id = 0;
  mavlink_statustext.chunk_seq = 0;
  mavlink_statustext.severity = msg.level;
  std::memcpy(&mavlink_statustext.text, msg.message, 50);
  mavlink_msg_statustext_encode(sys_id, comp_id, &ret, &mavlink_statustext);
  return ret;
}

std::vector<MavlinkMessage> OHDMainComponent::generateLogMessages() {
  // return m_status_text_accumulator->get_mavlink_messages(m_sys_id,m_comp_id);
  auto messages =
      openhd::log::MavlinkLogMessageBuffer::instance().dequeue_log_messages();
  std::vector<MavlinkMessage> ret;
  ret.reserve(messages.size());
  for (auto& message : messages) {
    ret.push_back(MavlinkMessage{
        create_mavlink_log_message(message, m_sys_id, m_comp_id)});
  }
  return ret;
}

MavlinkMessage OHDMainComponent::generate_ohd_version() const {
  MavlinkMessage msg;
  mavlink_msg_openhd_version_message_pack(
      m_sys_id, m_comp_id, &msg.m, openhd::MAJOR_VERSION, openhd::MINOR_VERSION,
      openhd::PATCH_VERSION, openhd::RELEASE_TYPE, 0);
  return msg;
}

MavlinkMessage OHDMainComponent::ack_command(const uint8_t source_sys_id,
                                             const uint8_t source_comp_id,
                                             uint16_t command_id,
                                             bool success) {
  MavlinkMessage ret{};
  const auto result = success ? MAV_RESULT_ACCEPTED : MAV_RESULT_UNSUPPORTED;
  mavlink_msg_command_ack_pack(m_sys_id, m_comp_id, &ret.m, command_id, result,
                               255, 0, source_sys_id, source_comp_id);
  return ret;
}

std::optional<MavlinkMessage> OHDMainComponent::handle_timesync_message(
    const MavlinkMessage& message) {
  const auto msg = message.m;
  assert(msg.msgid == MAVLINK_MSG_ID_TIMESYNC);
  mavlink_timesync_t tsync;
  mavlink_msg_timesync_decode(&msg, &tsync);
  // m_console->debug(
  //     "Got timesync message target_system:{} target_component:{} "
  //     "ts1{} tc1{}",
  //     tsync.target_system, tsync.target_component, tsync.ts1, tsync.tc1);
  if (tsync.tc1 == 0) {
    // request, pack response
    mavlink_timesync_t rsync{};
    rsync.tc1 = get_time_microseconds() * 1000;
    rsync.ts1 = tsync.ts1;
    rsync.target_system = msg.sysid;
    rsync.target_component = msg.compid;
    mavlink_message_t response_message;
    mavlink_msg_timesync_encode(m_sys_id, m_comp_id, &response_message, &rsync);
    return MavlinkMessage{response_message};
  } else if (tsync.target_system == m_sys_id &&
             tsync.target_component == m_comp_id &&
             msg.sysid == OHD_SYS_ID_AIR) {
    if (m_last_timesync_out_us == tsync.ts1) {
      handle_timesync_response_self(tsync);
    }
  } else {
    m_console->debug(
        "Cannot handle timesync message target_system:{} target_component:{} "
        "ts1{} tc1{}",
        tsync.target_system, tsync.target_component, tsync.ts1, tsync.tc1);
  }
  return std::nullopt;
}

void OHDMainComponent::check_fc_messages_for_actions(
    const std::vector<MavlinkMessage>& messages) {
  for (const auto& msg : messages) {
    if (msg.m.msgid == MAVLINK_MSG_ID_HEARTBEAT) {
      // This is mainly for the user to debug
      if (RUNS_ON_AIR) {
        m_air_fc_sys_id = msg.m.sysid;
      }
      // We filter a bit more to not accidentally set armed state
      if (((msg.m.sysid == OHD_SYS_ID_FC) ||
           (msg.m.sysid == OHD_SYS_ID_FC_BETAFLIGHT))) {
        mavlink_heartbeat_t heartbeat;
        mavlink_msg_heartbeat_decode(&msg.m, &heartbeat);
        const auto mode = (MAV_MODE_FLAG)heartbeat.base_mode;
        const bool armed = (mode & MAV_MODE_FLAG_SAFETY_ARMED);
        openhd::ArmingStateHelper::instance().update_arming_state_if_changed(
            armed);
      }
    }
    // We only change the mcs on the air unit (since downlink is the only thing
    // that requires 'higher' bandwidth)
    if (RUNS_ON_AIR) {
      if ((msg.m.sysid == OHD_SYS_ID_FC) ||
          (msg.m.sysid == OHD_SYS_ID_FC_BETAFLIGHT)) {
        if (msg.m.msgid == MAVLINK_MSG_ID_RC_CHANNELS) {
          mavlink_rc_channels_t rc_channels;
          mavlink_msg_rc_channels_decode(&msg.m, &rc_channels);
          const auto tmp = mavlink_msg_rc_channels_to_array(rc_channels);
          openhd::FCRcChannelsHelper::instance().update_rc_channels(tmp);
        } else if (msg.m.msgid == MAVLINK_MSG_ID_RC_CHANNELS_RAW) {
          mavlink_rc_channels_raw_t rc_channels;
          mavlink_msg_rc_channels_raw_decode(&msg.m, &rc_channels);
          const auto tmp = mavlink_msg_rc_channels_raw_to_array(rc_channels);
          openhd::FCRcChannelsHelper::instance().update_rc_channels(tmp);
        }
      }
    }
  }
}

std::optional<MavlinkMessage> OHDMainComponent::create_heartbeat_if_needed() {
  const auto elapsed = std::chrono::steady_clock::now() - m_last_heartbeat;
  if (elapsed >= m_heartbeats_interval) {
    m_last_heartbeat = std::chrono::steady_clock::now();
    return MavlinkComponent::create_heartbeat();
  }
  return std::nullopt;
}

std::vector<MavlinkMessage>
OHDMainComponent::create_broadcast_stats_if_needed() {
  std::vector<MavlinkMessage> ret;
  const auto now = std::chrono::steady_clock::now();
  const auto elapsed_onboard_computer_status = now - m_last_onboard_computer;
  if (elapsed_onboard_computer_status > m_onboard_computer_status_interval) {
    m_last_onboard_computer = now;
    std::optional<OnboardComputerStatusProvider::ExtraUartInfo> opt_uart_info =
        std::nullopt;
    if (RUNS_ON_AIR) {
      opt_uart_info = OnboardComputerStatusProvider::ExtraUartInfo{
          m_air_fc_sys_id.load(), 0};
    }
    OnboardComputerStatusProvider::ExtraUartInfo extra{m_air_fc_sys_id};
    ret.push_back(m_onboard_computer_status_provider
                      ->get_current_status_as_mavlink_message(
                          m_sys_id, m_comp_id, opt_uart_info));
    ret.push_back(openhd::LinkStatisticsHelper::generate_sys_status1(
        m_sys_id, m_comp_id, openhd::LinkActionHandler::instance()));
  }
  {
    const auto elapsed_version = now - m_last_version_message_tp;
    if (elapsed_version > m_version_message_interval) {
      m_last_version_message_tp = now;
      ret.push_back(generate_ohd_version());
    }
  }
  const auto elapsed_wb = now - m_last_wb_stats;
  if (elapsed_wb > m_wb_stats_interval) {
    m_last_wb_stats = now;
    OHDUtil::vec_append(ret, generate_mav_wb_stats());
    if (RUNS_ON_AIR) {
      auto cam_stats1 = openhd::LinkActionHandler::instance().get_cam_info(0);
      auto cam_stats2 = openhd::LinkActionHandler::instance().get_cam_info(1);
      // NOTE: We use the comp id of primary / secondary camera here, since even
      // though we are not the camera itself, We send the broadcast message(s)
      // for it
      if (cam_stats1.active) {
        ret.push_back(openhd::LinkStatisticsHelper::pack_camera_stats(
            m_sys_id, MAV_COMP_ID_CAMERA, cam_stats1));
      }
      if (cam_stats2.active) {
        ret.push_back(openhd::LinkStatisticsHelper::pack_camera_stats(
            m_sys_id, MAV_COMP_ID_CAMERA2, cam_stats2));
      }
    }
  }
  return ret;
}

void OHDMainComponent::process_command_self(
    const mavlink_command_long_t& command, int source_sys_id,
    int source_comp_id, std::vector<MavlinkMessage>& message_buffer) {
  assert(command.target_system == m_sys_id);
  assert(command.target_component == m_comp_id);
  m_console->debug("Got MAVLINK_MSG_ID_COMMAND_LONG: {} {}", command.command,
                   static_cast<uint32_t>(command.param1));
  if (command.command == MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN) {
    // https://mavlink.io/en/messages/common.html#MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN
    m_console->debug("Got shutdown command");
    // we are a companion computer, so we use param2 to get the actual action
    const auto action_for_companion = command.param2;
    if (action_for_companion > 0) {
      message_buffer.push_back(
          ack_command(source_sys_id, source_comp_id, command.command));
      const bool shutdownOnly = action_for_companion == 2;
      openhd::reboot::handle_power_command_async(std::chrono::seconds(1),
                                                 shutdownOnly);
    }
    // dirty, we don't have a custom message for that yet
    if (command.param3 == 1) {
      message_buffer.push_back(
          ack_command(source_sys_id, source_comp_id, command.command));
      m_console->debug("Unimplemented");
    }

  } else if (command.command == MAV_CMD_REQUEST_MESSAGE) {
    const auto requested_message_id = static_cast<uint32_t>(command.param1);
    m_console->debug("Someone requested a specific message: {}",
                     requested_message_id);
    if (requested_message_id == MAVLINK_MSG_ID_OPENHD_VERSION_MESSAGE) {
      m_console->info("Sent OpenHD version");
      message_buffer.push_back(generate_ohd_version());
    } else if (requested_message_id ==
               MAVLINK_MSG_ID_OPENHD_WIFBROADCAST_SUPPORTED_CHANNELS) {
      m_console->debug("Supported channels requested");
      if (openhd::LinkActionHandler::instance().wb_get_supported_channels !=
          nullptr) {
        auto channels =
            openhd::LinkActionHandler::instance().wb_get_supported_channels();
        message_buffer.push_back(
            openhd::LinkStatisticsHelper::
                generate_msg_openhd_wifibroadcast_supported_channels(
                    m_sys_id, m_comp_id, channels));
        m_console->info("Sent supported channels");
      } else {
        m_console->warn("Cannot get channels from wb (no handler");
      }
    } else {
      m_console->info("Message {} request not supported", requested_message_id);
    }
  } else if (command.command == OPENHD_CMD_INITIATE_CHANNEL_SEARCH) {
    if (RUNS_ON_AIR) {
      m_console->debug("Scan channels is only a feature for ground unit");
      return;
    } else {
      const auto channels_to_scan = static_cast<uint32_t>(command.param1);
      m_console->debug("OPENHD_CMD_INITIATE_CHANNEL_SEARCH {}",
                       channels_to_scan);
      bool success = false;
      if (channels_to_scan == 0 || channels_to_scan == 1 ||
          channels_to_scan == 2) {
        if (openhd::LinkActionHandler::instance().wb_cmd_scan_channels) {
          openhd::LinkActionHandler::ScanChannelsParam scanChannelsParam{};
          scanChannelsParam.channels_to_scan = channels_to_scan;
          success = openhd::LinkActionHandler::instance().wb_cmd_scan_channels(
              scanChannelsParam);
        }
        m_console->debug("OPENHD_CMD_INITIATE_CHANNEL_SEARCH result: {}",
                         success);
      }
      message_buffer.push_back(
          ack_command(source_sys_id, source_comp_id, command.command, success));
    }
  } else if (command.command == OPENHD_CMD_INITIATE_CHANNEL_ANALYZE) {
    if (RUNS_ON_AIR) {
      m_console->debug("Scan channels is only a feature for ground unit");
      return;
    } else {
      const int channels_to_scan = static_cast<uint32_t>(command.param1);
      m_console->debug("OPENHD_CMD_INITIATE_CHANNEL_ANALYZE {}",
                       channels_to_scan);
      bool success = false;
      if (openhd::LinkActionHandler::instance().wb_cmd_analyze_channels &&
              channels_to_scan == 0 ||
          channels_to_scan == 1 || channels_to_scan == 2) {
        success = openhd::LinkActionHandler::instance().wb_cmd_analyze_channels(
            channels_to_scan);
      }
      message_buffer.push_back(
          ack_command(source_sys_id, source_comp_id, command.command, success));
    }
  } else {
    m_console->debug("Unknown command {}", command.command);
  }
}

std::vector<MavlinkMessage> OHDMainComponent::perform_time_synchronisation() {
  if (RUNS_ON_AIR) {
    // We only ever ask the air for a timesync
    return {};
  }
  if (m_has_synced_time) return {};
  const auto elapsed =
      std::chrono::steady_clock::now() - m_last_timesync_request;
  if (elapsed > std::chrono::milliseconds(1000)) {
    mavlink_timesync_t timesync{};
    timesync.target_system = OHD_SYS_ID_AIR;
    timesync.target_component = MAV_COMP_ID_ONBOARD_COMPUTER;
    timesync.tc1 = 0;
    // Ardupilot seems to use us
    m_last_timesync_out_us = get_time_microseconds();
    timesync.ts1 = m_last_timesync_out_us;
    MavlinkMessage msg;
    mavlink_msg_timesync_encode(m_sys_id, m_comp_id, &msg.m, &timesync);
    m_last_timesync_request = std::chrono::steady_clock::now();
    m_console->debug("Sending timesync");
    return {msg};
  }
  return {};
}

void OHDMainComponent::handle_timesync_response_self(
    const mavlink_timesync_t& tsync) {
  const auto now_us = get_time_microseconds();
  const auto round_trip_time_us = now_us - tsync.ts1;
  const auto local_time_offset = now_us + tsync.tc1;
  m_console->debug(
      "handle_timesync_response_self, round trip:{}, local_time_offset:{}us",
      openhd::util::time_readable_ns(round_trip_time_us * 1000),
      local_time_offset);
  if (round_trip_time_us >= 0 && round_trip_time_us < 5 * 1000) {
    const auto local_time_offset_adjusted =
        now_us - round_trip_time_us / 2 - tsync.tc1;
    m_good_timesync_offset_count++;
    m_good_timesync_offset_total += local_time_offset_adjusted;
    if (m_good_timesync_offset_count > 10) {
      int64_t average_offset =
          m_good_timesync_offset_total / m_good_timesync_offset_count;
      openhd::util::store_air_unit_time_offset_us(average_offset);
      m_console->debug("Synced time, offset {}", average_offset);
      m_has_synced_time = true;
    }
  }
}
