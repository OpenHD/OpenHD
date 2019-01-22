// MESSAGE SIM_STATE support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief SIM_STATE message
 *
 * Status of simulation environment, if used
 */
struct SIM_STATE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 108;
    static constexpr size_t LENGTH = 84;
    static constexpr size_t MIN_LENGTH = 84;
    static constexpr uint8_t CRC_EXTRA = 32;
    static constexpr auto NAME = "SIM_STATE";


    float q1; /*<  True attitude quaternion component 1, w (1 in null-rotation) */
    float q2; /*<  True attitude quaternion component 2, x (0 in null-rotation) */
    float q3; /*<  True attitude quaternion component 3, y (0 in null-rotation) */
    float q4; /*<  True attitude quaternion component 4, z (0 in null-rotation) */
    float roll; /*<  Attitude roll expressed as Euler angles, not recommended except for human-readable outputs */
    float pitch; /*<  Attitude pitch expressed as Euler angles, not recommended except for human-readable outputs */
    float yaw; /*<  Attitude yaw expressed as Euler angles, not recommended except for human-readable outputs */
    float xacc; /*< [m/s/s] X acceleration */
    float yacc; /*< [m/s/s] Y acceleration */
    float zacc; /*< [m/s/s] Z acceleration */
    float xgyro; /*< [rad/s] Angular speed around X axis */
    float ygyro; /*< [rad/s] Angular speed around Y axis */
    float zgyro; /*< [rad/s] Angular speed around Z axis */
    float lat; /*< [deg] Latitude */
    float lon; /*< [deg] Longitude */
    float alt; /*< [m] Altitude */
    float std_dev_horz; /*<  Horizontal position standard deviation */
    float std_dev_vert; /*<  Vertical position standard deviation */
    float vn; /*< [m/s] True velocity in NORTH direction in earth-fixed NED frame */
    float ve; /*< [m/s] True velocity in EAST direction in earth-fixed NED frame */
    float vd; /*< [m/s] True velocity in DOWN direction in earth-fixed NED frame */


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
        ss << "  q1: " << q1 << std::endl;
        ss << "  q2: " << q2 << std::endl;
        ss << "  q3: " << q3 << std::endl;
        ss << "  q4: " << q4 << std::endl;
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
        ss << "  lon: " << lon << std::endl;
        ss << "  alt: " << alt << std::endl;
        ss << "  std_dev_horz: " << std_dev_horz << std::endl;
        ss << "  std_dev_vert: " << std_dev_vert << std::endl;
        ss << "  vn: " << vn << std::endl;
        ss << "  ve: " << ve << std::endl;
        ss << "  vd: " << vd << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << q1;                            // offset: 0
        map << q2;                            // offset: 4
        map << q3;                            // offset: 8
        map << q4;                            // offset: 12
        map << roll;                          // offset: 16
        map << pitch;                         // offset: 20
        map << yaw;                           // offset: 24
        map << xacc;                          // offset: 28
        map << yacc;                          // offset: 32
        map << zacc;                          // offset: 36
        map << xgyro;                         // offset: 40
        map << ygyro;                         // offset: 44
        map << zgyro;                         // offset: 48
        map << lat;                           // offset: 52
        map << lon;                           // offset: 56
        map << alt;                           // offset: 60
        map << std_dev_horz;                  // offset: 64
        map << std_dev_vert;                  // offset: 68
        map << vn;                            // offset: 72
        map << ve;                            // offset: 76
        map << vd;                            // offset: 80
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> q1;                            // offset: 0
        map >> q2;                            // offset: 4
        map >> q3;                            // offset: 8
        map >> q4;                            // offset: 12
        map >> roll;                          // offset: 16
        map >> pitch;                         // offset: 20
        map >> yaw;                           // offset: 24
        map >> xacc;                          // offset: 28
        map >> yacc;                          // offset: 32
        map >> zacc;                          // offset: 36
        map >> xgyro;                         // offset: 40
        map >> ygyro;                         // offset: 44
        map >> zgyro;                         // offset: 48
        map >> lat;                           // offset: 52
        map >> lon;                           // offset: 56
        map >> alt;                           // offset: 60
        map >> std_dev_horz;                  // offset: 64
        map >> std_dev_vert;                  // offset: 68
        map >> vn;                            // offset: 72
        map >> ve;                            // offset: 76
        map >> vd;                            // offset: 80
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
