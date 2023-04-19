//
// Created by consti10 on 19.04.23.
//

#ifndef OPENHD_TCPENDPOINT_H
#define OPENHD_TCPENDPOINT_H

#include "MEndpoint.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

// Simple TCP Mavlink server
// Really nice tutorial: https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
class TCPEndpoint : public MEndpoint {
 public:
  struct Config{
    // always localhost
    //std::string ip;
    int port;
  };
  explicit TCPEndpoint(Config config);
  ~TCPEndpoint();
  static constexpr int DEFAULT_PORT=5760;
 private:
  const Config m_config;
  std::shared_ptr<spdlog::logger> m_console;
  std::unique_ptr<std::thread> m_loop_thread = nullptr;
  bool keep_alive=true;
  int server_fd=0;
  int client_socket =0;
  static constexpr const size_t READ_BUFF_SIZE = 65507;
  void loop();
  void setup_and_allow_connection_once();
  bool sendMessagesImpl(const std::vector<MavlinkMessage>& messages) override;
};

#endif  // OPENHD_TCPENDPOINT_H
