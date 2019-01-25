// MESSAGE MISSION_ACK support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief MISSION_ACK message
 *
 * Acknowledgment message during waypoint handling. The type field states if this message is a positive ack (type=0) or if an error happened (type=non-zero).
 */
struct MISSION_ACK : mavlink::Message {
    static constexpr msgid_t MSG_ID = 47;
    static constexpr size_t LENGTH = 4;
    static constexpr size_t MIN_LENGTH = 3;
    static constexpr uint8_t CRC_EXTRA = 153;
    static constexpr auto NAME = "MISSION_ACK";


    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
    uint8_t type; /*<  Mission result. */
    uint8_t mission_type; /*<  Mission type. */


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
        ss << "  type: " << +type << std::endl;
        ss << "  mission_type: " << +mission_type << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << target_system;                 // offset: 0
        map << target_component;              // offset: 1
        map << type;                          // offset: 2
        map << mission_type;                  // offset: 3
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> target_system;                 // offset: 0
        map >> target_component;              // offset: 1
        map >> type;                          // offset: 2
        map >> mission_type;                  // offset: 3
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
