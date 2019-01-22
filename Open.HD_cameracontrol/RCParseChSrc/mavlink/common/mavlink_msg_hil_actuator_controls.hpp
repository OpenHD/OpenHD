// MESSAGE HIL_ACTUATOR_CONTROLS support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief HIL_ACTUATOR_CONTROLS message
 *
 * Sent from autopilot to simulation. Hardware in the loop control outputs (replacement for HIL_CONTROLS)
 */
struct HIL_ACTUATOR_CONTROLS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 93;
    static constexpr size_t LENGTH = 81;
    static constexpr size_t MIN_LENGTH = 81;
    static constexpr uint8_t CRC_EXTRA = 47;
    static constexpr auto NAME = "HIL_ACTUATOR_CONTROLS";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    std::array<float, 16> controls; /*<  Control outputs -1 .. 1. Channel assignment depends on the simulated hardware. */
    uint8_t mode; /*<  System mode. Includes arming state. */
    uint64_t flags; /*<  Flags as bitfield, reserved for future use. */


    inline std::string get_name(void) const override
    {
            return NAME;
    }

    inline Info get_message_info(void) const override
    {
            return { MSG_ID, LENGTH, MIN_LENGTH, CRC_EXTRA };
    }

    inline std::string to_yaml(void) const override
    {
        std::stringstream ss;

        ss << NAME << ":" << std::endl;
        ss << "  time_usec: " << time_usec << std::endl;
        ss << "  controls: [" << to_string(controls) << "]" << std::endl;
        ss << "  mode: " << +mode << std::endl;
        ss << "  flags: " << flags << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << flags;                         // offset: 8
        map << controls;                      // offset: 16
        map << mode;                          // offset: 80
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> flags;                         // offset: 8
        map >> controls;                      // offset: 16
        map >> mode;                          // offset: 80
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
