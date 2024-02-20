//
// Created by consti10 on 31.01.24.
//

#include "openhd_tcp.h"

#include <arpa/inet.h>
#include <unistd.h>

#include <csignal>
#include <queue>
#include <utility>

openhd::TCPServer::TCPServer(const std::string tag,
                             openhd::TCPServer::Config config, bool debug)
    : m_config(config), m_debug(debug) {
  m_console = openhd::log::create_or_get(tag);
  assert(m_console);
  m_accept_thread =
      std::make_unique<std::thread>(&TCPServer::loop_accept, this);
  m_console->debug("created with {}", m_config.port);
}

openhd::TCPServer::~TCPServer() {
  // debug_if("TCPEndpoint::~TCPEndpoint() begin");
  //  First we make sure we don't accept any new connections anymore
  m_keep_accept_thread_alive = false;
  shutdown(server_fd, SHUT_RDWR);  // This will break out of select
  m_accept_thread->join();
  m_accept_thread = nullptr;
  server_fd = 0;
  // Then we make sure to clean up any connected client(s) (If there are any)
  for (const auto& client : m_clients_list) {
    client->marked_to_be_removed = true;
    client->keep_rx_looping = false;
    shutdown(client->sock_fd, SHUT_RDWR);
    client->rx_loop_thread->join();
    client->rx_loop_thread = nullptr;
  }
  m_console->debug("TCPEndpoint::~TCPEndpoint() end");
}

void openhd::TCPServer::loop_accept() {
  struct sockaddr_in sockaddr {};
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    m_console->warn("open socket failed");
    return;
  }
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    m_console->warn("setsockopt failed");
    close(server_fd);
    return;
  }
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = INADDR_ANY;
  sockaddr.sin_port = htons(m_config.port);
  if (bind(server_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
    m_console->warn("bind failed");
    close(server_fd);
    return;
  }
  // signal readiness to accept clients
  if (listen(server_fd, 5) < 0) {
    m_console->warn("listen failed");
    close(server_fd);
    return;
  }
  const int sockaddr_len = sizeof(sockaddr);
  while (m_keep_accept_thread_alive) {
    const auto accept_result = accept(server_fd, (struct sockaddr*)&sockaddr,
                                      (socklen_t*)&sockaddr_len);
    if (accept_result < 0) {
      m_console->debug("accept failed");
      close(server_fd);
      return;
    }
    const std::string client_ip = inet_ntoa(sockaddr.sin_addr);
    const int client_port = ntohs(sockaddr.sin_port);
    m_console->debug("accepted client,sockfd:{}, ip:{}, port:{}", accept_result,
                     client_ip, client_port);
    auto new_client = std::make_shared<ConnectedClient>();
    new_client->sock_fd = accept_result;
    new_client->ip = client_ip;
    new_client->port = client_port;
    new_client->keep_rx_looping = true;
    new_client->parent = this;
    new_client->rx_loop_thread = std::make_shared<std::thread>(
        &TCPServer::ConnectedClient::loop_rx, new_client.get());
    on_external_device(client_ip, client_port, true);
    {
      std::lock_guard<std::mutex> guard(m_clients_list_mutex);
      m_clients_list.push_back(new_client);
    }
  }
}

void openhd::TCPServer::send_message_to_all_clients(const uint8_t* data,
                                                    int data_len) {
  std::lock_guard<std::mutex> guard(m_clients_list_mutex);
  for (auto& client : m_clients_list) {
    if (!client->marked_to_be_removed) {
      const int flags =
          MSG_DONTWAIT |  // otherwise we might block if the socket got
                          // disconnected
          MSG_NOSIGNAL;   // otherwise we might crash if the socket disconnects
      if (!send(client->sock_fd, data, data_len, flags)) {
        m_console->debug("Client {} disconnected (cannot send data)",
                         client->ip);
        // Will be disconnected / removed by the accept thread
        client->marked_to_be_removed = true;
      }
    }
  }
}

void openhd::TCPServer::ConnectedClient::loop_rx() {
  auto console = openhd::log::create_or_get(fmt::format("TCPClient{}", ip));
  const auto buff = std::make_unique<std::array<uint8_t, READ_BUFF_SIZE>>();
  while (keep_rx_looping) {
    const ssize_t message_length = read(sock_fd, buff->data(), buff->size());
    if (message_length < 0) {
      console->debug("Read error {} {}", message_length, strerror(errno));
      marked_to_be_removed = true;
      break;
    }
    if (message_length == 0) {
      console->debug("Client disconnected");
      marked_to_be_removed = true;
      break;
    }
    parent->on_packet_any_tcp_client(buff->data(), message_length);
  }
  parent->on_external_device(ip, port, false);
}
