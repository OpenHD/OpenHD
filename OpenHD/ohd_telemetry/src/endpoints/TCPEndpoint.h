//
// Created by consti10 on 19.04.23.
//

#ifndef OPENHD_TCPENDPOINT_H
#define OPENHD_TCPENDPOINT_H

#include "MEndpoint.h"

// TCP MAvlink server
class TCPEndpoint : public MEndpoint {
 public:
  struct Config{
    std::string ip;
    int port;
  };
  explicit TCPEndpoint(Config config);
 private:
  const Config m_config;
  std::shared_ptr<spdlog::logger> m_console;
  bool sendMessagesImpl(const std::vector<MavlinkMessage>& messages) override;
};

#endif  // OPENHD_TCPENDPOINT_H
