// MESSAGE ATTITUDE_QUATERNION support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief ATTITUDE_QUATERNION message
 *
 * The attitude in the aeronautical frame (right-handed, Z-down, X-front, Y-right), expressed as quaternion. Quaternion order is w, x, y, z and a zero rotation would be expressed as (1 0 0 0).
 */
struct ATTITUDE_QUATERNION : mavlink::Message {
    static constexpr msgid_t MSG_ID = 31;
    static constexpr size_t LENGTH = 32;
    static constexpr size_t MIN_LENGTH = 32;
    static constexpr uint8_t CRC_EXTRA = 246;
    static constexpr auto NAME = "ATTITUDE_QUATERNION";


    uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot). */
    float q1; /*<  Quaternion component 1, w (1 in null-rotation) */
    float q2; /*<  Quaternion component 2, x (0 in null-rotation) */
    float q3; /*<  Quaternion component 3, y (0 in null-rotation) */
    float q4; /*<  Quaternion component 4, z (0 in null-rotation) */
    float rollspeed; /*< [rad/s] Roll angular speed */
    float pitchspeed; /*< [rad/s] Pitch angular speed */
    float yawspeed; /*< [rad/s] Yaw angular speed */


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
        ss << "  time_boot_ms: " << time_boot_ms << std::endl;
        ss << "  q1: " << q1 << std::endl;
        ss << "  q2: " << q2 << std::endl;
        ss << "  q3: " << q3 << std::endl;
        ss << "  q4: " << q4 << std::endl;
        ss << "  rollspeed: " << rollspeed << std::endl;
        ss << "  pitchspeed: " << pitchspeed << std::endl;
        ss << "  yawspeed: " << yawspeed << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_boot_ms;                  // offset: 0
        map << q1;                            // offset: 4
        map << q2;                            // offset: 8
        map << q3;                            // offset: 12
        map << q4;                            // offset: 16
        map << rollspeed;                     // offset: 20
        map << pitchspeed;                    // offset: 24
        map << yawspeed;                      // offset: 28
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_boot_ms;                  // offset: 0
        map >> q1;                            // offset: 4
        map >> q2;                            // offset: 8
        map >> q3;                            // offset: 12
        map >> q4;                            // offset: 16
        map >> rollspeed;                     // offset: 20
        map >> pitchspeed;                    // offset: 24
        map >> yawspeed;                      // offset: 28
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
