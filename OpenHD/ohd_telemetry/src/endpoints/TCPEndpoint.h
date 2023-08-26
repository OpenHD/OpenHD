//
// Created by consti10 on 19.04.23.
//

#ifndef OPENHD_TCPENDPOINT_H
#define OPENHD_TCPENDPOINT_H

#include "MEndpoint.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "openhd_external_device.hpp"

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
  explicit TCPEndpoint(Config config,std::shared_ptr<openhd::ExternalDeviceManager> opt_external_device_manager= nullptr);
  ~TCPEndpoint();
  static constexpr int DEFAULT_PORT=5760;
  void set_external_device_manager(std::shared_ptr<openhd::ExternalDeviceManager> external_device_manager);
 private:
  const Config m_config;
  std::shared_ptr<spdlog::logger> m_console;
  std::unique_ptr<std::thread> m_loop_thread = nullptr;
  std::shared_ptr<openhd::ExternalDeviceManager> m_opt_external_device_manager;
  bool keep_alive=true;
  std::atomic<bool> m_has_clients=false;
  int server_fd=0;
  int client_socket =0;
  static constexpr const size_t READ_BUFF_SIZE = 65507;
  void loop();
  void setup_and_allow_connection_once();
  bool sendMessagesImpl(const std::vector<MavlinkMessage>& messages) override;
  void send_message_to_all_clients(const uint8_t* data,int data_len);
  void receive_client_data_until_disconnect();
  struct Client{
      std::string ip;
      int port;
  };
  std::mutex m_external_devices_mutex;
  std::vector<Client> m_connected_clients;
  void on_external_device(const Client& client,bool connected);
};

#endif  // OPENHD_TCPENDPOINT_H
