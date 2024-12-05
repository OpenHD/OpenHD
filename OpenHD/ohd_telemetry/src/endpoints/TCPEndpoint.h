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
