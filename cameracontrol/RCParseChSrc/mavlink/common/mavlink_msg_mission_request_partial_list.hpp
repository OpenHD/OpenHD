// MESSAGE MISSION_REQUEST_PARTIAL_LIST support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief MISSION_REQUEST_PARTIAL_LIST message
 *
 * Request a partial list of mission items from the system/component. https://mavlink.io/en/protocol/mission.html. If start and end index are the same, just send one waypoint.
 */
struct MISSION_REQUEST_PARTIAL_LIST : mavlink::Message {
    static constexpr msgid_t MSG_ID = 37;
    static constexpr size_t LENGTH = 7;
    static constexpr size_t MIN_LENGTH = 6;
    static constexpr uint8_t CRC_EXTRA = 212;
    static constexpr auto NAME = "MISSION_REQUEST_PARTIAL_LIST";


    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
    int16_t start_index; /*<  Start index, 0 by default */
    int16_t end_index; /*<  End index, -1 by default (-1: send list to end). Else a valid index of the list */
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
        ss << "  start_index: " << start_index << std::endl;
        ss << "  end_index: " << end_index << std::endl;
        ss << "  mission_type: " << +mission_type << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << start_index;                   // offset: 0
        map << end_index;                     // offset: 2
        map << target_system;                 // offset: 4
        map << target_component;              // offset: 5
        map << mission_type;                  // offset: 6
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> start_index;                   // offset: 0
        map >> end_index;                     // offset: 2
        map >> target_system;                 // offset: 4
        map >> target_component;              // offset: 5
        map >> mission_type;                  // offset: 6
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
