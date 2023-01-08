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
  m_console = openhd::log::create_or_get(TAG);
  assert(m_console);
  const auto cb = [this](const uint8_t *payload, const std::size_t payloadSize)mutable {
	this->parseNewData(payload, (int)payloadSize);
  };
  m_receiver_sender = std::make_unique<SocketHelper::UDPReceiver>(RECV_IP, RECV_PORT, cb);
  m_receiver_sender->runInBackground();
}

UDPEndpoint2::~UDPEndpoint2() {
  m_receiver_sender->stopBackground();
}

bool UDPEndpoint2::sendMessagesImpl(const std::vector<MavlinkMessage>& messages) {
  auto message_buffers= pack_messages(messages);
  for(const auto& message_buffer:message_buffers){
    m_receiver_sender->forwardPacketViaUDP(SENDER_IP,SEND_PORT,message_buffer.data(), message_buffer.size());
    std::lock_guard<std::mutex> lock(m_sender_mutex);
    for(const auto& [key,value]: m_other_dest_ips){
      m_receiver_sender->forwardPacketViaUDP(key,SEND_PORT,message_buffer.data(),message_buffer.size());
    }
  }
  return true;
}

void UDPEndpoint2::addAnotherDestIpAddress(std::string ip) {
  std::lock_guard<std::mutex> lock(m_sender_mutex);
  m_console->debug("addAnotherDestIpAddress {}",ip);
  m_other_dest_ips[ip]=nullptr;
}

void UDPEndpoint2::removeAnotherDestIpAddress(std::string ip) {
  std::lock_guard<std::mutex> lock(m_sender_mutex);
  m_console->debug("removeAnotherDestIpAddress {}",ip);
  m_other_dest_ips.erase(ip);
}
