// MESSAGE MISSION_ITEM_REACHED support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief MISSION_ITEM_REACHED message
 *
 * A certain mission item has been reached. The system will either hold this position (or circle on the orbit) or (if the autocontinue on the WP was set) continue to the next waypoint.
 */
struct MISSION_ITEM_REACHED : mavlink::Message {
    static constexpr msgid_t MSG_ID = 46;
    static constexpr size_t LENGTH = 2;
    static constexpr size_t MIN_LENGTH = 2;
    static constexpr uint8_t CRC_EXTRA = 11;
    static constexpr auto NAME = "MISSION_ITEM_REACHED";


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
