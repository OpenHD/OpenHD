//
// Created by consti10 on 19.04.23.
//

#ifndef OPENHD_TCPENDPOINT_H
#define OPENHD_TCPENDPOINT_H

#include "MEndpoint.h"
#include "openhd_external_device.h"
#include "openhd_tcp.h"

// Simple TCP Mavlink server (UDP-like)
class TCPEndpoint : public MEndpoint, openhd::TCPServer {
 public:
  explicit TCPEndpoint(openhd::TCPServer::Config config);
  ~TCPEndpoint() = default;
  static constexpr int DEFAULT_PORT = 5760;

 private:
  bool sendMessagesImpl(const std::vector<MavlinkMessage>& messages) override;
  std::mutex m_rx_parse_mutex;
  void on_external_device(std::string ip, int port, bool connected) override;
  void on_packet_any_tcp_client(const uint8_t* data, int data_len) override;
};

#endif  // OPENHD_TCPENDPOINT_H
