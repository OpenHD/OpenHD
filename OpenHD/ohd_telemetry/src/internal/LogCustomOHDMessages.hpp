//
// Created by consti10 on 24.04.22.
// Every OHD mavlink telemetry message should have a verbose loggin method
//

#ifndef XMAVLINKSERVICE_LOGCUSTOMOHDMESSAGES_H
#define XMAVLINKSERVICE_LOGCUSTOMOHDMESSAGES_H

#include "../mav_include.h"
#include "../mav_helper.h"
#include "openhd-spdlog.hpp"
#include <iostream>
#include <cassert>

/**
 * Helper for converting ohd custom messages to mavlink and printing them to stdout
 */
namespace LogCustomOHDMessages {

static void logOnboardComputerStatus(const mavlink_onboard_computer_status_t& decoded){
  std::stringstream ss;
  ss<<"MAVLINK_MSG_ID_ONBOARD_COMPUTER_STATUS: cpu_usage:"<<(int)decoded.cpu_cores[0]<<" temp:"<<(int)decoded.temperature_core[0];
  openhd::log::get_default()->debug(ss.str());
}

static void logLogMessage(const mavlink_message_t &msg) {
  assert(msg.msgid == MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE);
  mavlink_openhd_log_message_t decoded;
  mavlink_msg_openhd_log_message_decode(&msg, &decoded);
  std::stringstream ss;
  ss << "LOG:" << decoded.severity << ":" << decoded.text;
  openhd::log::get_default()->debug(ss.str());
}

static void logOpenHDMessages(const std::vector<MavlinkMessage> &msges) {
  for (const auto &msg: msges) {
    if (msg.m.msgid == MAVLINK_MSG_ID_ONBOARD_COMPUTER_STATUS) {
      mavlink_onboard_computer_status_t decoded;
      mavlink_msg_onboard_computer_status_decode(&msg.m,&decoded);
      logOnboardComputerStatus(decoded);
    } else if (msg.m.msgid == MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE) {
      logLogMessage(msg.m);
    } else {
      openhd::log::get_default()->debug("unknown ohd msg with msgid:{}",msg.m.msgid);
    }
  }
}
}

#endif //XMAVLINKSERVICE_LOGCUSTOMOHDMESSAGES_H
