//
// Created by consti10 on 11.06.22.
//

#include "UDPEndpoint2.h"

#include <utility>

UDPEndpoint2::UDPEndpoint2(const std::string &TAG,
						   int senderPort,
						   int receiverPort,
						   std::string senderIp,
						   std::string receiverIp):
	MEndpoint(TAG),
	SEND_PORT(senderPort), RECV_PORT(receiverPort),
	SENDER_IP(std::move(senderIp)),RECV_IP(std::move(receiverIp)){
  const auto cb = [this](const uint8_t *payload, const std::size_t payloadSize)mutable {
	this->parseNewData(payload, (int)payloadSize);
  };
  receiver_sender = std::make_unique<SocketHelper::UDPReceiver>(RECV_IP, RECV_PORT, cb);
  receiver_sender->runInBackground();
}

bool UDPEndpoint2::sendMessageImpl(const MavlinkMessage &message) {
  const auto data = message.pack();
  receiver_sender->forwardPacketViaUDP(SENDER_IP,SEND_PORT,data.data(),data.size());
  std::lock_guard<std::mutex> lock(_sender_mutex);
  for(const auto& other_ip:_other_dest_ips){
	receiver_sender->forwardPacketViaUDP(other_ip,SEND_PORT,data.data(),data.size());
  }
  return true;
}

void UDPEndpoint2::addAnotherDestIpAddress(std::string ip) {
  std::lock_guard<std::mutex> lock(_sender_mutex);
  std::stringstream ss;
  ss<<"UDPEndpoint2::addAnotherDestIpAddress:["<<ip<<"]\n";
  std::cout<<ss.str();
  _other_dest_ips.push_back(ip);
}
