// MESSAGE SIMSTATE support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief SIMSTATE message
 *
 * Status of simulation environment, if used.
 */
struct SIMSTATE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 164;
    static constexpr size_t LENGTH = 44;
    static constexpr size_t MIN_LENGTH = 44;
    static constexpr uint8_t CRC_EXTRA = 154;
    static constexpr auto NAME = "SIMSTATE";


    float roll; /*< [rad] Roll angle. */
    float pitch; /*< [rad] Pitch angle. */
    float yaw; /*< [rad] Yaw angle. */
    float xacc; /*< [m/s/s] X acceleration. */
    float yacc; /*< [m/s/s] Y acceleration. */
    float zacc; /*< [m/s/s] Z acceleration. */
    float xgyro; /*< [rad/s] Angular speed around X axis. */
    float ygyro; /*< [rad/s] Angular speed around Y axis. */
    float zgyro; /*< [rad/s] Angular speed around Z axis. */
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
        ss << "  xacc: " << xacc << std::endl;
        ss << "  yacc: " << yacc << std::endl;
        ss << "  zacc: " << zacc << std::endl;
        ss << "  xgyro: " << xgyro << std::endl;
        ss << "  ygyro: " << ygyro << std::endl;
        ss << "  zgyro: " << zgyro << std::endl;
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
        map << xacc;                          // offset: 12
        map << yacc;                          // offset: 16
        map << zacc;                          // offset: 20
        map << xgyro;                         // offset: 24
        map << ygyro;                         // offset: 28
        map << zgyro;                         // offset: 32
        map << lat;                           // offset: 36
        map << lng;                           // offset: 40
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> roll;                          // offset: 0
        map >> pitch;                         // offset: 4
        map >> yaw;                           // offset: 8
        map >> xacc;                          // offset: 12
        map >> yacc;                          // offset: 16
        map >> zacc;                          // offset: 20
        map >> xgyro;                         // offset: 24
        map >> ygyro;                         // offset: 28
        map >> zgyro;                         // offset: 32
        map >> lat;                           // offset: 36
        map >> lng;                           // offset: 40
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
