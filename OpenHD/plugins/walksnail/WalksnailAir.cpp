#include "WalksnailAir.h"
#include "WalksnailUtils.h"
#include <iostream>

void WalksnailAir::process_ground_messages(std::vector<MavlinkMessage> messages)
{
    for (const auto& msg : messages) {
        if (msg.m.msgid == MAVLINK_MSG_ID_TUNNEL) {
            mavlink_tunnel_t tunnel;
            mavlink_msg_tunnel_decode(&msg.m, &tunnel);

            std::string str(reinterpret_cast<const char*>(tunnel.payload), tunnel.payload_length);
            std::cout << msg.m.msgid << ", payload: " << str << std::endl;
        }
    }
}