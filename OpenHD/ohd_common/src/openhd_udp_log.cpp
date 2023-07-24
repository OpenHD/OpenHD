//
// Created by consti10 on 14.02.23.
//

#include "openhd_udp_log.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

openhd::log::udp::STATUS_LEVEL openhd::log::udp::level_spdlog_to_mavlink(
    const spdlog::level::level_enum& level) {
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

void openhd::log::udp::sendLocalLogMessageUDP(
    const openhd::log::udp::LogMessage &message) {
  assert(message.hasNullTerminator());
  int sockfd;
  struct sockaddr_in servaddr{};
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    std::cerr<<"Log message - create socket failed";
    return;
  }
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(openhd::LOCAL_LOG_MESSAGES_UDP_PORT);
  inet_aton("127.0.0.1", (in_addr *)&servaddr.sin_addr.s_addr);
  sendto(sockfd, &message, sizeof(message), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
  close(sockfd);
}

void openhd::log::udp::ohd_log(openhd::log::udp::STATUS_LEVEL level,
                               const std::string &message) {
  LogMessage lmessage{};
  lmessage.level = static_cast<uint8_t>(level);
  strncpy((char *)lmessage.message, message.c_str(), 50);
  if (lmessage.message[49] != '\0') {
    lmessage.message[49] = '\0';
  }
  sendLocalLogMessageUDP(lmessage);
}
