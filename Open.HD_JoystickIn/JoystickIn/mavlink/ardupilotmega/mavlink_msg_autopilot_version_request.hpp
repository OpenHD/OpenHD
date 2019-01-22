// MESSAGE AUTOPILOT_VERSION_REQUEST support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief AUTOPILOT_VERSION_REQUEST message
 *
 * Request the autopilot version from the system/component.
 */
struct AUTOPILOT_VERSION_REQUEST : mavlink::Message {
    static constexpr msgid_t MSG_ID = 183;
    static constexpr size_t LENGTH = 2;
    static constexpr size_t MIN_LENGTH = 2;
    static constexpr uint8_t CRC_EXTRA = 85;
    static constexpr auto NAME = "AUTOPILOT_VERSION_REQUEST";


    uint8_t target_system; /*<  System ID. */
    uint8_t target_component; /*<  Component ID. */


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
        ss << "  target_system: " << +target_system << std::endl;
        ss << "  target_component: " << +target_component << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << target_system;                 // offset: 0
        map << target_component;              // offset: 1
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> target_system;                 // offset: 0
        map >> target_component;              // offset: 1
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
