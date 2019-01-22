// MESSAGE MISSION_COUNT support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief MISSION_COUNT message
 *
 * This message is emitted as response to MISSION_REQUEST_LIST by the MAV and to initiate a write transaction. The GCS can then request the individual mission item based on the knowledge of the total number of waypoints.
 */
struct MISSION_COUNT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 44;
    static constexpr size_t LENGTH = 5;
    static constexpr size_t MIN_LENGTH = 4;
    static constexpr uint8_t CRC_EXTRA = 221;
    static constexpr auto NAME = "MISSION_COUNT";


    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
    uint16_t count; /*<  Number of mission items in the sequence */
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
        ss << "  count: " << count << std::endl;
        ss << "  mission_type: " << +mission_type << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << count;                         // offset: 0
        map << target_system;                 // offset: 2
        map << target_component;              // offset: 3
        map << mission_type;                  // offset: 4
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> count;                         // offset: 0
        map >> target_system;                 // offset: 2
        map >> target_component;              // offset: 3
        map >> mission_type;                  // offset: 4
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
