// MESSAGE MISSION_WRITE_PARTIAL_LIST support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief MISSION_WRITE_PARTIAL_LIST message
 *
 * This message is sent to the MAV to write a partial list. If start index == end index, only one item will be transmitted / updated. If the start index is NOT 0 and above the current list size, this request should be REJECTED!
 */
struct MISSION_WRITE_PARTIAL_LIST : mavlink::Message {
    static constexpr msgid_t MSG_ID = 38;
    static constexpr size_t LENGTH = 7;
    static constexpr size_t MIN_LENGTH = 6;
    static constexpr uint8_t CRC_EXTRA = 9;
    static constexpr auto NAME = "MISSION_WRITE_PARTIAL_LIST";


    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
    int16_t start_index; /*<  Start index, 0 by default and smaller / equal to the largest index of the current onboard list. */
    int16_t end_index; /*<  End index, equal or greater than start index. */
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
