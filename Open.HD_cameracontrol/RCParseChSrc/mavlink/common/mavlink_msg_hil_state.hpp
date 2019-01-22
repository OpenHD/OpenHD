// MESSAGE HIL_STATE support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief HIL_STATE message
 *
 * Sent from simulation to autopilot. This packet is useful for high throughput applications such as hardware in the loop simulations.
 */
struct HIL_STATE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 90;
    static constexpr size_t LENGTH = 56;
    static constexpr size_t MIN_LENGTH = 56;
    static constexpr uint8_t CRC_EXTRA = 183;
    static constexpr auto NAME = "HIL_STATE";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    float roll; /*< [rad] Roll angle */
    float pitch; /*< [rad] Pitch angle */
    float yaw; /*< [rad] Yaw angle */
    float rollspeed; /*< [rad/s] Body frame roll / phi angular speed */
    float pitchspeed; /*< [rad/s] Body frame pitch / theta angular speed */
    float yawspeed; /*< [rad/s] Body frame yaw / psi angular speed */
    int32_t lat; /*< [degE7] Latitude */
    int32_t lon; /*< [degE7] Longitude */
    int32_t alt; /*< [mm] Altitude */
    int16_t vx; /*< [cm/s] Ground X Speed (Latitude) */
    int16_t vy; /*< [cm/s] Ground Y Speed (Longitude) */
    int16_t vz; /*< [cm/s] Ground Z Speed (Altitude) */
    int16_t xacc; /*< [mG] X acceleration */
    int16_t yacc; /*< [mG] Y acceleration */
    int16_t zacc; /*< [mG] Z acceleration */


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
        ss << "  roll: " << roll << std::endl;
        ss << "  pitch: " << pitch << std::endl;
        ss << "  yaw: " << yaw << std::endl;
        ss << "  rollspeed: " << rollspeed << std::endl;
        ss << "  pitchspeed: " << pitchspeed << std::endl;
        ss << "  yawspeed: " << yawspeed << std::endl;
        ss << "  lat: " << lat << std::endl;
        ss << "  lon: " << lon << std::endl;
        ss << "  alt: " << alt << std::endl;
        ss << "  vx: " << vx << std::endl;
        ss << "  vy: " << vy << std::endl;
        ss << "  vz: " << vz << std::endl;
        ss << "  xacc: " << xacc << std::endl;
        ss << "  yacc: " << yacc << std::endl;
        ss << "  zacc: " << zacc << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << roll;                          // offset: 8
        map << pitch;                         // offset: 12
        map << yaw;                           // offset: 16
        map << rollspeed;                     // offset: 20
        map << pitchspeed;                    // offset: 24
        map << yawspeed;                      // offset: 28
        map << lat;                           // offset: 32
        map << lon;                           // offset: 36
        map << alt;                           // offset: 40
        map << vx;                            // offset: 44
        map << vy;                            // offset: 46
        map << vz;                            // offset: 48
        map << xacc;                          // offset: 50
        map << yacc;                          // offset: 52
        map << zacc;                          // offset: 54
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> roll;                          // offset: 8
        map >> pitch;                         // offset: 12
        map >> yaw;                           // offset: 16
        map >> rollspeed;                     // offset: 20
        map >> pitchspeed;                    // offset: 24
        map >> yawspeed;                      // offset: 28
        map >> lat;                           // offset: 32
        map >> lon;                           // offset: 36
        map >> alt;                           // offset: 40
        map >> vx;                            // offset: 44
        map >> vy;                            // offset: 46
        map >> vz;                            // offset: 48
        map >> xacc;                          // offset: 50
        map >> yacc;                          // offset: 52
        map >> zacc;                          // offset: 54
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
