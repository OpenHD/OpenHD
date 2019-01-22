// MESSAGE ALTITUDE support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief ALTITUDE message
 *
 * The current system altitude.
 */
struct ALTITUDE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 141;
    static constexpr size_t LENGTH = 32;
    static constexpr size_t MIN_LENGTH = 32;
    static constexpr uint8_t CRC_EXTRA = 47;
    static constexpr auto NAME = "ALTITUDE";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    float altitude_monotonic; /*< [m] This altitude measure is initialized on system boot and monotonic (it is never reset, but represents the local altitude change). The only guarantee on this field is that it will never be reset and is consistent within a flight. The recommended value for this field is the uncorrected barometric altitude at boot time. This altitude will also drift and vary between flights. */
    float altitude_amsl; /*< [m] This altitude measure is strictly above mean sea level and might be non-monotonic (it might reset on events like GPS lock or when a new QNH value is set). It should be the altitude to which global altitude waypoints are compared to. Note that it is *not* the GPS altitude, however, most GPS modules already output AMSL by default and not the WGS84 altitude. */
    float altitude_local; /*< [m] This is the local altitude in the local coordinate frame. It is not the altitude above home, but in reference to the coordinate origin (0, 0, 0). It is up-positive. */
    float altitude_relative; /*< [m] This is the altitude above the home position. It resets on each change of the current home position. */
    float altitude_terrain; /*< [m] This is the altitude above terrain. It might be fed by a terrain database or an altimeter. Values smaller than -1000 should be interpreted as unknown. */
    float bottom_clearance; /*< [m] This is not the altitude, but the clear space below the system according to the fused clearance estimate. It generally should max out at the maximum range of e.g. the laser altimeter. It is generally a moving target. A negative value indicates no measurement available. */


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
        ss << "  time_usec: " << time_usec << std::endl;
        ss << "  altitude_monotonic: " << altitude_monotonic << std::endl;
        ss << "  altitude_amsl: " << altitude_amsl << std::endl;
        ss << "  altitude_local: " << altitude_local << std::endl;
        ss << "  altitude_relative: " << altitude_relative << std::endl;
        ss << "  altitude_terrain: " << altitude_terrain << std::endl;
        ss << "  bottom_clearance: " << bottom_clearance << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << altitude_monotonic;            // offset: 8
        map << altitude_amsl;                 // offset: 12
        map << altitude_local;                // offset: 16
        map << altitude_relative;             // offset: 20
        map << altitude_terrain;              // offset: 24
        map << bottom_clearance;              // offset: 28
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> altitude_monotonic;            // offset: 8
        map >> altitude_amsl;                 // offset: 12
        map >> altitude_local;                // offset: 16
        map >> altitude_relative;             // offset: 20
        map >> altitude_terrain;              // offset: 24
        map >> bottom_clearance;              // offset: 28
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
