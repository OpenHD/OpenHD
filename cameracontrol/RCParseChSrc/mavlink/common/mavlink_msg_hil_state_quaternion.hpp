// MESSAGE HIL_STATE_QUATERNION support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief HIL_STATE_QUATERNION message
 *
 * Sent from simulation to autopilot, avoids in contrast to HIL_STATE singularities. This packet is useful for high throughput applications such as hardware in the loop simulations.
 */
struct HIL_STATE_QUATERNION : mavlink::Message {
    static constexpr msgid_t MSG_ID = 115;
    static constexpr size_t LENGTH = 64;
    static constexpr size_t MIN_LENGTH = 64;
    static constexpr uint8_t CRC_EXTRA = 4;
    static constexpr auto NAME = "HIL_STATE_QUATERNION";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    std::array<float, 4> attitude_quaternion; /*<  Vehicle attitude expressed as normalized quaternion in w, x, y, z order (with 1 0 0 0 being the null-rotation) */
    float rollspeed; /*< [rad/s] Body frame roll / phi angular speed */
    float pitchspeed; /*< [rad/s] Body frame pitch / theta angular speed */
    float yawspeed; /*< [rad/s] Body frame yaw / psi angular speed */
    int32_t lat; /*< [degE7] Latitude */
    int32_t lon; /*< [degE7] Longitude */
    int32_t alt; /*< [mm] Altitude */
    int16_t vx; /*< [cm/s] Ground X Speed (Latitude) */
    int16_t vy; /*< [cm/s] Ground Y Speed (Longitude) */
    int16_t vz; /*< [cm/s] Ground Z Speed (Altitude) */
    uint16_t ind_airspeed; /*< [cm/s] Indicated airspeed */
    uint16_t true_airspeed; /*< [cm/s] True airspeed */
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
        ss << "  attitude_quaternion: [" << to_string(attitude_quaternion) << "]" << std::endl;
        ss << "  rollspeed: " << rollspeed << std::endl;
        ss << "  pitchspeed: " << pitchspeed << std::endl;
        ss << "  yawspeed: " << yawspeed << std::endl;
        ss << "  lat: " << lat << std::endl;
        ss << "  lon: " << lon << std::endl;
        ss << "  alt: " << alt << std::endl;
        ss << "  vx: " << vx << std::endl;
        ss << "  vy: " << vy << std::endl;
        ss << "  vz: " << vz << std::endl;
        ss << "  ind_airspeed: " << ind_airspeed << std::endl;
        ss << "  true_airspeed: " << true_airspeed << std::endl;
        ss << "  xacc: " << xacc << std::endl;
        ss << "  yacc: " << yacc << std::endl;
        ss << "  zacc: " << zacc << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << attitude_quaternion;           // offset: 8
        map << rollspeed;                     // offset: 24
        map << pitchspeed;                    // offset: 28
        map << yawspeed;                      // offset: 32
        map << lat;                           // offset: 36
        map << lon;                           // offset: 40
        map << alt;                           // offset: 44
        map << vx;                            // offset: 48
        map << vy;                            // offset: 50
        map << vz;                            // offset: 52
        map << ind_airspeed;                  // offset: 54
        map << true_airspeed;                 // offset: 56
        map << xacc;                          // offset: 58
        map << yacc;                          // offset: 60
        map << zacc;                          // offset: 62
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> attitude_quaternion;           // offset: 8
        map >> rollspeed;                     // offset: 24
        map >> pitchspeed;                    // offset: 28
        map >> yawspeed;                      // offset: 32
        map >> lat;                           // offset: 36
        map >> lon;                           // offset: 40
        map >> alt;                           // offset: 44
        map >> vx;                            // offset: 48
        map >> vy;                            // offset: 50
        map >> vz;                            // offset: 52
        map >> ind_airspeed;                  // offset: 54
        map >> true_airspeed;                 // offset: 56
        map >> xacc;                          // offset: 58
        map >> yacc;                          // offset: 60
        map >> zacc;                          // offset: 62
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
