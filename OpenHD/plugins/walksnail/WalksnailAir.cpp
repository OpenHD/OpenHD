#include "WalksnailAir.h"
#include "WalksnailUtils.h"
#include <iostream>
#include <iomanip>

void WalksnailAir::setup_bridge(uint8_t src_sys_id, uint8_t target_sys_id)
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

    m_stop_requested = false;
    std::lock_guard<std::mutex> lock(m_receive_thread_mutex);
    m_receive_thread = std::make_unique<std::thread>(
        &WalksnailAir::reading_loop,
        this
    );
}

void WalksnailAir::stop_bridge()
{
    m_stop_requested = true;
    if (m_receive_thread->joinable())
        m_receive_thread->join();
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

void WalksnailAir::reading_loop()
{
    while (!m_stop_requested)
    {
        uint8_t buffer[256];
        int n = m_walksnail_serial->read(buffer, sizeof(buffer));
        if (n <= 0)
            continue;

        m_log_file.write(reinterpret_cast<char*>(buffer), n);
        m_log_file.flush();

        // std::string packet = m_walksnail_serial->readline();
        // if (packet.empty())
        //     continue;

        // m_log_file << packet << std::endl;

        // // Restore delimiter removed by readline
        // packet.push_back('\r');
        // packet.push_back('\n');

        // std::lock_guard<std::mutex> lock(m_callback_mutex);
        // if (!m_callback) return;

        // auto messages = pack_walksnail_data_to_mavlink(
        //     reinterpret_cast<const uint8_t*>(packet.data()), packet.size(),
        //     m_src_sys_id,
        //     m_target_sys_id
        // );

        // m_callback(messages);
    }
}

void WalksnailAir::register_callback(MAV_MSG_CALLBACK callback) {
    std::lock_guard<std::mutex> lock(m_callback_mutex);
    m_callback = std::move(callback);
}
