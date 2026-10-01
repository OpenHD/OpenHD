#include <stdexcept>

#include "mavsdk_temporary/mavlink_parameter_set.h"

static void require(bool condition) {
  if (!condition) throw std::runtime_error("Parameter reapply regression");
}

int main() {
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
