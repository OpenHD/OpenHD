#pragma once

#include "mavlink_include.h"
#include <cstdint>

namespace mavsdk {

class Sender {
public:
    enum class Autopilot {
        Unknown,
        Px4,
        ArduPilot,
    };

    Sender() = default;
    virtual ~Sender() = default;
    virtual bool send_message(mavlink_message_t& message) = 0;
    [[nodiscard]] virtual uint8_t get_own_system_id() const = 0;
    [[nodiscard]] virtual uint8_t get_own_component_id() const = 0;
    [[nodiscard]] virtual uint8_t get_system_id() const = 0;
    [[nodiscard]] virtual Autopilot autopilot() const = 0;
};

} // namespace mavsdk