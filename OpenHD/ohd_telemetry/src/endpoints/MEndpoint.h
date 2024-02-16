//
// Created by consti10 on 15.04.22.
//

#ifndef XMAVLINKSERVICE_MENDPOINT_H
#define XMAVLINKSERVICE_MENDPOINT_H

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>
#include <utility>

#include "../mav_helper.h"
#include "../mav_include.h"
#include "openhd_spdlog.h"

// Mavlink Endpoint
// A Mavlink endpoint hides away the underlying connection - e.g. UART, TCP, WB.
// It has a (implementation-specific) method to send messages (sendMessage) and
// (implementation-specific) continuously forwards new incoming messages via a
// callback. It MUST also hide away any problems that could exist with this
// endpoint - e.g. a disconnecting UART. If (for example) in case of UART the
// connection is lost, it should just try to reconnect and as soon as the
// connection has been re-established, continue working as if nothing happened.
// This "send/receive data when possible, otherwise do nothing" behaviour fits
// well with the mavlink paradigm: https://mavlink.io/en/services/heartbeat.html
// "A component is considered to be connected to the network if its HEARTBEAT
// message is regularly received, and disconnected if a number of expected
// messages are not received."
// => A endpoint is considered alive it has received any mavlink messages in the
// last X seconds.
class MEndpoint {
 public:
  /**
   * The implementation-specific constructor SHOULD try and establish a
   * connection as soon as possible And re-establish the connection when
   * disconnected.
   * @param tag a tag for debugging.
   * @param mavlink_channel the mavlink channel to use for parsing.
   */
  explicit MEndpoint(std::string tag,
                     bool debug_mavlink_msg_packet_loss = false);
  /**
   * send one or more messages via this endpoint.
   * If the endpoint is silently disconnected, this MUST NOT FAIL/CRASH.
   * This calls the underlying implementation's sendMessageImpl() function (pure
   * virtual) and increases the sent message count
   * @param messages the messages to send
   */
  void sendMessages(const std::vector<MavlinkMessage>& messages);
  /**
   * register a callback that is called every time
   * this endpoint has received a new message
   * @param cb the callback function to register that is then called with a
   * message every time a new full mavlink message has been parsed
   */
  void registerCallback(MAV_MSG_CALLBACK cb);
  /**
   * If (for some reason) you need to reason if this endpoint is alive, just
   * check if it has received any mavlink messages in the last X seconds
   */
  [[nodiscard]] bool isAlive() const;
  /**
   * @return info about this endpoint, for debugging
   */
  [[nodiscard]] std::string createInfo() const;
  // can be public since immutable
  const std::string TAG;

 protected:
  // parse new data as it comes in, extract mavlink messages and forward them on
  // the registered callback (if it has been registered)
  void parseNewData(const uint8_t* data, int data_len);
  // this one is special, since mavsdk in this case has already done the message
  // parsing
  void parseNewDataEmulateForMavsdk(mavlink_message_t msg) {
    onNewMavlinkMessages({MavlinkMessage{msg}});
  }
  // Must be overridden by the implementation
  // Returns true if the message(s) have been properly sent (e.g. a connection
  // exists on connection-based endpoints) false otherwise
  virtual bool sendMessagesImpl(
      const std::vector<MavlinkMessage>& messages) = 0;

 private:
  MAV_MSG_CALLBACK m_callback = nullptr;
  // increases message count and forwards the messages via the callback if
  // registered.
  void onNewMavlinkMessages(std::vector<MavlinkMessage> messages);
  mavlink_status_t receiveMavlinkStatus{};
  const uint8_t m_mavlink_channel;
  std::chrono::steady_clock::time_point lastMessage{};
  int m_n_messages_received = 0;
  // sendMessage() might be called by different threads.
  std::atomic<int> m_n_messages_sent = 0;
  std::atomic<int> m_n_messages_send_failed = 0;
  // I think mavlink channels are static, so each endpoint should use his own
  // channel. Based on mavsdk::mavlink_channels It is not clear what the limit
  // of the number of channels is, except UINT8_MAX.
  static int checkoutFreeChannel();

 private:
  // Used to measure incoming / outgoing bits per second
  int m_tx_n_bytes = 0;
  int m_rx_n_bytes = 0;

 private:
  const bool m_debug_mavlink_msg_packet_loss;
  mavlink_status_t m_last_status;
};

#endif  // XMAVLINKSERVICE_MENDPOINT_H
