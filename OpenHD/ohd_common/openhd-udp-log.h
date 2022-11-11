//
// Created by consti10 on 11.11.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_UDP_LOG_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_UDP_LOG_H_

#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>
#include <iostream>
#include <sstream>
#include <cassert>

#include "openhd-global-constants.hpp"

/**
 * This provides convenient methods to send log messages from any service running on the air or ground unit to a final output deice,
 * for example QOpenHD.
 * As an example, this makes it possible to view the logs from the air unit on QOpenHD without connecting a display to the air unit.
 * The general way this works is simple:
 * The log messages is sent to a specific udp port on localhost and then picked up by the telemetry service,
 * which converts it to mavlink and forwards it accordingly.
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
    default:
      break;
  }
  return STATUS_LEVEL::DEBUG;
}

/**
 * Send a log message out via udp to localhost, it wll be picked up by the telemetry service.
 * @param message the log message to send, has to have a valid null terminator.
 */
static void sendLocalLogMessageUDP(const LogMessage & message){
  assert(message.hasNullTerminator());
  int sockfd;
  struct sockaddr_in servaddr{};
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Socket create failed");
    exit(EXIT_FAILURE);
  }
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(OHD_LOCAL_LOG_MESSAGES_UDP_PORT);
  inet_aton("127.0.0.1", (in_addr *)&servaddr.sin_addr.s_addr);
  sendto(sockfd, &message, sizeof(message), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
  close(sockfd);
}

/*
 * Messages sent here will end up in the telemetry microservice, where they will be packed up and sent through
 * mavlink for storage and review by qopenhd, the boot screen system, and other software.
 */
static void ohd_log(STATUS_LEVEL level, const std::string &message) {
  LogMessage lmessage{};
  lmessage.level = static_cast<uint8_t>(level);
  strncpy((char *)lmessage.message, message.c_str(), 50);
  if (lmessage.message[49] != '\0') {
    lmessage.message[49] = '\0';
  }
  sendLocalLogMessageUDP(lmessage);
}

}

#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_UDP_LOG_H_
