//
// Created by consti10 on 19.04.23.
//

#include "TCPEndpoint.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <csignal>
#include <utility>

TCPEndpoint::TCPEndpoint(TCPEndpoint::Config config)
    : MEndpoint("TCPServer"),
      m_config(std::move(config))
{
  m_console = openhd::log::create_or_get(TAG);
  assert(m_console);
  bzero(&sockaddr, sizeof(sockaddr));
  m_loop_thread=std::make_unique<std::thread>(&TCPEndpoint::loop, this);
  m_console->debug("created with {}",m_config.port);
}

TCPEndpoint::~TCPEndpoint() {
  keep_alive= false;
  // signal it to stop
  shutdown(server_fd, SHUT_RDWR);
  if(m_loop_thread){
    m_loop_thread->join();
    m_loop_thread.reset();
  }
}

bool TCPEndpoint::sendMessagesImpl(
    const std::vector<MavlinkMessage>& messages) {
  if(new_socket!=0){
    auto message_buffers= pack_messages(messages);
    for(const auto& message_buffer:message_buffers){
      send(new_socket, message_buffer.data(), message_buffer.size(), 0);
    }
  }
  return false;
}

void TCPEndpoint::loop() {
  while (keep_alive){
    m_console->debug("TCPEndpoint::setup_and_allow_connection_once() begin");
    setup_and_allow_connection_once();
    m_console->debug("TCPEndpoint::setup_and_allow_connection_once() end");
    if(keep_alive){
      // Don't peg cpu on errors
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
}

void TCPEndpoint::setup_and_allow_connection_once() {
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    m_console->debug("socket failed");
    return ;
  }
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET,
                 SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    m_console->debug("setsockopt");
    return ;
  }
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = INADDR_ANY;
  sockaddr.sin_port = htons(m_config.port);

  if (bind(server_fd, (struct sockaddr*)&sockaddr,sizeof(sockaddr))< 0) {
    m_console->debug("bind failed");
    return ;
  }
  // signal readiness to accept clients
  if (listen(server_fd, 3) < 0) {
    m_console->debug("listen");
    return ;
  }
  m_console->debug("After listen");
  const  int addrlen = sizeof(sockaddr);
  const auto accept_result=accept(server_fd, (struct sockaddr*)&sockaddr,(socklen_t*)&addrlen);
  if (accept_result < 0) {
    m_console->debug("accept failed");
    return ;
  }
  new_socket=accept_result;
  m_console->debug("accepted client");
  // read buffer
  const auto buff = std::make_unique<std::array<uint8_t,READ_BUFF_SIZE>>();
  while (keep_alive){
    // Read from all the client(s)
    //std::this_thread::sleep_for(std::chrono::seconds(1));
    const ssize_t message_length = recv(server_fd, buff->data(), buff->size(), MSG_WAITALL);
    if (message_length > 0) {
      MEndpoint::parseNewData(buff->data(),message_length);
    }else{
      m_console->debug("Cannot read data");
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  // closing the connected (client) socket(s)
  close(new_socket);
  new_socket=0;
  // closing the listening socket
  shutdown(server_fd, SHUT_RDWR);
}

