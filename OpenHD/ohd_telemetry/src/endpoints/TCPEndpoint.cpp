//
// Created by consti10 on 19.04.23.
//

#include "TCPEndpoint.h"

#include <arpa/inet.h>
#include <unistd.h>

#include <csignal>
#include <queue>
#include <utility>

#include "openhd_util.h"

TCPEndpoint::TCPEndpoint(openhd::TCPServer::Config config)
    : MEndpoint("TCPServer"), openhd::TCPServer("MTCPServer", config) {}

bool TCPEndpoint::sendMessagesImpl(
    const std::vector<MavlinkMessage>& messages) {
  auto message_buffers = aggregate_pack_messages(messages, 1024);
  for (const auto& message_buffer : message_buffers) {
    const auto& buff = message_buffer.aggregated_data;
    send_message_to_all_clients(buff->data(), buff->size());
  }
  return true;
}

void TCPEndpoint::on_external_device(std::string ip, int port, bool connected) {
  auto external_device = openhd::ExternalDevice{"MAV TCP CLIENT", ip, true};
  openhd::ExternalDeviceManager::instance().on_new_external_device(
      external_device, connected);
}

void TCPEndpoint::on_packet_any_tcp_client(const uint8_t* data, int data_len) {
  std::lock_guard<std::mutex> guard(m_rx_parse_mutex);
  MEndpoint::parseNewData(data, (int)data_len);
}
