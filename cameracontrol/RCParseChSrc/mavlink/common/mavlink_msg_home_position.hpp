// MESSAGE HOME_POSITION support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief HOME_POSITION message
 *
 * This message can be requested by sending the MAV_CMD_GET_HOME_POSITION command. The position the system will return to and land on. The position is set automatically by the system during the takeoff in case it was not explicitly set by the operator before or after. The position the system will return to and land on. The global and local positions encode the position in the respective coordinate frames, while the q parameter encodes the orientation of the surface. Under normal conditions it describes the heading and terrain slope, which can be used by the aircraft to adjust the approach. The approach 3D vector describes the point to which the system should fly in normal flight mode and then perform a landing sequence along the vector.
 */
struct HOME_POSITION : mavlink::Message {
    static constexpr msgid_t MSG_ID = 242;
    static constexpr size_t LENGTH = 60;
    static constexpr size_t MIN_LENGTH = 52;
    static constexpr uint8_t CRC_EXTRA = 104;
    static constexpr auto NAME = "HOME_POSITION";


    int32_t latitude; /*< [degE7] Latitude (WGS84) */
    int32_t longitude; /*< [degE7] Longitude (WGS84) */
    int32_t altitude; /*< [mm] Altitude (AMSL). Positive for up. */
    float x; /*< [m] Local X position of this position in the local coordinate frame */
    float y; /*< [m] Local Y position of this position in the local coordinate frame */
    float z; /*< [m] Local Z position of this position in the local coordinate frame */
    std::array<float, 4> q; /*<  World to surface normal and heading transformation of the takeoff position. Used to indicate the heading and slope of the ground */
    float approach_x; /*< [m] Local X position of the end of the approach vector. Multicopters should set this position based on their takeoff path. Grass-landing fixed wing aircraft should set it the same way as multicopters. Runway-landing fixed wing aircraft should set it to the opposite direction of the takeoff, assuming the takeoff happened from the threshold / touchdown zone. */
    float approach_y; /*< [m] Local Y position of the end of the approach vector. Multicopters should set this position based on their takeoff path. Grass-landing fixed wing aircraft should set it the same way as multicopters. Runway-landing fixed wing aircraft should set it to the opposite direction of the takeoff, assuming the takeoff happened from the threshold / touchdown zone. */
    float approach_z; /*< [m] Local Z position of the end of the approach vector. Multicopters should set this position based on their takeoff path. Grass-landing fixed wing aircraft should set it the same way as multicopters. Runway-landing fixed wing aircraft should set it to the opposite direction of the takeoff, assuming the takeoff happened from the threshold / touchdown zone. */
    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */


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
        ss << "  latitude: " << latitude << std::endl;
        ss << "  longitude: " << longitude << std::endl;
        ss << "  altitude: " << altitude << std::endl;
        ss << "  x: " << x << std::endl;
        ss << "  y: " << y << std::endl;
        ss << "  z: " << z << std::endl;
        ss << "  q: [" << to_string(q) << "]" << std::endl;
        ss << "  approach_x: " << approach_x << std::endl;
        ss << "  approach_y: " << approach_y << std::endl;
        ss << "  approach_z: " << approach_z << std::endl;
        ss << "  time_usec: " << time_usec << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << latitude;                      // offset: 0
        map << longitude;                     // offset: 4
        map << altitude;                      // offset: 8
        map << x;                             // offset: 12
        map << y;                             // offset: 16
        map << z;                             // offset: 20
        map << q;                             // offset: 24
        map << approach_x;                    // offset: 40
        map << approach_y;                    // offset: 44
        map << approach_z;                    // offset: 48
        map << time_usec;                     // offset: 52
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> latitude;                      // offset: 0
        map >> longitude;                     // offset: 4
        map >> altitude;                      // offset: 8
        map >> x;                             // offset: 12
        map >> y;                             // offset: 16
        map >> z;                             // offset: 20
        map >> q;                             // offset: 24
        map >> approach_x;                    // offset: 40
        map >> approach_y;                    // offset: 44
        map >> approach_z;                    // offset: 48
        map >> time_usec;                     // offset: 52
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
