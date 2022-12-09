//
// Created by consti10 on 14.04.22.
//

#ifndef XMAVLINKSERVICE_UDPENDPOINT_H
#define XMAVLINKSERVICE_UDPENDPOINT_H

#include <thread>

#include "HelperSources/SocketHelper.hpp"
#include "MEndpoint.h"
#include "openhd-spdlog.hpp"
#include "openhd-telemetry-link-interface.h"

// Wraps two UDP ports, one for sending and one for receiving data
// (since TCP and UART for example also allow sending and receiving).
// If this endpoint shall only send data, set the receive port to -1 and never
// call sendMessage
class UDPEndpoint : public MEndpoint {
 public:
  /**
   * @param senderPort port for sending data to, or -1 to disable.
   * @param receiverPort port for receiving data, or -1 to disable.
   */
  UDPEndpoint(const std::string& TAG,int senderPort, int receiverPort,
			  std::string senderIp=SocketHelper::ADDRESS_LOCALHOST,std::string receiverIp=SocketHelper::ADDRESS_LOCALHOST);
  ~UDPEndpoint();
  // Makes it easy to not mess up the "what is UDP tx port on air unit is UDP rx port on ground unit" paradigm
  static std::unique_ptr<UDPEndpoint> createEndpointForOHDWifibroadcast(bool isAir);
 private:
  bool sendMessagesImpl(const std::vector<MavlinkMessage>& messages) override;
  const int SEND_PORT;
  const int RECV_PORT;
  std::unique_ptr<SocketHelper::UDPReceiver> receiver;
  std::unique_ptr<SocketHelper::UDPForwarder> transmitter;
  std::shared_ptr<spdlog::logger> m_console;
};

#endif //XMAVLINKSERVICE_UDPENDPOINT_H
