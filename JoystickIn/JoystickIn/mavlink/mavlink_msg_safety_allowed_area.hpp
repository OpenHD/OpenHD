// MESSAGE SAFETY_ALLOWED_AREA support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief SAFETY_ALLOWED_AREA message
 *
 * Read out the safety zone the MAV currently assumes.
 */
struct SAFETY_ALLOWED_AREA : mavlink::Message {
    static constexpr msgid_t MSG_ID = 55;
    static constexpr size_t LENGTH = 25;
    static constexpr size_t MIN_LENGTH = 25;
    static constexpr uint8_t CRC_EXTRA = 3;
    static constexpr auto NAME = "SAFETY_ALLOWED_AREA";


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
        map << frame;                         // offset: 24
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> p1x;                           // offset: 0
        map >> p1y;                           // offset: 4
        map >> p1z;                           // offset: 8
        map >> p2x;                           // offset: 12
        map >> p2y;                           // offset: 16
        map >> p2z;                           // offset: 20
        map >> frame;                         // offset: 24
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
