//
// Created by consti10 on 15.04.22.
//

#ifndef XMAVLINKSERVICE_MAV_HELPER_H
#define XMAVLINKSERVICE_MAV_HELPER_H

#include <chrono>
#include <iostream>
#include <sstream>

#include "mav_include.h"
#include "openhd_spdlog.h"

namespace MExampleMessage {
// mostly from
// https://github.com/mavlink/mavlink/blob/master/examples/linux/mavlink_udp.c
static uint64_t microsSinceEpoch() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}
static MavlinkMessage heartbeat(const int sys_id = 255, const int comp_id = 0) {
  MavlinkMessage msg{};
  mavlink_msg_heartbeat_pack(sys_id, comp_id, &msg.m, MAV_TYPE_HELICOPTER,
                             MAV_AUTOPILOT_GENERIC, MAV_MODE_GUIDED_ARMED, 0,
                             MAV_STATE_ACTIVE);
  return msg;
}
static MavlinkMessage position(const int sys_id = 255, const int comp_id = 0) {
  MavlinkMessage msg{};
  float position[6] = {};
  mavlink_msg_local_position_ned_pack(
      sys_id, comp_id, &msg.m, microsSinceEpoch(), position[0], position[1],
      position[2], position[3], position[4], position[5]);
  return msg;
}
static MavlinkMessage attitude(const int sys_id = 255, const int comp_id = 0) {
  MavlinkMessage msg{};
  mavlink_msg_attitude_pack(sys_id, comp_id, &msg.m, microsSinceEpoch(), 1.2,
                            1.7, 3.14, 0.01, 0.02, 0.03);
  return msg;
}
}  // namespace MExampleMessage

namespace OHDMessages {
/**
 * Both the air and ground service announce their existence with regular
 * heartbeats one can easily determine from which component the heartbeat comes
 * by the corresponding sys id
 * TODO: should we also consider the component id ? Right now, we don't really
 * seperate by components yet.
 * @param isAir if true, the sys_id (source) equals OHD air id, otherwise OHD
 * ground id.
 */
static MavlinkMessage createHeartbeat(const int sys_id, const int comp_id) {
  MavlinkMessage heartbeat;
  mavlink_msg_heartbeat_pack(sys_id, comp_id, &heartbeat.m, MAV_TYPE_GENERIC,
                             MAV_AUTOPILOT_GENERIC, MAV_MODE_GUIDED_ARMED, 0,
                             MAV_STATE_ACTIVE);
  return heartbeat;
}

}  // namespace OHDMessages

namespace MavlinkHelpers {

static std::string mavlink_status_to_string(
    const mavlink_status_t& mavlink_status) {
  std::stringstream ss;
  ss << "MavlinkStatus{";
  ss << "msg_received:" << (int)mavlink_status.msg_received << ",";
  ss << "buffer_overrun:" << (int)mavlink_status.buffer_overrun << ",";
  ss << "parse_error:" << (int)mavlink_status.parse_error << ",";
  ss << "packet_idx:" << (int)mavlink_status.packet_idx << ",";
  ss << "current_rx_seq:" << (int)mavlink_status.current_rx_seq << ",";
  ss << "current_tx_seq:" << (int)mavlink_status.current_tx_seq << ",";
  ss << "packet_rx_success_count:"
     << (int)mavlink_status.packet_rx_success_count << ",";
  ss << " packet_rx_drop_count:" << (int)mavlink_status.packet_rx_drop_count
     << ",";
  // ss<<":"<<(int)mavlink_status.<<",";
  return ss.str();
}

static std::string raw_content(const uint8_t* data, int data_len) {
  std::stringstream ss;
  ss << "[";
  for (int i = 0; i < data_len; i++) {
    ss << (int)data[i] << ",";
  }
  ss << "]";
  return ss.str();
}

static void parseAndLog(const uint8_t* data, int data_len,
                        const uint8_t m_mavlink_channel,
                        mavlink_status_t& receiveMavlinkStatus) {
  int nMessages = 0;
  mavlink_message_t msg;
  for (int i = 0; i < data_len; i++) {
    uint8_t res = mavlink_parse_char(m_mavlink_channel, (uint8_t)data[i], &msg,
                                     &receiveMavlinkStatus);
    if (res) {
      MavlinkMessage message{msg};
      // debugMavlinkMessage(message.m,"XYZ");
      nMessages++;
    }
  }
  std::cout << "parseAndLog:"
            << " N messages:" << nMessages << "\n";
}

static void debugMavlinkPingMessage(const mavlink_message_t msg) {
  mavlink_ping_t ping;
  mavlink_msg_ping_decode(&msg, &ping);
  std::stringstream ss;
  ss << "Ping["
     << "sys_id:" << (int)msg.sysid << " comp_id:" << (int)msg.compid
     << " seq:" << ping.seq << " target_system:" << (int)ping.target_system
     << " target_component:" << (int)ping.target_component << "]\n";
  std::cout << ss.str();
}

}  // namespace MavlinkHelpers

static void debugMavlinkMessage(const mavlink_message_t& msg, const char* TAG) {
  printf("%s message with ID %d, sequence: %d from component %d of system %d\n",
         TAG, (int)msg.msgid, msg.seq, msg.compid, msg.sysid);
}
static void debugMavlinkMessages(const std::vector<MavlinkMessage>& messages,
                                 const char* TAG) {
  for (const auto& msg : messages) {
    debugMavlinkMessage(msg.m, TAG);
  }
  std::cout << std::endl;
}

static void lululululu() {
  // mavlink_msg_param_request_list_pack
  // mavlink_msg_param_ext_request_list_pack
}

static MavlinkMessage rc_channels_override_from_array(
    const int sys_id, const int comp_id,
    const std::array<uint16_t, 18>& rc_data, const uint8_t target_system,
    const uint8_t target_component) {
  MavlinkMessage ret{};
  mavlink_rc_channels_override_t mavlink_rc_channels_override;
  mavlink_rc_channels_override.target_system = target_system;
  mavlink_rc_channels_override.target_component = target_component;
  mavlink_rc_channels_override.chan1_raw = rc_data[0];
  mavlink_rc_channels_override.chan2_raw = rc_data[1];
  mavlink_rc_channels_override.chan3_raw = rc_data[2];
  mavlink_rc_channels_override.chan4_raw = rc_data[3];
  mavlink_rc_channels_override.chan5_raw = rc_data[4];
  mavlink_rc_channels_override.chan6_raw = rc_data[5];
  mavlink_rc_channels_override.chan7_raw = rc_data[6];
  mavlink_rc_channels_override.chan8_raw = rc_data[7];
  mavlink_rc_channels_override.chan9_raw = rc_data[8];
  mavlink_rc_channels_override.chan10_raw = rc_data[9];
  mavlink_rc_channels_override.chan11_raw = rc_data[10];
  mavlink_rc_channels_override.chan12_raw = rc_data[11];
  mavlink_rc_channels_override.chan13_raw = rc_data[12];
  mavlink_rc_channels_override.chan14_raw = rc_data[13];
  mavlink_rc_channels_override.chan15_raw = rc_data[14];
  mavlink_rc_channels_override.chan16_raw = rc_data[15];
  mavlink_rc_channels_override.chan17_raw = rc_data[16];
  mavlink_rc_channels_override.chan18_raw = rc_data[17];
  mavlink_msg_rc_channels_override_encode(sys_id, comp_id, &ret.m,
                                          &mavlink_rc_channels_override);
  return ret;
}

static std::array<int, 18> mavlink_msg_rc_channels_to_array(
    const mavlink_rc_channels_t& parsedMsg) {
  std::array<int, 18> ret{};
  ret[0] = parsedMsg.chan1_raw;
  ret[1] = parsedMsg.chan2_raw;
  ret[2] = parsedMsg.chan3_raw;
  ret[3] = parsedMsg.chan4_raw;
  ret[4] = parsedMsg.chan5_raw;
  ret[5] = parsedMsg.chan6_raw;
  ret[6] = parsedMsg.chan7_raw;
  ret[7] = parsedMsg.chan8_raw;
  ret[8] = parsedMsg.chan9_raw;
  ret[9] = parsedMsg.chan10_raw;
  ret[10] = parsedMsg.chan11_raw;
  ret[11] = parsedMsg.chan12_raw;
  ret[12] = parsedMsg.chan13_raw;
  ret[13] = parsedMsg.chan14_raw;
  ret[14] = parsedMsg.chan15_raw;
  ret[15] = parsedMsg.chan16_raw;
  ret[16] = parsedMsg.chan17_raw;
  ret[17] = parsedMsg.chan18_raw;
  return ret;
}

static std::array<int, 18> mavlink_msg_rc_channels_raw_to_array(
    const mavlink_rc_channels_raw_t& parsedMsg) {
  std::array<int, 18> ret{};
  ret[0] = parsedMsg.chan1_raw;
  ret[1] = parsedMsg.chan2_raw;
  ret[2] = parsedMsg.chan3_raw;
  ret[3] = parsedMsg.chan4_raw;
  ret[4] = parsedMsg.chan5_raw;
  ret[5] = parsedMsg.chan6_raw;
  ret[6] = parsedMsg.chan7_raw;
  ret[7] = parsedMsg.chan8_raw;
  // Has only 8 channels
  return ret;
}
// Optimize message routing: If a message has a target sys / comp id, only send
// it to the specified sys / comp id
struct MTarget {
  uint16_t sys_id;
  uint16_t comp_id;
  bool has_target() const { return sys_id != 0; }
};
static MTarget get_target_from_message_if_available(
    const mavlink_message_t& msg) {
  // if(msg.incompat_flags)
  if (msg.msgid == MAVLINK_MSG_ID_COMMAND_LONG) {
    mavlink_command_long_t command;
    mavlink_msg_command_long_decode(&msg, &command);
    return {command.target_system, command.target_component};
  }
  if (msg.msgid == MAVLINK_MSG_ID_PARAM_EXT_SET) {
    mavlink_param_ext_set_t tmp;
    mavlink_msg_param_ext_set_decode(&msg, &tmp);
    return {tmp.target_system, tmp.target_component};
  }
  if (msg.msgid == MAVLINK_MSG_ID_PARAM_EXT_REQUEST_READ) {
    mavlink_param_ext_request_read_t tmp;
    mavlink_msg_param_ext_request_read_decode(&msg, &tmp);
    return {tmp.target_system, tmp.target_component};
  }
  if (msg.msgid == MAVLINK_MSG_ID_PARAM_EXT_REQUEST_LIST) {
    mavlink_param_ext_request_list_t tmp;
    mavlink_msg_param_ext_request_list_decode(&msg, &tmp);
    return {tmp.target_system, tmp.target_component};
  }
  // 0 == broadcast
  return {0, 0};
}

// The optimization we want to achieve is to "consume" messages with a given
// target sys and comp id that matches the given target sys id (e.g. the openhd
// ground unit) instead of forwarding them to all endpoints, e.g. the FC (those
// other endpoints won't do anything with the given message anyway) pictorial
// speaking, we fill 2 buckets (generic and local_only) with the given
// message(s)
static std::tuple<std::vector<MavlinkMessage>, std::vector<MavlinkMessage>>
split_into_generic_and_local_only(const std::vector<MavlinkMessage>& messages,
                                  uint16_t local_target_sys_id) {
  std::vector<MavlinkMessage> generic{};
  generic.reserve(messages.size());
  std::vector<MavlinkMessage> local_only{};
  for (const auto& msg : messages) {
    const auto target = get_target_from_message_if_available(msg.m);
    if (target.has_target() && target.sys_id == local_target_sys_id) {
      // openhd::log::get_default()->debug("local only message");
      local_only.push_back(msg);
    } else {
      generic.push_back(msg);
    }
  }
  return {generic, local_only};
}

// Return all messages where the source sys id matches the given sys id
static std::vector<MavlinkMessage> filter_by_source_sys_id(
    const std::vector<MavlinkMessage>& messages, uint16_t source_sys_id) {
  std::vector<MavlinkMessage> ret;
  for (const auto& msg : messages) {
    if (msg.m.sysid == source_sys_id) {
      ret.push_back(msg);
    }
  }
  return ret;
}

#endif  // XMAVLINKSERVICE_MAV_HELPER_H
