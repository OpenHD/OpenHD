//
// Created by consti10 on 19.04.23.
//

#include "TCPEndpoint.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <csignal>
#include <utility>

#include "openhd_util.h"

TCPEndpoint::TCPEndpoint(TCPEndpoint::Config config,std::shared_ptr<openhd::ExternalDeviceManager> opt_external_device_manager)
    : MEndpoint("TCPServer"),
      m_config(std::move(config)),
      m_opt_external_device_manager(opt_external_device_manager)
{
  m_console = openhd::log::create_or_get(TAG);
  assert(m_console);
  m_loop_thread=std::make_unique<std::thread>(&TCPEndpoint::loop, this);
  m_console->debug("created with {}",m_config.port);
}

TCPEndpoint::~TCPEndpoint() {
  m_console->debug("TCPEndpoint::~TCPEndpoint() begin");
  // remove all external devices
  set_external_device_manager(nullptr);
  keep_alive= false;
  // this signals the fd to stop if needed
  shutdown(server_fd, SHUT_RDWR);
  if(client_socket){
      shutdown(client_socket, SHUT_RDWR);
  }
  //close(server_fd);
  if(m_loop_thread){
    m_loop_thread->join();
    m_loop_thread.reset();
  }
  m_console->debug("TCPEndpoint::~TCPEndpoint() end");
}

bool TCPEndpoint::sendMessagesImpl(const std::vector<MavlinkMessage>& messages) {
    if(m_has_clients){
        auto message_buffers= aggregate_pack_messages(messages,1024);
        for(const auto& message_buffer:message_buffers){
            const auto& buff=message_buffer.aggregated_data;
            send_message_to_all_clients(buff->data(),buff->size());
        }
        return true;
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
  struct sockaddr_in sockaddr{};
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    m_console->warn("open socket failed");
    return ;
  }
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET,
                 SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    m_console->warn("setsockopt failed");
    close(server_fd);
    return ;
  }
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = INADDR_ANY;
  sockaddr.sin_port = htons(m_config.port);

  if (bind(server_fd, (struct sockaddr*)&sockaddr,sizeof(sockaddr))< 0) {
    m_console->warn("bind failed");
    close(server_fd);
    return ;
  }
  // signal readiness to accept clients
  if (listen(server_fd, 3) < 0) {
    m_console->warn("listen failed");
    close(server_fd);
    return ;
  }
  m_console->debug("After listen");
  const  int addrlen = sizeof(sockaddr);
  const auto accept_result=accept(server_fd, (struct sockaddr*)&sockaddr,(socklen_t*)&addrlen);
  if (accept_result < 0) {
    m_console->debug("accept failed");
    close(server_fd);
    return ;
  }
  client_socket =accept_result;
  const std::string client_ip=inet_ntoa(sockaddr.sin_addr);
  const int client_port=ntohs(sockaddr.sin_port);
  m_console->debug("accepted client,sockfd:{}, ip:{}, port:{}",client_socket,
                   client_ip,client_port);
  on_external_device({client_ip,client_port},true);
  m_has_clients=true;
  receive_client_data_until_disconnect();
  // Client disconnected
  m_has_clients= false;
  on_external_device({client_ip,client_port}, false);
  // closing the connected (client) socket(s)
  close(client_socket);
  client_socket =0;
  // closing the listening socket
  close(server_fd);
}

void TCPEndpoint::send_message_to_all_clients(const uint8_t *data, int data_len) {
    if(client_socket !=0){
        const int flags =MSG_DONTWAIT |  // otherwise we might block if the socket got disconnected
                      MSG_NOSIGNAL; //otherwise we might crash if the socket disconnects
        send(client_socket, data, data_len, flags);
    }
}

void TCPEndpoint::receive_client_data_until_disconnect() {
    m_console->debug("receive_client_data_until_disconnect() begin");
    const auto buff = std::make_unique<std::array<uint8_t,READ_BUFF_SIZE>>();
    while (keep_alive){
        // Read from all the client(s)
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        //const ssize_t message_length = recv(client_socket, buff->data(), buff->size(), MSG_WAITALL);
        const ssize_t message_length = read(client_socket, buff->data(), buff->size());
        if(message_length<0){
            m_console->debug("Read error {} {}",message_length,strerror(errno));
            break ;
        }
        if(message_length==0){
            m_console->debug("Client disconnected");
            break ;
        }
        MEndpoint::parseNewData(buff->data(),(int)message_length);
    }
    m_console->debug("receive_client_data_until_disconnect() end");
}

void TCPEndpoint::set_external_device_manager(std::shared_ptr<openhd::ExternalDeviceManager> external_device_manager) {
    std::lock_guard<std::mutex> guard(m_external_devices_mutex);
    if(external_device_manager== nullptr){
        // remove
        if(m_opt_external_device_manager!= nullptr){
            // set all clients to disconnected
            for(auto& client: m_connected_clients){
                // Forward video there, but no telemetry (we already do telemetry via tcp)
                auto external_device=openhd::ExternalDevice{"TCP CLIENT",client.ip,true};
                m_opt_external_device_manager->on_new_external_device(external_device, false);
            }
            m_opt_external_device_manager= nullptr;
        }
        return;
    }
    m_opt_external_device_manager=external_device_manager;
    for(auto& client: m_connected_clients){
        // Forward video there, but no telemetry (we already do telemetry via tcp)
        auto external_device=openhd::ExternalDevice{"TCP CLIENT",client.ip,true};
        m_opt_external_device_manager->on_new_external_device(external_device, true);
    }
}

void TCPEndpoint::on_external_device(const TCPEndpoint::Client &client, bool connected) {
    std::lock_guard<std::mutex> guard(m_external_devices_mutex);
    if(m_opt_external_device_manager){
        auto external_device=openhd::ExternalDevice{"MAV TCP CLIENT",client.ip,true};
        m_opt_external_device_manager->on_new_external_device(external_device,connected);
    }
}

