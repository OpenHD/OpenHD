#include "serial/serial.h"
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>

#define WALKSNAIL_DEFAULT_UART      "/tmp/ttyV0"
#define WALKSNAIL_DEFAULT_BAUDRATE  115200

class WalksnailBridge
{
public:
    void setup_bridge();
private:
    void reading_loop();

    std::unique_ptr<Serial> m_walksnail_serial = nullptr;
    std::mutex m_receive_thread_mutex;
    std::unique_ptr<std::thread> m_receive_thread = nullptr;
    std::atomic<bool> m_stop_requested = false;
};
