#include "internal/FcFirmwareDetector.h"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

using openhd::telemetry::FcFirmware;
using Detector = openhd::telemetry::FcFirmwareDetector;
#define CHECK(value) do { if (!(value)) throw std::runtime_error(#value); } while (0)

mavlink_message_t heartbeat(int autopilot, int system = 1, int component = 1,
                            int type = MAV_TYPE_QUADROTOR) {
  mavlink_message_t message{};
  mavlink_msg_heartbeat_pack(system, component, &message, type, autopilot, 0, 0, MAV_STATE_STANDBY);
  return message;
}
mavlink_message_t version(uint32_t value, int system = 1, int component = 1) {
  mavlink_message_t message{}; mavlink_autopilot_version_t payload{};
  payload.flight_sw_version = value;
  mavlink_msg_autopilot_version_encode(system, component, &message, &payload);
  return message;
}
mavlink_message_t parameter(bool empty, int system = 1, int component = 1) {
  mavlink_message_t message{}; mavlink_param_value_t payload{};
  if (!empty) {
    std::memcpy(payload.param_id, "SYSID_THISMAV", 13);
    payload.param_count = 300; payload.param_index = 0; payload.param_type = MAV_PARAM_TYPE_REAL32;
  }
  mavlink_msg_param_value_encode(system, component, &message, &payload);
  return message;
}
Detector make_detector(unsigned retries = 2) { return Detector({10, 10, 1000, retries}); }
void request_version(Detector& d) {
  const auto requests = d.tick(0);
  CHECK(requests.size() == 1);
  CHECK(requests[0].sysid == 255 && requests[0].compid == 190);
  mavlink_command_long_t value{};
  mavlink_msg_command_long_decode(&requests[0], &value);
  CHECK(value.command == 512 && value.param1 == 148 && value.target_system == d.result().system && value.target_component == d.result().component);
}
void request_parameters(Detector& d, int64_t now = 1) {
  const auto requests = d.tick(now);
  CHECK(requests.size() == 1 && requests[0].msgid == 21);
  CHECK(requests[0].sysid == 255 && requests[0].compid == 190);
  mavlink_param_request_list_t value{};
  mavlink_msg_param_request_list_decode(&requests[0], &value);
  CHECK(value.target_system == d.result().system && value.target_component == d.result().component);
}
int main() {
  try {
    // INAV 8/9 signatures: generic and spoofed ArduPilot heartbeat. No version
    // is inferred from the signature, and the family remains "likely".
    for (int autopilot : {0, 3}) {
      auto d = make_detector(); d.receive(heartbeat(autopilot), 0); request_version(d);
      d.tick(10, 101, 191); d.tick(20, 101, 191); request_parameters(d, 30);
      d.receive(parameter(true), 31);
      CHECK(d.result().firmware == FcFirmware::INAV && d.result().confidence == "likely" && d.result().version == 0);
      CHECK(d.tick(100, 101, 191).empty());
    }
    // Old Betaflight without version response must remain unknown.
    {
      auto d = make_detector(); d.receive(heartbeat(0, 0, 200), 0); request_version(d);
      for (int t = 10; t <= 100; t += 10) d.tick(t, 101, 191);
      CHECK(d.result().firmware == FcFirmware::UNKNOWN);
      CHECK(d.tick(200, 101, 191).empty());
    }
    // Newer Betaflight: plausible version is candidate evidence, never proof.
    for (uint32_t v : {0x040500ffu, 0x08030100u}) {
      auto d = make_detector(); d.receive(heartbeat(0), 0); request_version(d);
      d.receive(version(v), 1); request_parameters(d);
      CHECK(d.result().firmware == FcFirmware::BETAFLIGHT && d.result().confidence == "candidate");
      d.tick(11, 101, 191); d.tick(21, 101, 191); d.tick(31, 101, 191);
      CHECK(d.result().firmware == FcFirmware::BETAFLIGHT && d.result().confidence == "candidate");
    }
    {
      auto d = make_detector(); d.receive(heartbeat(3), 0); request_version(d);
      d.receive(version(0x040601ff), 1); request_parameters(d); d.receive(parameter(false), 2);
      CHECK(d.result().firmware == FcFirmware::ARDUPILOT);
    }
    {
      auto d = make_detector(); d.receive(heartbeat(12), 0);
      CHECK(d.result().firmware == FcFirmware::PX4); request_version(d);
      d.receive(version(0x010f00ff), 1); CHECK(d.tick(100, 101, 191).empty());
    }
    // Wrong sender, unsolicited responses, GCS and companion heartbeat ignored.
    {
      auto d = make_detector(); d.receive(heartbeat(3, 1, 1, MAV_TYPE_GCS), 0);
      d.receive(heartbeat(0, 1, MAV_COMP_ID_ONBOARD_COMPUTER, MAV_TYPE_ONBOARD_CONTROLLER), 0);
      CHECK(d.result().system == -1);
      d.receive(heartbeat(0, 1, MAV_COMP_ID_SYSTEM_CONTROL), 0);
      d.receive(parameter(true, 1, MAV_COMP_ID_SYSTEM_CONTROL), 0);
      CHECK(d.result().firmware == FcFirmware::UNKNOWN); request_version(d);
      d.receive(version(0x040500ff, 2, MAV_COMP_ID_SYSTEM_CONTROL), 1);
      d.receive(version(0x040500ff), 1);
      CHECK(d.result().version == 0);
      d.tick(10, 101, 191); d.tick(20, 101, 191); request_parameters(d, 30);
      d.receive(parameter(true, 1, MAV_COMP_ID_SYSTEM_CONTROL), 31);
      CHECK(d.result().firmware == FcFirmware::INAV);
      d.receive(heartbeat(0, 1, MAV_COMP_ID_SYSTEM_CONTROL, MAV_TYPE_GCS), 900);
      d.tick(1031, 101, 191);
      CHECK(d.result().system == -1); // GCS traffic must not prolong the FC cache.
    }
    // A delayed first heartbeat must not discard positive UART probe evidence.
    {
      auto d = make_detector();
      mavlink_message_t rc{}; mavlink_rc_channels_t payload{};
      mavlink_msg_rc_channels_encode(1, 1, &rc, &payload);
      d.receive(rc, 0);
      d.tick(10); d.tick(20); d.tick(30); d.tick(40);
      d.receive(parameter(true), 41);
      CHECK(d.result().firmware == FcFirmware::INAV);
      d.receive(heartbeat(0), 42);
      CHECK(d.result().firmware == FcFirmware::INAV && d.result().generation == 0);
      CHECK(d.tick(43).empty());
    }
    // Retry cap, packet loss, delayed and repeated responses, cache expiry.
    {
      auto d = make_detector();
      mavlink_message_t rc{}; mavlink_rc_channels_t payload{};
      mavlink_msg_rc_channels_encode(1, 191, &rc, &payload);
      d.receive(rc, 0); CHECK(d.result().system == -1); // companion isn't an FC
      mavlink_msg_rc_channels_encode(1, 1, &rc, &payload);
      d.receive(rc, 0); CHECK(d.result().firmware == FcFirmware::UNKNOWN);
      CHECK(d.tick(0, 101, 191).empty());
      CHECK(d.tick(10, 101, 191).size() == 1);
      d.receive(version(0x040601ff), 11);
      CHECK(d.result().firmware == FcFirmware::UNKNOWN); // no generic heartbeat
      request_parameters(d, 11); d.receive(parameter(true), 12);
      CHECK(d.result().firmware == FcFirmware::INAV);
      auto silent = make_detector(); silent.receive(rc, 0);
      for (int t = 0; t <= 100; t += 10) silent.tick(t, 101, 191);
      CHECK(silent.result().firmware == FcFirmware::UNKNOWN); // absence proves nothing
    }
    {
      auto d = make_detector(99); d.receive(heartbeat(3), 0);
      int version_requests = 0, param_requests = 0;
      for (int t = 0; t < 100; t += 10) for (const auto& request : d.tick(t, 101, 191)) {
        if (request.msgid == 76) ++version_requests; else if (request.msgid == 21) ++param_requests;
      }
      CHECK(version_requests == 3 && param_requests == 3);
      CHECK(d.result().firmware == FcFirmware::UNKNOWN);
      d.receive(parameter(false), 100); d.receive(parameter(false), 110);
      CHECK(d.result().firmware == FcFirmware::ARDUPILOT && d.tick(200, 101, 191).empty());
      d.tick(1110, 101, 191); CHECK(d.result().system == -1 && d.result().firmware == FcFirmware::UNKNOWN);
      d.receive(heartbeat(12, 2), 1200); CHECK(d.result().firmware == FcFirmware::PX4);
    }
    // Empty, malformed, invalid versions and contradictory evidence.
    {
      auto d = make_detector(); d.receive(heartbeat(3), 0); request_version(d);
      // First version and parameter requests lost; second requests succeed.
      CHECK(d.tick(10, 101, 191).size() == 1);
      d.receive(version(0x040601ff), 11); request_parameters(d, 11);
      CHECK(d.tick(21, 101, 191).size() == 1);
      d.receive(parameter(false), 22);
      CHECK(d.result().firmware == FcFirmware::ARDUPILOT);
    }
    for (auto v : {0u, UINT32_MAX}) {
      auto d = make_detector(); d.receive(heartbeat(0), 0); request_version(d); d.receive(version(v), 1);
      CHECK(d.result().firmware == FcFirmware::UNKNOWN); request_parameters(d);
      auto malformed = parameter(false); mavlink_param_value_t p{}; mavlink_msg_param_value_decode(&malformed, &p);
      p.param_count = 0; mavlink_msg_param_value_encode(1, 1, &malformed, &p); d.receive(malformed, 2);
      CHECK(d.result().firmware == FcFirmware::UNKNOWN);
      d.receive(parameter(true), 3); d.receive(parameter(false), 4);
      CHECK(d.result().firmware == FcFirmware::UNKNOWN);
    }
    // Uptime restart, identity change and firmware-version change invalidate cache.
    {
      auto d = make_detector(); d.receive(heartbeat(3), 0); request_version(d); d.receive(version(0x040601ff), 1);
      request_parameters(d); d.receive(parameter(false), 2);
      auto boot = [](uint32_t time) { mavlink_message_t m{}; mavlink_system_time_t p{}; p.time_boot_ms = time; mavlink_msg_system_time_encode(1, 1, &m, &p); return m; };
      d.receive(boot(10000), 3); d.receive(boot(100), 4);
      CHECK(d.result().firmware == FcFirmware::UNKNOWN && d.result().generation == 1);
      d.tick(4, 101, 191); d.receive(version(0x040700ff), 5);
      CHECK(d.result().version == 0x040700ff);
      d.receive(version(0x040800ff), 6); CHECK(d.result().generation == 2);
      d.receive(heartbeat(12), 7); CHECK(d.result().firmware == FcFirmware::PX4 && d.result().generation == 3);
    }
    {
      auto d = make_detector(); d.receive(heartbeat(12), 0);
      auto attitude = [](uint32_t time) { mavlink_message_t m{}; mavlink_attitude_t p{}; p.time_boot_ms = time; mavlink_msg_attitude_encode(1, 1, &m, &p); return m; };
      d.receive(attitude(10000), 1); d.receive(attitude(9999), 2);
      CHECK(d.result().generation == 0);
      d.receive(attitude(0xfffffff0), 3); d.receive(attitude(30), 4);
      CHECK(d.result().generation == 0);
      auto unsupported = make_detector(); unsupported.receive(heartbeat(8), 0);
      CHECK(unsupported.result().firmware == FcFirmware::UNKNOWN && unsupported.tick(0, 101, 191).empty());
    }
    std::cout << "FC firmware detection: all scenarios passed\n";
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return EXIT_FAILURE; }
}
