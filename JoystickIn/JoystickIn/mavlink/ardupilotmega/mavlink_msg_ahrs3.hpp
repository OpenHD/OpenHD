// MESSAGE AHRS3 support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief AHRS3 message
 *
 * Status of third AHRS filter if available. This is for ANU research group (Ali and Sean).
 */
struct AHRS3 : mavlink::Message {
    static constexpr msgid_t MSG_ID = 182;
    static constexpr size_t LENGTH = 40;
    static constexpr size_t MIN_LENGTH = 40;
    static constexpr uint8_t CRC_EXTRA = 229;
    static constexpr auto NAME = "AHRS3";


    float roll; /*< [rad] Roll angle. */
    float pitch; /*< [rad] Pitch angle. */
    float yaw; /*< [rad] Yaw angle. */
    float altitude; /*< [m] Altitude (MSL). */
    int32_t lat; /*< [degE7] Latitude. */
    int32_t lng; /*< [degE7] Longitude. */
    float v1; /*<  Test variable1. */
    float v2; /*<  Test variable2. */
    float v3; /*<  Test variable3. */
    float v4; /*<  Test variable4. */


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
        ss << "  v1: " << v1 << std::endl;
        ss << "  v2: " << v2 << std::endl;
        ss << "  v3: " << v3 << std::endl;
        ss << "  v4: " << v4 << std::endl;

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
        map << v1;                            // offset: 24
        map << v2;                            // offset: 28
        map << v3;                            // offset: 32
        map << v4;                            // offset: 36
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> roll;                          // offset: 0
        map >> pitch;                         // offset: 4
        map >> yaw;                           // offset: 8
        map >> altitude;                      // offset: 12
        map >> lat;                           // offset: 16
        map >> lng;                           // offset: 20
        map >> v1;                            // offset: 24
        map >> v2;                            // offset: 28
        map >> v3;                            // offset: 32
        map >> v4;                            // offset: 36
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
