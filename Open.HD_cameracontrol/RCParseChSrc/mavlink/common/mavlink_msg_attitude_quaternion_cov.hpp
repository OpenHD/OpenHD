// MESSAGE ATTITUDE_QUATERNION_COV support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief ATTITUDE_QUATERNION_COV message
 *
 * The attitude in the aeronautical frame (right-handed, Z-down, X-front, Y-right), expressed as quaternion. Quaternion order is w, x, y, z and a zero rotation would be expressed as (1 0 0 0).
 */
struct ATTITUDE_QUATERNION_COV : mavlink::Message {
    static constexpr msgid_t MSG_ID = 61;
    static constexpr size_t LENGTH = 72;
    static constexpr size_t MIN_LENGTH = 72;
    static constexpr uint8_t CRC_EXTRA = 167;
    static constexpr auto NAME = "ATTITUDE_QUATERNION_COV";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    std::array<float, 4> q; /*<  Quaternion components, w, x, y, z (1 0 0 0 is the null-rotation) */
    float rollspeed; /*< [rad/s] Roll angular speed */
    float pitchspeed; /*< [rad/s] Pitch angular speed */
    float yawspeed; /*< [rad/s] Yaw angular speed */
    std::array<float, 9> covariance; /*<  Attitude covariance */


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
        ss << "  q: [" << to_string(q) << "]" << std::endl;
        ss << "  rollspeed: " << rollspeed << std::endl;
        ss << "  pitchspeed: " << pitchspeed << std::endl;
        ss << "  yawspeed: " << yawspeed << std::endl;
        ss << "  covariance: [" << to_string(covariance) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << q;                             // offset: 8
        map << rollspeed;                     // offset: 24
        map << pitchspeed;                    // offset: 28
        map << yawspeed;                      // offset: 32
        map << covariance;                    // offset: 36
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> q;                             // offset: 8
        map >> rollspeed;                     // offset: 24
        map >> pitchspeed;                    // offset: 28
        map >> yawspeed;                      // offset: 32
        map >> covariance;                    // offset: 36
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
