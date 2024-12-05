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

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_UDPENDPOINT2_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_UDPENDPOINT2_H_

#include <map>
#include <thread>

#include "MEndpoint.h"
#include "openhd_udp.h"

/**
 * Special, for communicating with QGroundControl.. / QOpenHD (any common
 * mavlink based QGC application) Quick: QGroundControl wants a TCP-like
 * communication - even in UDP if it receives data from a UDP sender::port
 * tuple, the responses are sent there,too. This actually makes sense, it is
 * just different to how EZ-WB / OpenHD used to do telemetry forwarding all the
 * time. But it means that we have to listen and send from the same UDP port.
 */
class UDPEndpoint : public MEndpoint {
 public:
  UDPEndpoint(const std::string& TAG, int senderPort, int receiverPort,
              std::string senderIp = openhd::ADDRESS_LOCALHOST,
              std::string receiverIp = openhd::ADDRESS_LOCALHOST);
  ~UDPEndpoint();
  // Delete copy and move
  UDPEndpoint(const UDPEndpoint&) = delete;
  UDPEndpoint(const UDPEndpoint&&) = delete;
  // These are for "external device forwarding"
  void addAnotherDestIpAddress(const std::string& ip);
  void removeAnotherDestIpAddress(const std::string& ip);

 private:
  std::shared_ptr<spdlog::logger> m_console;
  bool sendMessagesImpl(const std::vector<MavlinkMessage>& messages) override;
  const std::string SENDER_IP;
  const int SEND_PORT;
  const std::string RECV_IP;
  const int RECV_PORT;
  std::unique_ptr<openhd::UDPReceiver> m_receiver_sender;
  //
  std::mutex m_sender_mutex;
  std::map<std::string, void*> m_other_dest_ips{};
  std::vector<std::string> get_all_curr_dest_ips();
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_UDPENDPOINT2_H_
