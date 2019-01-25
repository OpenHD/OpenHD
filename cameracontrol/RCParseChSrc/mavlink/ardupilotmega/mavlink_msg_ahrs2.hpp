// MESSAGE AHRS2 support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief AHRS2 message
 *
 * Status of secondary AHRS filter if available.
 */
struct AHRS2 : mavlink::Message {
    static constexpr msgid_t MSG_ID = 178;
    static constexpr size_t LENGTH = 24;
    static constexpr size_t MIN_LENGTH = 24;
    static constexpr uint8_t CRC_EXTRA = 47;
    static constexpr auto NAME = "AHRS2";


    float roll; /*< [rad] Roll angle. */
    float pitch; /*< [rad] Pitch angle. */
    float yaw; /*< [rad] Yaw angle. */
    float altitude; /*< [m] Altitude (MSL). */
    int32_t lat; /*< [degE7] Latitude. */
    int32_t lng; /*< [degE7] Longitude. */


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
        ss << "  roll: " << roll << std::endl;
        ss << "  pitch: " << pitch << std::endl;
        ss << "  yaw: " << yaw << std::endl;
        ss << "  altitude: " << altitude << std::endl;
        ss << "  lat: " << lat << std::endl;
        ss << "  lng: " << lng << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << roll;                          // offset: 0
        map << pitch;                         // offset: 4
        map << yaw;                           // offset: 8
        map << altitude;                      // offset: 12
        map << lat;                           // offset: 16
        map << lng;                           // offset: 20
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> roll;                          // offset: 0
        map >> pitch;                         // offset: 4
        map >> yaw;                           // offset: 8
        map >> altitude;                      // offset: 12
        map >> lat;                           // offset: 16
        map >> lng;                           // offset: 20
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
