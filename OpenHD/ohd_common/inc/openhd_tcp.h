//
// Created by consti10 on 31.01.24.
//

#ifndef OPENHD_OPENHD_TCP_H
#define OPENHD_OPENHD_TCP_H

#include <deque>
#include <thread>

#include "openhd_spdlog.h"

namespace openhd {
/**
 * Non-blocking multiple-client(s) multi-threaded TCP server
 * FEATURES:
 * 1) Multiple clients (multi-threaded)
 * 2) Automatically disconnect dead clients
 * 3) Generic interface where implementation can overwrite the following events:
 *      a) client connected / disconnected
 *      b) message received (any client)
 *   And send messages with a broadcast-like interface.
 */
class TCPServer {
 public:
  struct Config {
    // always localhost
    // std::string ip;
    int port;
  };
  explicit TCPServer(std::string tag, Config config, bool debug = false);
  ~TCPServer();
  /**
   * Needs to be overridden by implementation.
   * Called every time a packet (from any client) has been received.
   * The thread calling the method is different for every client.
   */
  virtual void on_packet_any_tcp_client(const uint8_t* data, int data_len) = 0;
  /**
   * Send the given message to all (currently) connected clients.
   * Non-blocking -
   */
  void send_message_to_all_clients(const uint8_t* data, int data_len);
  /**
   * Needs to be overridden by implementation.
   * Called with connected=true once a client connects, and connected==false
   * once a client disconnects (Or is dead and has been disconnected as a
   * caution feature)
   */
  virtual void on_external_device(std::string ip, int port, bool connected) = 0;

 private:
  const Config m_config;
  const bool m_debug;
  std::shared_ptr<spdlog::logger> m_console;
  std::unique_ptr<std::thread> m_accept_thread = nullptr;
  bool m_keep_accept_thread_alive = true;
  int server_fd = 0;
  static constexpr const size_t READ_BUFF_SIZE = 65507;
  void loop_accept();

 private:
  struct ConnectedClient {
    int sock_fd;
    std::string ip;
    int port;
    bool marked_to_be_removed = false;
    std::shared_ptr<std::thread> rx_loop_thread;
    void loop_rx();
    bool keep_rx_looping = true;
    TCPServer* parent;
  };
  std::mutex m_clients_list_mutex;
  std::deque<std::shared_ptr<ConnectedClient>> m_clients_list;
};
}  // namespace openhd

#endif  // OPENHD_OPENHD_TCP_H
