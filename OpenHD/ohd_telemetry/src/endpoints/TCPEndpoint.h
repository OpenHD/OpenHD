//
// Created by consti10 on 19.04.23.
//

#ifndef OPENHD_TCPENDPOINT_H
#define OPENHD_TCPENDPOINT_H

#include "MEndpoint.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

// TCP Mavlink server
class TCPEndpoint : public MEndpoint {
 public:
  struct Config{
    std::string ip;
    int port;
  };
  explicit TCPEndpoint(Config config);
  ~TCPEndpoint();
 private:
  const Config m_config;
  std::shared_ptr<spdlog::logger> m_console;
  bool sendMessagesImpl(const std::vector<MavlinkMessage>& messages) override;
  struct sockaddr_in sockaddr;
  int accept(int listener_fd);        ///< accept incoming connection
  bool setup(); ///< open connection and apply config
  bool reopen();                      ///< re-try connecting to the server
  void close();
};

#endif  // OPENHD_TCPENDPOINT_H
