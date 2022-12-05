//
// Created by consti10 on 05.12.22.
//

#include "MEndpoint.h"

MEndpoint::MEndpoint(std::string tag)
    : TAG(std::move(tag)),m_mavlink_channel(checkoutFreeChannel()) {
  openhd::log::get_default()->debug(TAG+" using channel:{}",m_mavlink_channel);
}

void MEndpoint::sendMessage(const MavlinkMessage& message) {
#ifdef OHD_TELEMETRY_TESTING_ENABLE_PACKET_LOSS
  const auto rand= random_number(0,100);
    if(rand>50){
      openhd::loggers::get_default()->debug("Emulate packet loss - dropped packet");
      return;
    }
#endif
  const auto res=sendMessageImpl(message);
  m_n_messages_sent++;
  if(!res)m_n_messages_send_failed++;
}

void MEndpoint::sendMessages(const std::vector<MavlinkMessage>& messages) {
  for(const auto& msg:messages){
    sendMessage(msg);
  }
}

void MEndpoint::registerCallback(MAV_MSG_CALLBACK cb) {
  if (callback != nullptr) {
    // this might be a common programming mistake - you can only register one callback here
    openhd::log::get_default()->warn("Overwriting already existing callback");
  }
  callback = std::move(cb);
}

bool MEndpoint::isAlive() const {
  return (std::chrono::steady_clock::now() - lastMessage) < std::chrono::seconds(5);
}

std::string MEndpoint::createInfo() const {
  std::stringstream ss;
  ss<<TAG<<" {sent:"<<m_n_messages_sent<<" send_failed:"<<m_n_messages_send_failed<<" recv:"<<m_n_messages_received<<" alive:"<<(isAlive() ? "Y" : "N") << "}\n";
  return ss.str();
}

void MEndpoint::parseNewData(const uint8_t* data, const int data_len) {
  //<<TAG<<" received data:"<<data_len<<" "<<MavlinkHelpers::raw_content(data,data_len)<<"\n";
  int nMessages=0;
  mavlink_message_t msg;
  for (int i = 0; i < data_len; i++) {
    uint8_t res = mavlink_parse_char(m_mavlink_channel, (uint8_t)data[i], &msg, &receiveMavlinkStatus);
    if (res) {
      nMessages++;
      onNewMavlinkMessage(msg);
    }
  }
  //<<TAG<<" N messages:"<<nMessages<<"\n";
  //<<TAG<<MavlinkHelpers::mavlink_status_to_string(receiveMavlinkStatus)<<"\n";
}

void MEndpoint::onNewMavlinkMessage(mavlink_message_t msg) {
#ifdef OHD_TELEMETRY_TESTING_ENABLE_PACKET_LOSS
  const auto rand= random_number(0,100);
    if(rand>50){
      openhd::loggers::get_default()->debug("Emulate packet loss - dropped packet");
      return;1
    }
#endif
  lastMessage = std::chrono::steady_clock::now();
  MavlinkMessage message{msg};
  if (callback != nullptr) {
    callback(message);
  } else {
    openhd::log::get_default()->warn("No callback set,did you forget to add it ?");
  }
  m_n_messages_received++;
}
