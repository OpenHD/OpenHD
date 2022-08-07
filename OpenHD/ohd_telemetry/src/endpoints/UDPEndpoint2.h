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
 * Quick: MAVSK wants a TCP-like communication - if it receives data from a sender::port tuple, it will send the responses there,too.
 * This actually makes sense, it is just different to how OpenHD used to do telemetry forwarding all the time.
 */
class UDPEndpoint2 : public MEndpoint {
 public:
  UDPEndpoint2(const std::string& TAG,int senderPort, int receiverPort,
			  std::string senderIp=SocketHelper::ADDRESS_LOCALHOST,std::string receiverIp=SocketHelper::ADDRESS_LOCALHOST);
  ~UDPEndpoint2();
  // Delete copy and move
  UDPEndpoint2(const UDPEndpoint2&)=delete;
  UDPEndpoint2(const UDPEndpoint2&&)=delete;
  void addAnotherDestIpAddress(std::string ip);
 private:
  bool sendMessageImpl(const MavlinkMessage &message) override;
  const std::string SENDER_IP;
  const int SEND_PORT;
  const std::string RECV_IP;
  const int RECV_PORT;
  std::unique_ptr<SocketHelper::UDPReceiver> receiver_sender;
  //
  std::mutex _sender_mutex;
  std::vector<std::string> _other_dest_ips{};
};

#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_UDPENDPOINT2_H_
