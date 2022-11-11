//
// Created by consti10 on 11.11.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_SPDLOG_TELE_SINK_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_SPDLOG_TELE_SINK_H_

#include "spdlog/sinks/base_sink.h"
#include <iostream>
#include <utility>
#include <deque>

// bridge between any logger and telemetry
// We send logs higher or equal to the warning log level out via mavlink
namespace openhd::loggers::sink{

class TelemetryForwardSink : public spdlog::sinks::base_sink<std::mutex>{
 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override{

    // log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
    // msg.raw contains pre formatted log

    // If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg, formatted);
    const auto msg_string=fmt::to_string(formatted);
    //std::cout << "["<<msg_string<<"]";
    if(msg.level>=spdlog::level::warn){
      enqueue_message(level_spdlog_to_mavlink(msg.level),msg_string);
    }
  }
  void flush_() override{
    //std::cout << std::flush;
  }
 public:
  // these match the mavlink SEVERITY_LEVEL enum, but this code should not depend on
  // the mavlink headers
  // See https://mavlink.io/en/messages/common.html#MAV_SEVERITY
  enum class STATUS_LEVEL {
    EMERGENCY = 0,
    ALERT,
    CRITICAL,
    ERROR,
    WARNING,
    INFO,
    NOTICE,
    DEBUG
  };
  static STATUS_LEVEL level_spdlog_to_mavlink(const spdlog::level::level_enum& level){
    switch (level) {
      case spdlog::level::trace:
        return STATUS_LEVEL::DEBUG;
        break;
      case spdlog::level::debug:
        return STATUS_LEVEL::DEBUG;
        break;
      case spdlog::level::info:
        return STATUS_LEVEL::INFO;
        break;
      case spdlog::level::warn:
        return STATUS_LEVEL::WARNING;
      case spdlog::level::err:
        return STATUS_LEVEL::ERROR;
        break;
      case spdlog::level::critical:
        return STATUS_LEVEL::CRITICAL;
        break;
      case spdlog::level::off:
      case spdlog::level::n_levels:
      default:
        break;
    }
    return STATUS_LEVEL::DEBUG;
  }
  class StoredLogMessage{
   public:
    explicit StoredLogMessage(STATUS_LEVEL level,std::string message):
        m_level(level),m_message(std::move(message)){
    }
    STATUS_LEVEL m_level;
    std::string m_message;
  };
 private:
  // Called by the logger(s), thread-safe
  void enqueue_message(STATUS_LEVEL level,const std::string& message){
    std::lock_guard<std::mutex> guard(m_stored_messages_mutex);
    if(m_stored_messages.size()>=MAX_N_QUEUED_MESSAGES-1){
      m_stored_messages.pop_back();
    }
    m_stored_messages.push_back(std::make_shared<StoredLogMessage>(level,message));
  }
 public:
  // Called by the telemetry thread, thread-safe
  std::vector<std::shared_ptr<StoredLogMessage>> dequeue_messages(){
    std::lock_guard<std::mutex> guard(m_stored_messages_mutex);
    std::vector<std::shared_ptr<StoredLogMessage>> ret;
    for(int i=0;i<m_stored_messages.size();i++){
      ret.push_back(m_stored_messages.back());
      m_stored_messages.pop_back();
    }
    return ret;
  }
 private:
  static constexpr auto MAX_N_QUEUED_MESSAGES=10;
  std::mutex m_stored_messages_mutex;
  std::deque<std::shared_ptr<StoredLogMessage>> m_stored_messages{};
};

// Whichever thread calls this first instantiates the class
// Thread-safe
static std::shared_ptr<TelemetryForwardSink> instance(){
  static std::shared_ptr<TelemetryForwardSink> ret=std::make_shared<TelemetryForwardSink>();
  return ret;
}

}

#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_SPDLOG_TELE_SINK_H_
