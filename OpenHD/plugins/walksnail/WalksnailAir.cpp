#include "WalksnailAir.h"
#include "WalksnailUtils.h"
#include <iostream>
#include <iomanip>

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

            std::cout << "Payload (" << tunnel.payload_length << " bytes): ";
            for (uint8_t i = 0; i < tunnel.payload_length; i++)
                std::cout << std::hex << std::setw(2) << std::setfill('0')
                        << static_cast<int>(tunnel.payload[i]) << " ";
            std::cout << std::dec << std::endl;

            if (m_walksnail_serial->isOpen()) {
                m_walksnail_serial->write(tunnel.payload, tunnel.payload_length);
            }
        }
    }
}
