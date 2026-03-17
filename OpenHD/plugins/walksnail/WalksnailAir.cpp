#include "WalksnailAir.h"
#include "WalksnailUtils.h"
#include <iostream>

void WalksnailAir::setup_bridge()
{
    m_walksnail_serial = std::make_unique<Serial>(
        WALKSNAIL_DEFAULT_UART,
        WALKSNAIL_DEFAULT_BAUDRATE
    );
    if (!m_walksnail_serial->open())
    {
        std::cout << "Failed to open Walksnail serial" << std::endl;
        return;
    }
    else
    {
        std::cout << "Walksnail serial opened successfully!" << std::endl;
    }
}

void WalksnailAir::stop_bridge()
{
    
}

void WalksnailAir::process_ground_messages(std::vector<MavlinkMessage> messages)
{
    for (const auto& msg : messages) {
        if (msg.m.msgid == MAVLINK_MSG_ID_TUNNEL) {
            mavlink_tunnel_t tunnel;
            mavlink_msg_tunnel_decode(&msg.m, &tunnel);

            if (m_walksnail_serial->isOpen()) {
                m_walksnail_serial->write(tunnel.payload, tunnel.payload_length);
            }
        }
    }
}
