//
// Created by consti10 on 31.01.24.
//

#ifndef OPENHD_OPENHD_TCP_H
#define OPENHD_OPENHD_TCP_H

#include <deque>
#include <thread>

#include "openhd_spdlog.h"

namespace openhd{
/**
 * Non-blocking multiple-client(s) multi-threaded TCP server
 */
class TCPServer{
  struct Config{
    // always localhost
    //std::string ip;
    int port;
  };
  explicit TCPServer(std::string tag,Config config,bool debug=false);
  ~TCPServer();
  void on_packet_any_tcp_client(const uint8_t* data,int data_len);
 private:
  const Config m_config;
  const bool m_debug;
  std::shared_ptr<spdlog::logger> m_console;
  std::unique_ptr<std::thread> m_accept_thread = nullptr;
  bool m_keep_accept_thread_alive=true;
  int server_fd=0;
  static constexpr const size_t READ_BUFF_SIZE = 65507;
  void loop_accept();
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
    TCPServer* parent;
  };
  std::mutex m_clients_list_mutex;
  std::deque<std::shared_ptr<ConnectedClient>> m_clients_list;
  std::mutex m_rx_parse_mutex;
  template <typename... Args>
  void debug_if(spdlog::format_string_t<Args...> fmt, Args &&...args) {
    if(m_debug){
      m_console->debug(std::forward<Args>(args)...);
    }
  }
};
}

#endif  // OPENHD_OPENHD_TCP_H
