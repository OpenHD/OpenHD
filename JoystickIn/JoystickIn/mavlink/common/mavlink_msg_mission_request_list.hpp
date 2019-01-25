// MESSAGE MISSION_REQUEST_LIST support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief MISSION_REQUEST_LIST message
 *
 * Request the overall list of mission items from the system/component.
 */
struct MISSION_REQUEST_LIST : mavlink::Message {
    static constexpr msgid_t MSG_ID = 43;
    static constexpr size_t LENGTH = 3;
    static constexpr size_t MIN_LENGTH = 2;
    static constexpr uint8_t CRC_EXTRA = 132;
    static constexpr auto NAME = "MISSION_REQUEST_LIST";


    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
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
        ss << "  mission_type: " << +mission_type << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << target_system;                 // offset: 0
        map << target_component;              // offset: 1
        map << mission_type;                  // offset: 2
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> target_system;                 // offset: 0
        map >> target_component;              // offset: 1
        map >> mission_type;                  // offset: 2
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
