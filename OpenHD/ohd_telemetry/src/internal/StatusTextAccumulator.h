//
// Created by consti10 on 06.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_STATUSTEXT_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_STATUSTEXT_H_

#include <queue>

#include "HelperSources/SocketHelper.hpp"
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
  // Get all the currently buffered messages, removes the returned messages from the message queue.
  // Thread-safe
  std::vector<openhd::log::udp::LogMessage> get_messages(){
    std::vector<openhd::log::udp::LogMessage> ret;
    std::lock_guard<std::mutex> guard(bufferedLogMessagesLock);
    while (!bufferedLogMessages.empty()) {
      const auto msg = bufferedLogMessages.front();
      if(!msg.hasNullTerminator()){
        m_console->warn("Somethings wrong with the log message");
      }else{
        ret.push_back(msg);
      }
      bufferedLogMessages.pop();
    }
    return ret;
  }
  void manually_add_message(const std::string& text){
    openhd::log::udp::LogMessage msg{};
    msg.level=static_cast<uint8_t>(openhd::log::udp::STATUS_LEVEL::DEBUG);
    std::strncpy((char *)msg.message,text.c_str(),50);
    processLogMessage(msg);
  }
  static void convert(mavlink_message_t& mavlink_message,const openhd::log::udp::LogMessage& msg,uint8_t sys_id,uint8_t comp_id){
    mavlink_statustext_t mavlink_statustext;
    mavlink_statustext.id=0;
    mavlink_statustext.chunk_seq=0;
    mavlink_statustext.severity=msg.level;
    std::memcpy(&mavlink_statustext.text,msg.message,50);
    mavlink_msg_statustext_encode(sys_id,comp_id,&mavlink_message,&mavlink_statustext);
  }
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
