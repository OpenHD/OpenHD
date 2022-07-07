//
// Created by consti10 on 07.07.22.
//

#include "XMavlinkParamProvider.h"

#include "mavlink_parameter_receiver.h"

std::vector<MavlinkMessage> XMavlinkParamProvider::process_mavlink_message(
    const MavlinkMessage& msg) {
  return std::vector<MavlinkMessage>();
}

std::vector<MavlinkMessage> XMavlinkParamProvider::generate_mavlink_messages() {
  return std::vector<MavlinkMessage>();
}