#include <vector>
#include <mav_include.h>

class WalksnailAir
{
public:
    void process_ground_messages(std::vector<MavlinkMessage> messages);
};
