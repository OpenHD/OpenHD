//
// Created by consti10 on 11.06.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_UDPENDPOINT2_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_UDPENDPOINT2_H_

#include "MEndpoint.hpp"
#include "HelperSources/SocketHelper.hpp"
#include <thread>

/**
 * Special, for communicating with MAVSDK.
 */
class UDPEndpoint2 : public MEndpoint {
 public:
  UDPEndpoint2(const std::string& TAG,int senderPort, int receiverPort,
			  std::string senderIp=SocketHelper::ADDRESS_LOCALHOST,std::string receiverIp=SocketHelper::ADDRESS_LOCALHOST);
  void sendMessage(const MavlinkMessage &message) override;
 private:
  const std::string SENDER_IP;
  const int SEND_PORT;
  const std::string RECV_IP;
  const int RECV_PORT;
  std::unique_ptr<SocketHelper::UDPReceiver> receiver;
};

#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_UDPENDPOINT2_H_
