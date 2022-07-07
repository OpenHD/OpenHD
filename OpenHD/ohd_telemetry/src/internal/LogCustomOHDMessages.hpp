//
// Created by consti10 on 24.04.22.
// Every OHD mavlink telemetry message should have a verbose loggin method
//

#ifndef XMAVLINKSERVICE_LOGCUSTOMOHDMESSAGES_H
#define XMAVLINKSERVICE_LOGCUSTOMOHDMESSAGES_H

#include "../mav_include.h"
#include "../mav_helper.h"
#include <iostream>
#include <cassert>

/**
 * Helper for converting ohd custom messages to mavlink and printing them to stdout
 */
namespace LogCustomOHDMessages {

static void logOnboardComputerStatus(const mavlink_message_t &msg){
  assert(msg.msgid == MAVLINK_MSG_ID_ONBOARD_COMPUTER_STATUS);
  mavlink_onboard_computer_status_t decoded;
  mavlink_msg_onboard_computer_status_decode(&msg,&decoded);
  std::stringstream ss;
  ss<<"MAVLINK_MSG_ID_ONBOARD_COMPUTER_STATUS: cpu_usage:"<<decoded.cpu_cores[0]<<" temp:"<<decoded.temperature_core[0]<<"\n";
  std::cout<<ss.str();
}

static void logWifiBroadcast(const mavlink_message_t &msg) {
  assert(msg.msgid == MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS);
  mavlink_openhd_wifibroadcast_statistics_t decoded;
  mavlink_msg_openhd_wifibroadcast_statistics_decode(&msg, &decoded);
  std::stringstream ss;
  ss << "MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS:count_p_all:" << (int)decoded.count_p_all << "count_p_lost:"
	 << (int)decoded.count_p_lost << "\n";
  std::cout << ss.str();
}
static void logLogMessage(const mavlink_message_t &msg) {
  assert(msg.msgid == MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE);
  mavlink_openhd_log_message_t decoded;
  mavlink_msg_openhd_log_message_decode(&msg, &decoded);
  std::stringstream ss;
  ss << "LOG:" << decoded.severity << ":" << decoded.text << "\n";
  std::cout << ss.str();
}

static void logOpenHDMessages(const std::vector<MavlinkMessage> &msges) {
  for (const auto &msg: msges) {
	if (msg.m.msgid == MAVLINK_MSG_ID_ONBOARD_COMPUTER_STATUS) {
	  logOnboardComputerStatus(msg.m);
	} else if (msg.m.msgid == MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS) {
	  logWifiBroadcast(msg.m);
	} else if (msg.m.msgid == MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE) {
	  logLogMessage(msg.m);
	} else {
	  std::cerr << "unknown ohd msg with msgid:"<<msg.m.msgid<<"\n";
	}
  }
}
}

#endif //XMAVLINKSERVICE_LOGCUSTOMOHDMESSAGES_H
