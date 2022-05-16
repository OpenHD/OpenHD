//
// Created by consti10 on 24.04.22.
// Every OHD mavlink telemetry message should have a verbose loggin method
//

#ifndef XMAVLINKSERVICE_LOGCUSTOMOHDMESSAGES_H
#define XMAVLINKSERVICE_LOGCUSTOMOHDMESSAGES_H

#include "../mav_include.h"
#include "../mav_helper.h"
#include <iostream>

namespace LogCustomOHDMessages {
static void logSystem(const mavlink_message_t &msg) {
  assert(msg.msgid == MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY);
  mavlink_openhd_system_telemetry_t decoded;
  mavlink_msg_openhd_system_telemetry_decode(&msg, &decoded);
  std::stringstream ss;
  ss << "MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY:cpuload:" << (int)decoded.cpuload << "temp:" << (int)decoded.temperature
	 << "\n";
  std::cout << ss.str();
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
static void logMessages(const std::vector<MavlinkMessage> &msges) {
  for (const auto &msg: msges) {
	if (msg.m.msgid == MAVLINK_MSG_ID_OPENHD_SYSTEM_TELEMETRY) {
	  logSystem(msg.m);
	} else if (msg.m.msgid == MAVLINK_MSG_ID_OPENHD_WIFIBROADCAST_STATISTICS) {
	  logWifiBroadcast(msg.m);
	} else if (msg.m.msgid == MAVLINK_MSG_ID_OPENHD_LOG_MESSAGE) {
	  logLogMessage(msg.m);
	} else {
	  std::cerr << "unknown ohd msg\n";
	}
  }
}
}

#endif //XMAVLINKSERVICE_LOGCUSTOMOHDMESSAGES_H
