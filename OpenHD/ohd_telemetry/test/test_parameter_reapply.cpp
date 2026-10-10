#include <stdexcept>

#include "mavsdk_temporary/mavlink_parameter_set.h"
#include "mavsdk_temporary/XMavlinkParamProvider.h"

// Normally supplied by the executable's runtime path configuration.
const char* getVideoPath() { return "/tmp/"; }

static void test_sensor_mode_readback() {
  XMavlinkParamProvider provider(101, 100);
  std::string sensor = "1920x1080@30";
  int sensor_writes = 0;
  provider.add_param({"RESOLUTION_FPS", openhd::StringSetting{
      "1920x1080@30", [&](std::string, std::string) {
        sensor = "0x0@0";
        return true;
      }}});
  provider.add_param({"SENSOR_MODE", openhd::StringSetting{
      sensor, [&](std::string, std::string value) {
        ++sensor_writes;
        sensor = value;
        return true;
      }, [&]() { return sensor; }}});
  provider.set_ready();
  MavlinkMessage resolution, read, override_mode;
  mavlink_msg_param_ext_set_pack(255, 190, &resolution.m, 101, 100,
                                "RESOLUTION_FPS", "1280x720@60",
                                MAV_PARAM_EXT_TYPE_CUSTOM);
  mavlink_msg_param_ext_request_read_pack(255, 190, &read.m, 101, 100,
                                         "SENSOR_MODE", -1);
  mavlink_msg_param_ext_set_pack(255, 190, &override_mode.m, 101, 100,
                                "SENSOR_MODE", "1920x1080@30",
                                MAV_PARAM_EXT_TYPE_CUSTOM);
  // The read and write arrive in the same batch as the resolution change.
  const auto responses = provider.process_mavlink_messages(
      {resolution, read, override_mode});
  bool read_automatic = false;
  for (const auto& response : responses) {
    if (response.m.msgid == MAVLINK_MSG_ID_PARAM_EXT_VALUE) {
      mavlink_param_ext_value_t value{};
      mavlink_msg_param_ext_value_decode(&response.m, &value);
      if (std::string(value.param_id) == "SENSOR_MODE")
        read_automatic = std::string(value.param_value) == "0x0@0";
    }
  }
  if (!read_automatic || sensor_writes != 1 || sensor != "1920x1080@30")
    throw std::runtime_error("Sensor mode readback regression");
}

static void require(bool condition) {
  if (!condition) throw std::runtime_error("Parameter reapply regression");
}

int main() {
  test_sensor_mode_readback();
  using mavsdk::MavlinkParameterSet;
  using Result = MavlinkParameterSet::UpdateExistingParamResult;
  mavsdk::ParamValue eight;
  eight.set<int32_t>(8);
  mavsdk::ParamValue thirteen;
  thirteen.set<int32_t>(13);
  MavlinkParameterSet parameters;
  int encoder_kbits = 13750;
  int calls = 0;
  require(parameters.add_new_parameter(
      "BITRATE_MBITS", eight,
      [&](std::string, mavsdk::ParamValue requested) {
        ++calls;
        encoder_kbits = requested.get<int32_t>() * 1000;
        return true;
      }, true));
  require(parameters.update_existing_parameter("BITRATE_MBITS", eight) ==
          Result::SUCCESS);
  require(calls == 1 && encoder_kbits == 8000);

  encoder_kbits = 13750;
  require(parameters.update_existing_parameter("BITRATE_MBITS", thirteen,
                                                false) == Result::SUCCESS);
  require(calls == 1 && encoder_kbits == 13750);
  require(parameters.update_existing_parameter("BITRATE_MBITS", thirteen) ==
          Result::SUCCESS);
  require(calls == 2 && encoder_kbits == 13000);

  require(parameters.add_new_parameter(
      "CAMERA_TYPE", eight,
      [&](std::string, mavsdk::ParamValue) { ++calls; return true; }));
  require(parameters.update_existing_parameter("CAMERA_TYPE", eight) ==
          Result::NO_CHANGE);
  require(calls == 2);

  require(parameters.add_new_parameter(
      "REJECTED", eight,
      [](std::string, mavsdk::ParamValue) { return false; }, true));
  require(parameters.update_existing_parameter("REJECTED", eight) ==
          Result::REJECTED);
}
