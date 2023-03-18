//
// Created by consti10 on 06.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_STATUSTEXT_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_STATUSTEXT_H_

#include <queue>
// dirty, pull in header only
#include "../../../lib/wifibroadcast/src/HelperSources/SocketHelper.hpp"
#include "mav_include.h"
#include "openhd_spdlog.h"
#include "openhd_udp_log.h"

/**
 * Accumulates incoming udp log messages, such that they can be forwarded via
 * mavlink telemetry
 */
class StatusTextAccumulator{
 public:
  StatusTextAccumulator();
  ~StatusTextAccumulator();
  /**
   * Get all the currently buffered messages, removes the returned messages from the message queue.
   * Thread-safe
   */
  std::vector<openhd::log::udp::LogMessage> get_messages();
  /**
   * Similar to above, but returns "mavlink statustext messages"
   * @param limit: This is called in regular intervals, more than limit buffered messages are dropped
   */
   std::vector<MavlinkMessage> get_mavlink_messages(uint8_t sys_id,uint8_t comp_id,int limit=5);
  /**
   * For testing, manually add a message to the queue
   * Thread-safe
   */
  void manually_add_message(const std::string& text);
 private:
  // process the incoming log messages. This one is a bit dangerous, it must handle the character
  // limit imposed by mavlink and the null terminator
  void processLogMessageData(const uint8_t *data, std::size_t dataLen);
  // add a new message to the message queue.
  void processLogMessage(openhd::log::udp::LogMessage msg);
  // one thread writes the queue, another one reads the queue
  std::mutex bufferedLogMessagesLock;
  std::queue<openhd::log::udp::LogMessage> bufferedLogMessages{};
  std::shared_ptr<spdlog::logger> m_console;
  // here all the log messages are sent to - not in their mavlink form yet.
  std::unique_ptr<SocketHelper::UDPReceiver> m_log_messages_receiver;
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_STATUSTEXT_H_
