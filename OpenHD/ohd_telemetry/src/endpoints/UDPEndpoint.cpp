//
// Created by consti10 on 11.06.22.
//

#include "UDPEndpoint.h"

#include <cassert>
#include <utility>

#include "openhd_spdlog_include.h"

UDPEndpoint::UDPEndpoint(const std::string& TAG, int senderPort,
                         int receiverPort, std::string senderIp,
                         std::string receiverIp)
    : MEndpoint(TAG),
      SEND_PORT(senderPort),
      RECV_PORT(receiverPort),
      SENDER_IP(std::move(senderIp)),
      RECV_IP(std::move(receiverIp)) {
  m_console = openhd::log::create_or_get(TAG);
  assert(m_console);
  const auto cb = [this](const uint8_t* payload,
                         const std::size_t payloadSize) mutable {
    this->parseNewData(payload, (int)payloadSize);
  };
  m_receiver_sender =
      std::make_unique<openhd::UDPReceiver>(RECV_IP, RECV_PORT, cb);
  m_receiver_sender->runInBackground();
}

UDPEndpoint::~UDPEndpoint() { m_receiver_sender->stopBackground(); }

bool UDPEndpoint::sendMessagesImpl(
    const std::vector<MavlinkMessage>& messages) {
  auto message_buffers = aggregate_pack_messages(messages);
  const auto other_ips = get_all_curr_dest_ips();
  for (const auto& message_buffer : message_buffers) {
    m_receiver_sender->forwardPacketViaUDP(
        SENDER_IP, SEND_PORT, message_buffer.aggregated_data->data(),
        message_buffer.aggregated_data->size());
    for (const auto& ip : other_ips) {
      m_receiver_sender->forwardPacketViaUDP(
          ip, SEND_PORT, message_buffer.aggregated_data->data(),
          message_buffer.aggregated_data->size());
    }
  }
  return true;
}

void UDPEndpoint::addAnotherDestIpAddress(const std::string& ip) {
  std::lock_guard<std::mutex> lock(m_sender_mutex);
  m_console->debug("addAnotherDestIpAddress {}", ip);
  m_other_dest_ips[ip] = nullptr;
}

void UDPEndpoint::removeAnotherDestIpAddress(const std::string& ip) {
  std::lock_guard<std::mutex> lock(m_sender_mutex);
  m_console->debug("removeAnotherDestIpAddress {}", ip);
  m_other_dest_ips.erase(ip);
}

std::vector<std::string> UDPEndpoint::get_all_curr_dest_ips() {
  std::vector<std::string> ret;
  std::lock_guard<std::mutex> lock(m_sender_mutex);
  for (const auto& [key, value] : m_other_dest_ips) {
    ret.push_back(key);
  }
  return ret;
}

//// Now this is weird, but somehow we get a lot of junk from QGroundControll on
/// android ??!!
//      // QGroundControll defaults to 255
//      // QOpenHD defaults to 225;
//      const bool is_from_ground_controll=msg.m.sysid==255 || msg.m.sysid==225;
//      if(!is_from_ground_controll){
//        // This can't really be a message from a ground controll application
//        //m_console->debug("Dropping message");
//        return;
//      }