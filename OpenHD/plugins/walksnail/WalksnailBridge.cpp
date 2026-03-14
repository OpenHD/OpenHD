#include "WalksnailBridge.h"
#include <iostream>


void WalksnailBridge::setup_bridge()
{
    m_walksnail_serial = std::make_unique<Serial>();
    if (!m_walksnail_serial->open())
    {
        std::cout << "Failed to open Walksnail serial" std::endl;
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
        std::cout << "READING LOOP TICK" << std::endl;
    }
}
