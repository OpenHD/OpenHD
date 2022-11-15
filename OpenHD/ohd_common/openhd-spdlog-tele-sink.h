//
// Created by consti10 on 11.11.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_SPDLOG_TELE_SINK_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_SPDLOG_TELE_SINK_H_

#include "spdlog/sinks/base_sink.h"
#include <iostream>
#include <utility>
#include <deque>
#include <openhd-udp-log.h>

// bridge between any logger and telemetry
// We send logs higher or equal to the warning log level out via udp
// such that they can be picked up by the telemetry module
namespace openhd::log::sink{

class UdpTelemetrySink : public spdlog::sinks::base_sink<std::mutex>{
 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override{
    // log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
    // msg.raw contains pre formatted log
    // If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
    if(msg.level>=spdlog::level::warn){
      // We do not use the formatter here, since we are limited by 50 chars (and the level, for example, is embedded already but not as a string).
      //spdlog::memory_buf_t formatted;
      //spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg, formatted);
      const auto msg_string=fmt::to_string(msg.payload);
      const auto level=openhd::log::udp::level_spdlog_to_mavlink(msg.level);
      openhd::log::udp::ohd_log(level,msg_string);
      //std::cout << "["<<msg_string<<"]";
    }
  }
  void flush_() override{
    //std::cout << std::flush;
  }
};

}

#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_SPDLOG_TELE_SINK_H_
