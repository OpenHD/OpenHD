// MESSAGE SET_MODE support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief SET_MODE message
 *
 * Set the system mode, as defined by enum MAV_MODE. There is no target component id as the mode is by definition for the overall aircraft, not only for one component.
 */
struct SET_MODE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 11;
    static constexpr size_t LENGTH = 6;
    static constexpr size_t MIN_LENGTH = 6;
    static constexpr uint8_t CRC_EXTRA = 89;
    static constexpr auto NAME = "SET_MODE";


    uint8_t target_system; /*<  The system setting the mode */
    uint8_t base_mode; /*<  The new base mode. */
    uint32_t custom_mode; /*<  The new autopilot-specific mode. This field can be ignored by an autopilot. */


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
        ss << "  base_mode: " << +base_mode << std::endl;
        ss << "  custom_mode: " << custom_mode << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << custom_mode;                   // offset: 0
        map << target_system;                 // offset: 4
        map << base_mode;                     // offset: 5
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> custom_mode;                   // offset: 0
        map >> target_system;                 // offset: 4
        map >> base_mode;                     // offset: 5
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
