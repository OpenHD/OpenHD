// MESSAGE EXTENDED_SYS_STATE support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief EXTENDED_SYS_STATE message
 *
 * Provides state for additional features
 */
struct EXTENDED_SYS_STATE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 245;
    static constexpr size_t LENGTH = 2;
    static constexpr size_t MIN_LENGTH = 2;
    static constexpr uint8_t CRC_EXTRA = 130;
    static constexpr auto NAME = "EXTENDED_SYS_STATE";


    uint8_t vtol_state; /*<  The VTOL state if applicable. Is set to MAV_VTOL_STATE_UNDEFINED if UAV is not in VTOL configuration. */
    uint8_t landed_state; /*<  The landed state. Is set to MAV_LANDED_STATE_UNDEFINED if landed state is unknown. */


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
        ss << "  vtol_state: " << +vtol_state << std::endl;
        ss << "  landed_state: " << +landed_state << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << vtol_state;                    // offset: 0
        map << landed_state;                  // offset: 1
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> vtol_state;                    // offset: 0
        map >> landed_state;                  // offset: 1
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
