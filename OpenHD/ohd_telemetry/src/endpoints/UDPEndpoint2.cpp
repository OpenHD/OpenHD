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

void UDPEndpoint2::sendMessageImpl(const MavlinkMessage &message) {
  const auto data = message.pack();
  receiver_sender->forwardPacketViaUDP(SENDER_IP,SEND_PORT,data.data(),data.size());
}
