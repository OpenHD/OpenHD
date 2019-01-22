// MESSAGE MISSION_CURRENT support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief MISSION_CURRENT message
 *
 * Message that announces the sequence number of the current active mission item. The MAV will fly towards this mission item.
 */
struct MISSION_CURRENT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 42;
    static constexpr size_t LENGTH = 2;
    static constexpr size_t MIN_LENGTH = 2;
    static constexpr uint8_t CRC_EXTRA = 28;
    static constexpr auto NAME = "MISSION_CURRENT";


    uint16_t seq; /*<  Sequence */


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
        ss << "  seq: " << seq << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << seq;                           // offset: 0
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> seq;                           // offset: 0
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
