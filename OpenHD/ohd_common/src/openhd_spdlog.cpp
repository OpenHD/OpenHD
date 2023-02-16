//
// Created by consti10 on 14.02.23.
//
#include "openhd_spdlog.h"
#include "openhd_udp_log.h"

//
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
//
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <iostream>
#include <mutex>
#include <utility>

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

std::shared_ptr<spdlog::logger> openhd::log::create_or_get(
    const std::string& logger_name) {
  static std::mutex logger_mutex2{};
  std::lock_guard<std::mutex> guard(logger_mutex2);
  auto ret = spdlog::get(logger_name);
  if (ret == nullptr) {
    auto created = spdlog::stdout_color_mt(logger_name);
    assert(created);
    created->set_level(spdlog::level::debug);
    // Add the sink that sends out warning or higher via UDP
    created->sinks().push_back(std::make_shared<openhd::log::sink::UdpTelemetrySink>());
    // This is for debugging for "where a fmt exception occurred"
    spdlog::set_error_handler([](const std::string &msg) {
      std::cerr<<msg<<"\n;";
    });
    return created;
  }
  return ret;
}

std::shared_ptr<spdlog::logger> openhd::log::get_default() {
  return create_or_get("default");
}
