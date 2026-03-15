#include "WalksnailBridge.h"
#include "WalksnailUtils.h"
#include <iostream>
#include <chrono>

void WalksnailBridge::setup_bridge()
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
        std::lock_guard<std::mutex> lock(m_callback_mutex);
        if (!m_callback) return;

        const uint8_t test_data[] = {
            0x01, 0x02, 0x03, 0x04, 0x05,
            'H', 'E', 'L', 'L', 'O'
        };
        auto messages = pack_uart_data_to_mavlink(
            test_data, sizeof(test_data),
            OHD_SYS_ID_GROUND,
            OHD_SYS_ID_FC
        );

        m_callback(messages);
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
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