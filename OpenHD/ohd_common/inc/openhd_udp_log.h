//
// Created by consti10 on 11.11.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_UDP_LOG_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_UDP_LOG_H_


#include "openhd_global_constants.hpp"
#include "spdlog/common.h"

/**
 * This provides convenient methods to send log messages from any service
 * running on the air or ground unit to a final output deice, for example
 * QOpenHD. As an example, this makes it possible to view the logs from the air
 * unit on QOpenHD without connecting a display to the air unit. The general way
 * this works is simple: The log messages is sent to a specific udp port on
 * localhost and then picked up by the telemetry service, which converts it to
 * mavlink and forwards it accordingly.
 */
namespace openhd::log::udp{

struct LogMessage {
  uint8_t level;
  uint8_t message[50];
  // returns true if the message has a proper null terminator.
  // Only in this case we can treat the raw string array as a string.
  [[nodiscard]] bool hasNullTerminator() const {
    // check if the string has a null-terminator
    bool nullTerminatorFound = false;
    for (const auto &i: message) {
      if (i == '\0') {
        nullTerminatorFound = true;
        break;
      }
    }
    return nullTerminatorFound;
  }
  std::string msg_as_string()const{
    if(!hasNullTerminator())return "Null term missing";
    return "TDOO";
  }
} __attribute__((packed));

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

STATUS_LEVEL level_spdlog_to_mavlink(const spdlog::level::level_enum& level);

/**
 * Send a log message out via udp to localhost, it wll be picked up by the telemetry service.
 * @param message the log message to send, has to have a valid null terminator.
 */
void sendLocalLogMessageUDP(const LogMessage & message);

/*
 * Messages sent here will end up in the telemetry microservice, where they will be packed up and sent through
 * mavlink for storage and review by qopenhd, the boot screen system, and other software.
 */
void ohd_log(STATUS_LEVEL level, const std::string &message);

}

#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_UDP_LOG_H_
