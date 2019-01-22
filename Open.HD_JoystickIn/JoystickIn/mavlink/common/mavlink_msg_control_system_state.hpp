// MESSAGE CONTROL_SYSTEM_STATE support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief CONTROL_SYSTEM_STATE message
 *
 * The smoothed, monotonic system state used to feed the control loops of the system.
 */
struct CONTROL_SYSTEM_STATE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 146;
    static constexpr size_t LENGTH = 100;
    static constexpr size_t MIN_LENGTH = 100;
    static constexpr uint8_t CRC_EXTRA = 103;
    static constexpr auto NAME = "CONTROL_SYSTEM_STATE";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    float x_acc; /*< [m/s/s] X acceleration in body frame */
    float y_acc; /*< [m/s/s] Y acceleration in body frame */
    float z_acc; /*< [m/s/s] Z acceleration in body frame */
    float x_vel; /*< [m/s] X velocity in body frame */
    float y_vel; /*< [m/s] Y velocity in body frame */
    float z_vel; /*< [m/s] Z velocity in body frame */
    float x_pos; /*< [m] X position in local frame */
    float y_pos; /*< [m] Y position in local frame */
    float z_pos; /*< [m] Z position in local frame */
    float airspeed; /*< [m/s] Airspeed, set to -1 if unknown */
    std::array<float, 3> vel_variance; /*<  Variance of body velocity estimate */
    std::array<float, 3> pos_variance; /*<  Variance in local position */
    std::array<float, 4> q; /*<  The attitude, represented as Quaternion */
    float roll_rate; /*< [rad/s] Angular rate in roll axis */
    float pitch_rate; /*< [rad/s] Angular rate in pitch axis */
    float yaw_rate; /*< [rad/s] Angular rate in yaw axis */


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
        ss << "  x_acc: " << x_acc << std::endl;
        ss << "  y_acc: " << y_acc << std::endl;
        ss << "  z_acc: " << z_acc << std::endl;
        ss << "  x_vel: " << x_vel << std::endl;
        ss << "  y_vel: " << y_vel << std::endl;
        ss << "  z_vel: " << z_vel << std::endl;
        ss << "  x_pos: " << x_pos << std::endl;
        ss << "  y_pos: " << y_pos << std::endl;
        ss << "  z_pos: " << z_pos << std::endl;
        ss << "  airspeed: " << airspeed << std::endl;
        ss << "  vel_variance: [" << to_string(vel_variance) << "]" << std::endl;
        ss << "  pos_variance: [" << to_string(pos_variance) << "]" << std::endl;
        ss << "  q: [" << to_string(q) << "]" << std::endl;
        ss << "  roll_rate: " << roll_rate << std::endl;
        ss << "  pitch_rate: " << pitch_rate << std::endl;
        ss << "  yaw_rate: " << yaw_rate << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << x_acc;                         // offset: 8
        map << y_acc;                         // offset: 12
        map << z_acc;                         // offset: 16
        map << x_vel;                         // offset: 20
        map << y_vel;                         // offset: 24
        map << z_vel;                         // offset: 28
        map << x_pos;                         // offset: 32
        map << y_pos;                         // offset: 36
        map << z_pos;                         // offset: 40
        map << airspeed;                      // offset: 44
        map << vel_variance;                  // offset: 48
        map << pos_variance;                  // offset: 60
        map << q;                             // offset: 72
        map << roll_rate;                     // offset: 88
        map << pitch_rate;                    // offset: 92
        map << yaw_rate;                      // offset: 96
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> x_acc;                         // offset: 8
        map >> y_acc;                         // offset: 12
        map >> z_acc;                         // offset: 16
        map >> x_vel;                         // offset: 20
        map >> y_vel;                         // offset: 24
        map >> z_vel;                         // offset: 28
        map >> x_pos;                         // offset: 32
        map >> y_pos;                         // offset: 36
        map >> z_pos;                         // offset: 40
        map >> airspeed;                      // offset: 44
        map >> vel_variance;                  // offset: 48
        map >> pos_variance;                  // offset: 60
        map >> q;                             // offset: 72
        map >> roll_rate;                     // offset: 88
        map >> pitch_rate;                    // offset: 92
        map >> yaw_rate;                      // offset: 96
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
