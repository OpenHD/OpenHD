// MESSAGE SAFETY_SET_ALLOWED_AREA support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief SAFETY_SET_ALLOWED_AREA message
 *
 * Set a safety zone (volume), which is defined by two corners of a cube. This message can be used to tell the MAV which setpoints/waypoints to accept and which to reject. Safety areas are often enforced by national or competition regulations.
 */
struct SAFETY_SET_ALLOWED_AREA : mavlink::Message {
    static constexpr msgid_t MSG_ID = 54;
    static constexpr size_t LENGTH = 27;
    static constexpr size_t MIN_LENGTH = 27;
    static constexpr uint8_t CRC_EXTRA = 15;
    static constexpr auto NAME = "SAFETY_SET_ALLOWED_AREA";


    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
    uint8_t frame; /*<  Coordinate frame. Can be either global, GPS, right-handed with Z axis up or local, right handed, Z axis down. */
    float p1x; /*< [m] x position 1 / Latitude 1 */
    float p1y; /*< [m] y position 1 / Longitude 1 */
    float p1z; /*< [m] z position 1 / Altitude 1 */
    float p2x; /*< [m] x position 2 / Latitude 2 */
    float p2y; /*< [m] y position 2 / Longitude 2 */
    float p2z; /*< [m] z position 2 / Altitude 2 */


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
        ss << "  frame: " << +frame << std::endl;
        ss << "  p1x: " << p1x << std::endl;
        ss << "  p1y: " << p1y << std::endl;
        ss << "  p1z: " << p1z << std::endl;
        ss << "  p2x: " << p2x << std::endl;
        ss << "  p2y: " << p2y << std::endl;
        ss << "  p2z: " << p2z << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << p1x;                           // offset: 0
        map << p1y;                           // offset: 4
        map << p1z;                           // offset: 8
        map << p2x;                           // offset: 12
        map << p2y;                           // offset: 16
        map << p2z;                           // offset: 20
        map << target_system;                 // offset: 24
        map << target_component;              // offset: 25
        map << frame;                         // offset: 26
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> p1x;                           // offset: 0
        map >> p1y;                           // offset: 4
        map >> p1z;                           // offset: 8
        map >> p2x;                           // offset: 12
        map >> p2y;                           // offset: 16
        map >> p2z;                           // offset: 20
        map >> target_system;                 // offset: 24
        map >> target_component;              // offset: 25
        map >> frame;                         // offset: 26
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
