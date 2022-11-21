//
// Created by consti10 on 15.04.22.
//

#ifndef XMAVLINKSERVICE_MENDPOINT_H
#define XMAVLINKSERVICE_MENDPOINT_H

#include "openhd-spdlog.hpp"
#include "../mav_include.h"
#include "../mav_helper.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <mutex>
#include <utility>
#include <atomic>

// WARNING BE CAREFULL TO REMOVE ON RELEASE
//#define OHD_TELEMETRY_TESTING_ENABLE_PACKET_LOSS

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
// => A endpoint is considered alive it has received any mavlink messages in the last X seconds.
class MEndpoint {
 public:
  /**
   * The implementation-specific constructor SHOULD try and establish a connection as soon as possible
   * And re-establish the connection when disconnected.
   * @param tag a tag for debugging.
   * @param mavlink_channel the mavlink channel to use for parsing.
   */
  explicit MEndpoint(std::string tag) : TAG(std::move(tag)),m_mavlink_channel(checkoutFreeChannel()) {
    //m_console=openhd::log::create_or_get(tag);
    openhd::log::get_default()->debug(TAG+" using channel:{}",m_mavlink_channel);
  };
  /**
   * send a message via this endpoint.
   * If the endpoint is silently disconnected, this MUST NOT FAIL/CRASH.
   * This calls the underlying implementation's sendMessageImpl() function (pure virtual)
   * and increases the sent message count
   * @param message the message to send
   */
  void sendMessage(const MavlinkMessage &message){
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
  // Helper to send multiple messages at once
  void sendMessages(const std::vector<MavlinkMessage>& messages){
    for(const auto& msg:messages){
      sendMessage(msg);
    }
  }
  /**
   * register a callback that is called every time
   * this endpoint has received a new message
   * @param cb the callback function to register that is then called with a message every time a new full mavlink message has been parsed
   */
  void registerCallback(MAV_MSG_CALLBACK cb) {
	if (callback != nullptr) {
	  // this might be a common programming mistake - you can only register one callback here
	  openhd::log::get_default()->warn("Overwriting already existing callback");
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
	ss << TAG << " alive:" << (isAlive() ? "Y" : "N");
        openhd::log::get_default()->debug(ss.str());
  }
  [[nodiscard]] std::string createInfo()const{
	std::stringstream ss;
	ss<<TAG<<" {sent:"<<m_n_messages_sent<<" send_failed:"<<m_n_messages_send_failed<<" recv:"<<m_n_messages_received<<" alive:"<<(isAlive() ? "Y" : "N") << "}\n";
	return ss.str();
  }
  // can be public since immutable
  const std::string TAG;
 protected:
  // parse new data as it comes in, extract mavlink messages and forward them on the registered callback (if it has been registered)
  void parseNewData(const uint8_t *data,const int data_len) {
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
  // this one is special, since mavsdk in this case has already done the message parsing
  void parseNewDataEmulateForMavsdk(mavlink_message_t msg){
	onNewMavlinkMessage(msg);
  }
  // Must be overridden by the implementation
  // Returns true if the message has been properly sent (e.g. a connection exists on connection-based endpoints)
  // false otherwise
  virtual bool sendMessageImpl(const MavlinkMessage &message) = 0;
 private:
  MAV_MSG_CALLBACK callback = nullptr;
  // increases message count and forwards the message via the callback if registered.
  void onNewMavlinkMessage(mavlink_message_t msg){
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
  mavlink_status_t receiveMavlinkStatus{};
  const uint8_t m_mavlink_channel;
  std::chrono::steady_clock::time_point lastMessage{};
  int m_n_messages_received=0;
  // sendMessage() might be called by different threads.
  std::atomic<int> m_n_messages_sent=0;
  std::atomic<int> m_n_messages_send_failed=0;
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
  // https://stackoverflow.com/questions/12657962/how-do-i-generate-a-random-number-between-two-variables-that-i-have-stored
  static int random_number(int min,int max){
    srand(time(NULL)); // Seed the time
    const int num= rand()%(max-min+1)+min;
    return num;
  }
 protected:
  //std::shared_ptr<spdlog::logger> m_console;
};

#endif //XMAVLINKSERVICE_MENDPOINT_H
