//
// Created by consti10 on 11.06.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_UDPENDPOINT2_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_UDPENDPOINT2_H_

#include <map>
#include <thread>
// dirty, pull in header only
#include "../../../lib/wifibroadcast/src/HelperSources/SocketHelper.hpp"
#include "MEndpoint.h"

/**
 * Special, for communicating with QGroundControl.. / QOpenHD (any common mavlink based QGC application)
 * Quick: QGroundControl wants a TCP-like communication - even in UDP
 * if it receives data from a UDP sender::port tuple, the responses are sent there,too.
 * This actually makes sense, it is just different to how EZ-WB / OpenHD used to do telemetry forwarding all the time.
 * But it means that we have to listen and send from the same UDP port.
 */
class UDPEndpoint2 : public MEndpoint {
 public:
  UDPEndpoint2(const std::string& TAG,int senderPort, int receiverPort,
			  std::string senderIp=SocketHelper::ADDRESS_LOCALHOST,std::string receiverIp=SocketHelper::ADDRESS_LOCALHOST);
  ~UDPEndpoint2();
  // Delete copy and move
  UDPEndpoint2(const UDPEndpoint2&)=delete;
  UDPEndpoint2(const UDPEndpoint2&&)=delete;
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
  std::unique_ptr<SocketHelper::UDPReceiver> m_receiver_sender;
  //
  std::mutex m_sender_mutex;
  std::map<std::string,void*> m_other_dest_ips{};
  std::vector<std::string> get_all_curr_dest_ips();
};

#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_UDPENDPOINT2_H_
