//
// Created by consti10 on 15.04.22.
//

#ifndef XMAVLINKSERVICE_MENDPOINT_H
#define XMAVLINKSERVICE_MENDPOINT_H

#include "../mav_include.h"
#include "../mav_helper.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <mutex>
#include <utility>

// Mavlink Endpoint
// A Mavlink endpoint hides away the underlying connection - e.g. UART, TCP, UDP.
// It has a (implementation-specific) method to send a message (sendMessage) and (implementation-specific)
// continuously forwards new incoming messages via a callback.
// It MUST also hide away any problems that could exist with this endpoint - e.g. a disconnecting UART.
// If (for example) in case of UART the connection is lost, it should just try to reconnect
// and as soon as the connection has been re-established, continue working as if nothing happened.
// This "send/receive data when possible, otherwise do nothing" behaviour fits well with the mavlink paradigm:
// https://mavlink.io/en/services/heartbeat.html
// "A component is considered to be connected to the network if its HEARTBEAT message is regularly received, and disconnected if a number of expected messages are not received."
class MEndpoint {
 public:
  /**
   * The implementation-specific constructor SHOULD try and establish a connection as soon as possible
   * And re-establish the connection when disconnected.
   * @param tag a tag for debugging.
   * @param mavlink_channel the mavlink channel to use for parsing.
   */
  explicit MEndpoint(std::string tag) : TAG(std::move(tag)),m_mavlink_channel(checkoutFreeChannel()) {
	std::cout<<TAG<<" using channel:"<<(int)m_mavlink_channel<<"\n";
  };
  /**
   * send a message via this endpoint.
   * If the endpoint is silently disconnected, this MUST NOT FAIL/CRASH
   * @param message the message to send
   */
  virtual void sendMessage(const MavlinkMessage &message) = 0;
  /**
   * register a callback that is called every time
   * this endpoint has received a new message
   * @param cb the callback function to register that is then called with a message every time a new full mavlink message has been parsed
   */
  void registerCallback(MAV_MSG_CALLBACK cb) {
	if (callback != nullptr) {
	  // this might be a common programming mistake - you can only register one callback here
	  std::cerr << "Overwriting already existing callback\n";
	}
	callback = std::move(cb);
  }
  /**
   * If (for some reason) you need to reason if this endpoint is alive, just check if it has received any mavlink messages
   * in the last X seconds
   */
  [[nodiscard]] bool isAlive()const {
	return (std::chrono::steady_clock::now() - lastMessage) < std::chrono::seconds(5);
  }
  /**
   * For debugging, print if this endpoint is alive (an endpoint is alive if it has received mavlink messages in the last X seconds).
   */
  void debugIfAlive()const {
	std::stringstream ss;
	ss << TAG << " alive:" << (isAlive() ? "Y" : "N") << "\n";
	std::cout << ss.str();
  }
  [[nodiscard]] std::string createInfo()const{
	std::stringstream ss;
	ss<<TAG<<" sent:"<<0<<" recv:"<<m_n_messages_received<<" alive:"<<(isAlive() ? "Y" : "N") << "\n";
	return ss.str();
  }
  // can be public since immutable
  const std::string TAG;
 protected:
  // parse new data as it comes in, extract mavlink messages and forward them on the registered callback (if it has been registered)
  void parseNewData(const uint8_t *data,const int data_len) {
	//std::cout<<TAG<<" received data:"<<data_len<<" "<<MavlinkHelpers::raw_content(data,data_len)<<"\n";
	int nMessages=0;
	mavlink_message_t msg;
	for (int i = 0; i < data_len; i++) {
	  uint8_t res = mavlink_parse_char(m_mavlink_channel, (uint8_t)data[i], &msg, &receiveMavlinkStatus);
	  if (res) {
		nMessages++;
		onNewMavlinkMessage(msg);
	  }
	}
	//std::cout<<TAG<<" N messages:"<<nMessages<<"\n";
	//std::cout<<TAG<<MavlinkHelpers::mavlink_status_to_string(receiveMavlinkStatus)<<"\n";
  }
  // this one is special, since mavsdk in this case has already done the message parsing
  void parseNewDataEmulateForMavsdk(mavlink_message_t msg){
	onNewMavlinkMessage(msg);
  }
 private:
  MAV_MSG_CALLBACK callback = nullptr;
  // increases message count and forwards the message via the callback if registered.
  void onNewMavlinkMessage(mavlink_message_t msg){
	lastMessage = std::chrono::steady_clock::now();
	MavlinkMessage message{msg};
	if (callback != nullptr) {
	  callback(message);
	} else {
	  std::cerr << "No callback set,did you forget to add it ?\n";
	}
	m_n_messages_received++;
  }
  mavlink_status_t receiveMavlinkStatus{};
  const uint8_t m_mavlink_channel;
  std::chrono::steady_clock::time_point lastMessage{};
  int m_n_messages_received=0;
  // I think mavlink channels are static, so each endpoint should use his own channel.
  // Based on mavsdk::mavlink_channels
  // It is not clear what the limit of the number of channels is, except UINT8_MAX.
  static int checkoutFreeChannel(){
	static std::mutex _channels_used_mutex;
	static int channel_idx=0;
	std::lock_guard<std::mutex> lock(_channels_used_mutex);
	int ret=channel_idx;
	channel_idx++;
	return ret;
  }
};

#endif //XMAVLINKSERVICE_MENDPOINT_H
