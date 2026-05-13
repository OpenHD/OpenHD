#include "serial/serial.h"
#include "mav_include.h"
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>

#define WALKSNAIL_DEFAULT_UART      "/dev/serial0"
#define WALKSNAIL_DEFAULT_BAUDRATE  115200

class WalksnailGround
{
public:
    void setup_bridge(uint8_t src_sys_id, uint8_t target_sys_id);
    void stop_bridge();
    void process_air_messages(std::vector<MavlinkMessage> messages);
    void registerCallback(MAV_MSG_CALLBACK callback);
private:
    void reading_loop();

    uint8_t m_src_sys_id = 0;
    uint8_t m_target_sys_id = 0;

    std::unique_ptr<Serial> m_walksnail_serial = nullptr;
    std::mutex m_receive_thread_mutex;
    std::unique_ptr<std::thread> m_receive_thread = nullptr;
    std::atomic<bool> m_stop_requested = false;

    MAV_MSG_CALLBACK m_callback = nullptr;
    std::mutex m_callback_mutex;
};
