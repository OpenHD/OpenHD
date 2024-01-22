//
// Created by consti10 on 19.04.23.
//

#ifndef OPENHD_TCPENDPOINT_H
#define OPENHD_TCPENDPOINT_H

#include "MEndpoint.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "openhd_external_device.h"

// Simple TCP Mavlink server
// Really nice tutorial: https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
// R.n only supports up to 1 simultaneously connected client though
// The implementation for sending data is non-blocking and doesn't actually care if data arrives or not.

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
  //
  void on_packet_any_tcp_client(const uint8_t* data,int data_len);
 private:
  const Config m_config;
  std::shared_ptr<spdlog::logger> m_console;
  std::unique_ptr<std::thread> m_accept_thread = nullptr;
  bool m_keep_accept_thread_alive=true;
  int server_fd=0;
  static constexpr const size_t READ_BUFF_SIZE = 65507;
  void loop_accept();
  bool sendMessagesImpl(const std::vector<MavlinkMessage>& messages) override;
  void send_message_to_all_clients(const uint8_t* data,int data_len);
  void on_external_device(std::string ip,bool connected);
private:
    struct ConnectedClient{
        int sock_fd;
        std::string ip;
        int port;
        bool marked_to_be_removed= false;
        std::shared_ptr<std::thread> rx_loop_thread;
        void loop_rx();
        bool keep_rx_looping=true;
        TCPEndpoint* parent;
    };
    std::mutex m_clients_list_mutex;
    std::deque<std::shared_ptr<ConnectedClient>> m_clients_list;
    std::mutex m_rx_parse_mutex;
};

#endif  // OPENHD_TCPENDPOINT_H
