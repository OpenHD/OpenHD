//
// Created by consti10 on 13.04.22.
//

#ifndef XMAVLINKSERVICE_MAV_INCLUDE_H
#define XMAVLINKSERVICE_MAV_INCLUDE_H

extern "C" {
//NOTE: Make sure to include the openhd mavlink flavour, otherwise the custom messages won't bw parsed.
#include <openhd/mavlink.h>
}

#include <vector>
#include <functional>
#include <chrono>

// OpenHD mavlink sys IDs
static constexpr auto OHD_SYS_ID_GROUND = 100;
static constexpr auto OHD_SYS_ID_AIR = 101;
static_assert(OHD_SYS_ID_GROUND != OHD_SYS_ID_AIR);
// other
//static constexpr auto OHD_GROUND_CLIENT_TCP_PORT=14445;
static constexpr auto OHD_GROUND_CLIENT_TCP_PORT = 1234;

static constexpr auto OHD_GROUND_CLIENT_UDP_PORT_OUT = 14550;
static constexpr auto OHD_GROUND_CLIENT_UDP_PORT_IN = 14551;
//static constexpr auto OHD_GROUND_CLIENT_UDP_PORT_IN=58302;

struct MavlinkMessage {
  mavlink_message_t m{};
  [[nodiscard]] std::vector<uint8_t> pack() const {
	std::vector<uint8_t> buf(MAVLINK_MAX_PACKET_LEN);
	auto size = mavlink_msg_to_send_buffer(buf.data(), &m);
	buf.resize(size);
	return buf;
  }
};

// For registering a callback that is called every time component X receives one or more mavlink messages
typedef std::function<void(const std::vector<MavlinkMessage> messages)> MAV_MSG_CALLBACK;

static int64_t get_time_microseconds(){
  const auto time=std::chrono::steady_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::microseconds>(time).count();
}

#endif //XMAVLINKSERVICE_MAV_INCLUDE_H
