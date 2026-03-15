#include "WalksnailBridge.h"
#include "WalksnailUtils.h"
#include <iostream>
#include <chrono>

void WalksnailBridge::setup_bridge(uint8_t src_sys_id, uint8_t target_sys_id)
{
    m_src_sys_id = src_sys_id;
    m_target_sys_id = target_sys_id;

    m_walksnail_serial = std::make_unique<Serial>(
        WALKSNAIL_DEFAULT_UART,
        WALKSNAIL_DEFAULT_BAUDRATE
    );
    if (!m_walksnail_serial->open())
    {
        std::cout << "Failed to open Walksnail serial" << std::endl;
        return;
    }

    m_stop_requested = false;
    std::lock_guard<std::mutex> lock(m_receive_thread_mutex);
    m_receive_thread = std::make_unique<std::thread>(
        &WalksnailBridge::reading_loop,
        this
    );
}

void WalksnailBridge::reading_loop()
{
    while (!m_stop_requested)
    {
        uint8_t buf[64];
        const auto n = m_walksnail_serial->read(buf, sizeof(buf));
        if (n > 0) {
            std::lock_guard<std::mutex> lock(m_callback_mutex);
            if (!m_callback) return;

            auto messages = pack_walksnail_data_to_mavlink(
                buf, static_cast<size_t>(n),
                m_src_sys_id,
                m_target_sys_id
            );
    
            m_callback(messages);
        }
    }
}

void WalksnailBridge::stop_bridge()
{
    m_stop_requested = true;
    if (m_receive_thread->joinable())
        m_receive_thread->join();
}

void WalksnailBridge::registerCallback(MAV_MSG_CALLBACK callback) {
    std::lock_guard<std::mutex> lock(m_callback_mutex);
    m_callback = std::move(callback);
}
