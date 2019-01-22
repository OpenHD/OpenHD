// MESSAGE MISSION_ITEM_INT support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief MISSION_ITEM_INT message
 *
 * Message encoding a mission item. This message is emitted to announce
                the presence of a mission item and to set a mission item on the system. The mission item can be either in x, y, z meters (type: LOCAL) or x:lat, y:lon, z:altitude. Local frame is Z-down, right handed (NED), global frame is Z-up, right handed (ENU). See also https://mavlink.io/en/protocol/mission.html.
 */
struct MISSION_ITEM_INT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 73;
    static constexpr size_t LENGTH = 38;
    static constexpr size_t MIN_LENGTH = 37;
    static constexpr uint8_t CRC_EXTRA = 38;
    static constexpr auto NAME = "MISSION_ITEM_INT";


    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
    uint16_t seq; /*<  Waypoint ID (sequence number). Starts at zero. Increases monotonically for each waypoint, no gaps in the sequence (0,1,2,3,4). */
    uint8_t frame; /*<  The coordinate system of the waypoint. */
    uint16_t command; /*<  The scheduled action for the waypoint. */
    uint8_t current; /*<  false:0, true:1 */
    uint8_t autocontinue; /*<  Autocontinue to next waypoint */
    float param1; /*<  PARAM1, see MAV_CMD enum */
    float param2; /*<  PARAM2, see MAV_CMD enum */
    float param3; /*<  PARAM3, see MAV_CMD enum */
    float param4; /*<  PARAM4, see MAV_CMD enum */
    int32_t x; /*<  PARAM5 / local: x position in meters * 1e4, global: latitude in degrees * 10^7 */
    int32_t y; /*<  PARAM6 / y position: local: x position in meters * 1e4, global: longitude in degrees *10^7 */
    float z; /*<  PARAM7 / z position: global: altitude in meters (relative or absolute, depending on frame. */
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
        ss << "  seq: " << seq << std::endl;
        ss << "  frame: " << +frame << std::endl;
        ss << "  command: " << command << std::endl;
        ss << "  current: " << +current << std::endl;
        ss << "  autocontinue: " << +autocontinue << std::endl;
        ss << "  param1: " << param1 << std::endl;
        ss << "  param2: " << param2 << std::endl;
        ss << "  param3: " << param3 << std::endl;
        ss << "  param4: " << param4 << std::endl;
        ss << "  x: " << x << std::endl;
        ss << "  y: " << y << std::endl;
        ss << "  z: " << z << std::endl;
        ss << "  mission_type: " << +mission_type << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << param1;                        // offset: 0
        map << param2;                        // offset: 4
        map << param3;                        // offset: 8
        map << param4;                        // offset: 12
        map << x;                             // offset: 16
        map << y;                             // offset: 20
        map << z;                             // offset: 24
        map << seq;                           // offset: 28
        map << command;                       // offset: 30
        map << target_system;                 // offset: 32
        map << target_component;              // offset: 33
        map << frame;                         // offset: 34
        map << current;                       // offset: 35
        map << autocontinue;                  // offset: 36
        map << mission_type;                  // offset: 37
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> param1;                        // offset: 0
        map >> param2;                        // offset: 4
        map >> param3;                        // offset: 8
        map >> param4;                        // offset: 12
        map >> x;                             // offset: 16
        map >> y;                             // offset: 20
        map >> z;                             // offset: 24
        map >> seq;                           // offset: 28
        map >> command;                       // offset: 30
        map >> target_system;                 // offset: 32
        map >> target_component;              // offset: 33
        map >> frame;                         // offset: 34
        map >> current;                       // offset: 35
        map >> autocontinue;                  // offset: 36
        map >> mission_type;                  // offset: 37
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
