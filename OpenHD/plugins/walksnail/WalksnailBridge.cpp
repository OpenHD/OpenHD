#include "WalksnailBridge.h"
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
        std::lock_guard<std::mutex> lock(m_receive_thread_mutex);
        std::cout << m_walksnail_serial->readline() << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void WalksnailBridge::stop_bridge()
{
    m_stop_requested = true;
    if (m_receive_thread->joinable())
        m_receive_thread->join();
}
