// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <openhd/mavlink.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <map>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

namespace openhd::telemetry {

enum class FcFirmware { UNKNOWN = 0, ARDUPILOT = 1, PX4 = 2, INAV = 3, BETAFLIGHT = 4 };

// Pure event-driven protocol state. The owner serializes access and supplies
// steady-clock milliseconds. No threads, sleeps, IO, or telemetry interception.
class FcFirmwareDetector {
 public:
  // INAV accepts these requests only from the conventional GCS endpoint.
  // Use the same identity as QOpenHD for these read-only discovery probes.
  static constexpr uint8_t probe_system_id = 255;
  static constexpr uint8_t probe_component_id = MAV_COMP_ID_MISSIONPLANNER;
  struct Config {
    int64_t version_timeout_ms = 1500;
    int64_t parameter_timeout_ms = 2000;
    int64_t disconnect_timeout_ms = 10000;
    unsigned retries = 2;
  };
  struct Result {
    FcFirmware firmware = FcFirmware::UNKNOWN;
    std::string confidence = "inconclusive";
    std::string evidence = "Waiting for FC heartbeat";
    uint32_t version = 0;
    int system = -1, component = -1;
    uint64_t generation = 0;
  };
  FcFirmwareDetector() : FcFirmwareDetector(Config{}) {}
  explicit FcFirmwareDetector(Config config) : config_(config) {
    config_.retries = std::min(2u, config_.retries);
    config_.version_timeout_ms = std::max<int64_t>(1, config_.version_timeout_ms);
    config_.parameter_timeout_ms = std::max<int64_t>(1, config_.parameter_timeout_ms);
    config_.disconnect_timeout_ms = std::max<int64_t>(1, config_.disconnect_timeout_ms);
  }
  static const char* name(FcFirmware type) {
    switch (type) {
      case FcFirmware::ARDUPILOT: return "ArduPilot";
      case FcFirmware::PX4: return "PX4";
      case FcFirmware::INAV: return "INAV";
      case FcFirmware::BETAFLIGHT: return "Betaflight";
      default: return "Unknown";
    }
  }
  static bool is_fc_heartbeat(const mavlink_message_t& msg) {
    if (msg.msgid != MAVLINK_MSG_ID_HEARTBEAT) return false;
    mavlink_heartbeat_t hb{};
    mavlink_msg_heartbeat_decode(&msg, &hb);
    if (hb.autopilot == MAV_AUTOPILOT_INVALID || hb.type == MAV_TYPE_GCS ||
        hb.type == MAV_TYPE_ONBOARD_CONTROLLER || hb.type == MAV_TYPE_CAMERA ||
        hb.type == MAV_TYPE_GIMBAL || hb.type == MAV_TYPE_ANTENNA_TRACKER)
      return false;
    return msg.compid == MAV_COMP_ID_AUTOPILOT1 || msg.compid == MAV_COMP_ID_SYSTEM_CONTROL ||
        (msg.sysid == 0 && msg.compid == 200); // Legacy BF endpoint; not firmware proof.
  }
  const Result& result() const { return result_; }
  bool matches(const mavlink_message_t& msg) const {
    return result_.system >= 0 && msg.sysid == static_cast<uint32_t>(result_.system) &&
           msg.compid == result_.component;
  }
  void reset(const std::string& reason = "FC disconnected") {
    const auto generation = result_.generation + 1;
    result_ = Result{};
    result_.generation = generation;
    result_.evidence = reason;
    stage_ = Stage::Idle;
    autopilot_ = MAV_AUTOPILOT_INVALID;
    version_requested_ = parameters_requested_ = real_parameters_ = empty_parameters_ = false;
    heartbeat_seen_ = false;
    uptime_.clear();
  }
  void receive(const mavlink_message_t& msg, int64_t now) {
    if (result_.system >= 0 && now - last_rx_ >= config_.disconnect_timeout_ms)
      reset();
    if (msg.msgid == MAVLINK_MSG_ID_HEARTBEAT && !is_fc_heartbeat(msg)) return;
    // Some deployed INAV configurations disable the stream group containing
    // HEARTBEAT while keeping RC/status telemetry active. On the configured
    // FC UART, use a known vehicle telemetry packet only to address bounded
    // read-only probes. It is NOT evidence for any firmware family.
    if (result_.system < 0 && is_fc_endpoint(msg) &&
        (msg.msgid == MAVLINK_MSG_ID_RC_CHANNELS || msg.msgid == MAVLINK_MSG_ID_RC_CHANNELS_RAW ||
         msg.msgid == MAVLINK_MSG_ID_ATTITUDE || msg.msgid == MAVLINK_MSG_ID_SYS_STATUS)) {
      result_.system = msg.sysid; result_.component = msg.compid;
      last_rx_ = now; stage_ = Stage::Version; attempts_ = 0;
      deadline_ = now + config_.version_timeout_ms;
      result_.evidence = "FC UART telemetry present; waiting for heartbeat or positive probe evidence";
    }
    if (is_fc_heartbeat(msg)) {
      mavlink_heartbeat_t hb{};
      mavlink_msg_heartbeat_decode(&msg, &hb);
      if (result_.system < 0) start(msg, hb, now);
      else if (matches(msg) && !heartbeat_seen_) {
        // A delayed FIRST heartbeat augments UART/probe evidence; it is not
        // a reboot or an identity change. Keep an already observed INAV reply.
        heartbeat_seen_ = true;
        autopilot_ = hb.autopilot; vehicle_type_ = hb.type;
        classify();
      }
      else if (matches(msg) && (hb.autopilot != autopilot_ || hb.type != vehicle_type_ ||
                 (hb.system_status == MAV_STATE_BOOT && last_state_ != MAV_STATE_BOOT))) {
        reset("Heartbeat identity changed or FC rebooted");
        start(msg, hb, now);
      }
      if (matches(msg)) last_state_ = hb.system_status;
    }
    if (!matches(msg)) return;
    last_rx_ = now;
    std::optional<uint32_t> boot;
    if (msg.msgid == MAVLINK_MSG_ID_SYSTEM_TIME) {
      mavlink_system_time_t value{}; mavlink_msg_system_time_decode(&msg, &value); boot = value.time_boot_ms;
    } else if (msg.msgid == MAVLINK_MSG_ID_ATTITUDE) {
      mavlink_attitude_t value{}; mavlink_msg_attitude_decode(&msg, &value); boot = value.time_boot_ms;
    } else if (msg.msgid == MAVLINK_MSG_ID_GLOBAL_POSITION_INT) {
      mavlink_global_position_int_t value{}; mavlink_msg_global_position_int_decode(&msg, &value); boot = value.time_boot_ms;
    }
    if (boot) {
      auto previous = uptime_.find(msg.msgid);
      // Compare within a message type, tolerate reordering, and allow uint32 wrap.
      if (previous != uptime_.end() && previous->second > *boot &&
          previous->second - *boot > 2000 && previous->second < 0xf0000000u) {
        restart(now, "FC uptime restarted");
      }
      uptime_[msg.msgid] = *boot;
    }
    if (msg.msgid == MAVLINK_MSG_ID_AUTOPILOT_VERSION && version_requested_) {
      mavlink_autopilot_version_t value{};
      mavlink_msg_autopilot_version_decode(&msg, &value);
      if (result_.version && result_.version != value.flight_sw_version) {
        restart(now, "Firmware version changed");
        version_requested_ = true;
      }
      result_.version = value.flight_sw_version;
      if (stage_ == Stage::Version && autopilot_ != MAV_AUTOPILOT_PX4) {
        stage_ = Stage::Parameters; attempts_ = 0; deadline_ = now;
      } else if (autopilot_ == MAV_AUTOPILOT_PX4) stage_ = Stage::Done;
      classify();
    }
    if (msg.msgid == MAVLINK_MSG_ID_PARAM_VALUE && parameters_requested_) {
      mavlink_param_value_t value{};
      mavlink_msg_param_value_decode(&msg, &value);
      bool empty_id = true;
      for (unsigned i = 0; i < sizeof(value.param_id); ++i) empty_id = empty_id && value.param_id[i] == 0;
      const bool empty = value.param_count == 0 && value.param_index == 0 && value.param_type == 0 && empty_id;
      const bool real = value.param_count > 0 && value.param_index < value.param_count &&
          value.param_type >= MAV_PARAM_TYPE_UINT8 && value.param_type <= MAV_PARAM_TYPE_REAL64 && value.param_id[0] != 0;
      if (empty) empty_parameters_ = true;
      if (real) real_parameters_ = true;
      if (empty || real) { stage_ = Stage::Done; classify(); }
    }
  }
  std::vector<mavlink_message_t> tick(int64_t now,
      uint8_t sender_system = probe_system_id,
      uint8_t sender_component = probe_component_id) {
    std::vector<mavlink_message_t> requests;
    if (result_.system < 0) return requests;
    if (now - last_rx_ >= config_.disconnect_timeout_ms) { reset(); return requests; }
    if (stage_ == Stage::Idle || stage_ == Stage::Done || now < deadline_) return requests;
    if (attempts_ >= 1 + config_.retries) {
      if (stage_ == Stage::Version && autopilot_ != MAV_AUTOPILOT_PX4) {
        stage_ = Stage::Parameters; attempts_ = 0;
      } else {
        stage_ = Stage::Done;
        classify();
        result_.evidence += "; responses missing/inconclusive (retry limit)";
        return requests;
      }
    }
    mavlink_message_t request{};
    if (stage_ == Stage::Version) {
      mavlink_msg_command_long_pack(sender_system, sender_component, &request,
          result_.system, result_.component, MAV_CMD_REQUEST_MESSAGE, 0,
          MAVLINK_MSG_ID_AUTOPILOT_VERSION, 0, 0, 0, 0, 0, 0);
      version_requested_ = true;
      deadline_ = now + config_.version_timeout_ms;
    } else {
      mavlink_msg_param_request_list_pack(sender_system, sender_component, &request,
          result_.system, result_.component);
      parameters_requested_ = true;
      deadline_ = now + config_.parameter_timeout_ms;
    }
    ++attempts_;
    requests.push_back(request);
    return requests;
  }

 private:
  enum class Stage { Idle, Version, Parameters, Done };
  static bool is_fc_endpoint(const mavlink_message_t& msg) {
    return msg.compid == MAV_COMP_ID_AUTOPILOT1 || msg.compid == MAV_COMP_ID_SYSTEM_CONTROL ||
        (msg.sysid == 0 && msg.compid == 200);
  }
  void start(const mavlink_message_t& msg, const mavlink_heartbeat_t& hb, int64_t now) {
    result_.system = msg.sysid; result_.component = msg.compid;
    autopilot_ = hb.autopilot; vehicle_type_ = hb.type; last_state_ = hb.system_status;
    heartbeat_seen_ = true;
    last_rx_ = now; stage_ = Stage::Version; attempts_ = 0; deadline_ = now;
    if (autopilot_ != MAV_AUTOPILOT_GENERIC && autopilot_ != MAV_AUTOPILOT_ARDUPILOTMEGA &&
        autopilot_ != MAV_AUTOPILOT_PX4) stage_ = Stage::Done;
    classify();
  }
  void restart(int64_t now, const std::string& reason) {
    const auto system = result_.system, component = result_.component;
    const auto autopilot = autopilot_, type = vehicle_type_, state = last_state_;
    const bool had_heartbeat = heartbeat_seen_;
    reset(reason);
    result_.system = system; result_.component = component;
    autopilot_ = autopilot; vehicle_type_ = type; last_state_ = state;
    heartbeat_seen_ = had_heartbeat;
    last_rx_ = now; stage_ = Stage::Version; attempts_ = 0; deadline_ = now;
    classify();
  }
  void classify() {
    result_.firmware = FcFirmware::UNKNOWN; result_.confidence = "inconclusive";
    if (autopilot_ == MAV_AUTOPILOT_PX4) {
      result_.firmware = FcFirmware::PX4; result_.confidence = "identified";
      result_.evidence = "PX4 autopilot heartbeat (12)";
    } else if (empty_parameters_ && !real_parameters_) {
      result_.firmware = FcFirmware::INAV; result_.confidence = "likely";
      result_.evidence = "Empty PARAM_VALUE signature; likely INAV";
    } else if (empty_parameters_ && real_parameters_) {
      result_.evidence = "Conflicting empty and real parameter responses";
    } else if (autopilot_ == MAV_AUTOPILOT_ARDUPILOTMEGA && real_parameters_) {
      result_.firmware = FcFirmware::ARDUPILOT; result_.confidence = "identified";
      result_.evidence = "ArduPilot heartbeat (3) and real parameter list";
    } else if (heartbeat_seen_ && autopilot_ == MAV_AUTOPILOT_GENERIC && valid_version(result_.version)) {
      result_.firmware = FcFirmware::BETAFLIGHT; result_.confidence = "candidate";
      result_.evidence = "Generic heartbeat and valid AUTOPILOT_VERSION; Betaflight candidate only";
    } else {
      result_.evidence = !heartbeat_seen_ ? "FC UART telemetry without heartbeat; firmware still ambiguous" : autopilot_ == MAV_AUTOPILOT_ARDUPILOTMEGA
          ? "ArduPilot heartbeat (3); firmware still ambiguous"
          : "Generic/unknown heartbeat; firmware still ambiguous";
    }
  }
  static bool valid_version(uint32_t v) {
    // MAVLink encodes major.minor.patch.type. Version 0 and erased flash are
    // not a version. Do not require a particular Betaflight major or release type.
    return v != 0 && v != UINT32_MAX && ((v >> 24) & 255) != 0;
  }
  Config config_;
  Result result_;
  Stage stage_ = Stage::Idle;
  uint8_t autopilot_ = MAV_AUTOPILOT_INVALID, vehicle_type_ = 0, last_state_ = 0;
  int64_t last_rx_ = 0, deadline_ = 0;
  unsigned attempts_ = 0;
  bool version_requested_ = false, parameters_requested_ = false;
  bool real_parameters_ = false, empty_parameters_ = false;
  bool heartbeat_seen_ = false;
  std::map<uint32_t, uint32_t> uptime_;
};
}  // namespace openhd::telemetry
