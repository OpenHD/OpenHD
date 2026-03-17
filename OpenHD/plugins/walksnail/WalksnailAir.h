#include "serial/serial.h"
#include "mav_include.h"
#include <vector>

#define WALKSNAIL_DEFAULT_UART      "/dev/serial0"
#define WALKSNAIL_DEFAULT_BAUDRATE  115200

class WalksnailAir
{
public:
    void process_ground_messages(std::vector<MavlinkMessage> messages);
    void setup_bridge();
    void stop_bridge();

private:
    std::unique_ptr<Serial> m_walksnail_serial = nullptr;
};
