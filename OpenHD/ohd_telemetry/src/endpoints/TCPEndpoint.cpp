/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

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
