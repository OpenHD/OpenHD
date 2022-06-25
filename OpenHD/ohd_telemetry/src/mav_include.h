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

// For registering a callback that is called every time component X receives a new Mavlink Message
typedef std::function<void(MavlinkMessage &mavlinkMessage)> MAV_MSG_CALLBACK;

#endif //XMAVLINKSERVICE_MAV_INCLUDE_H
