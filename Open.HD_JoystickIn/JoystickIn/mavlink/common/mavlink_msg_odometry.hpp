// MESSAGE ODOMETRY support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief ODOMETRY message
 *
 * Odometry message to communicate odometry information with an external interface. Fits ROS REP 147 standard for aerial vehicles (http://www.ros.org/reps/rep-0147.html).
 */
struct ODOMETRY : mavlink::Message {
    static constexpr msgid_t MSG_ID = 331;
    static constexpr size_t LENGTH = 230;
    static constexpr size_t MIN_LENGTH = 230;
    static constexpr uint8_t CRC_EXTRA = 58;
    static constexpr auto NAME = "ODOMETRY";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    uint8_t frame_id; /*<  Coordinate frame of reference for the pose data. */
    uint8_t child_frame_id; /*<  Coordinate frame of reference for the velocity in free space (twist) data. */
    float x; /*< [m] X Position */
    float y; /*< [m] Y Position */
    float z; /*< [m] Z Position */
    std::array<float, 4> q; /*<  Quaternion components, w, x, y, z (1 0 0 0 is the null-rotation) */
    float vx; /*< [m/s] X linear speed */
    float vy; /*< [m/s] Y linear speed */
    float vz; /*< [m/s] Z linear speed */
    float rollspeed; /*< [rad/s] Roll angular speed */
    float pitchspeed; /*< [rad/s] Pitch angular speed */
    float yawspeed; /*< [rad/s] Yaw angular speed */
    std::array<float, 21> pose_covariance; /*<  Pose (states: x, y, z, roll, pitch, yaw) covariance matrix upper right triangle (first six entries are the first ROW, next five entries are the second ROW, etc.) */
    std::array<float, 21> twist_covariance; /*<  Twist (states: vx, vy, vz, rollspeed, pitchspeed, yawspeed) covariance matrix upper right triangle (first six entries are the first ROW, next five entries are the second ROW, etc.) */


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
        ss << "  frame_id: " << +frame_id << std::endl;
        ss << "  child_frame_id: " << +child_frame_id << std::endl;
        ss << "  x: " << x << std::endl;
        ss << "  y: " << y << std::endl;
        ss << "  z: " << z << std::endl;
        ss << "  q: [" << to_string(q) << "]" << std::endl;
        ss << "  vx: " << vx << std::endl;
        ss << "  vy: " << vy << std::endl;
        ss << "  vz: " << vz << std::endl;
        ss << "  rollspeed: " << rollspeed << std::endl;
        ss << "  pitchspeed: " << pitchspeed << std::endl;
        ss << "  yawspeed: " << yawspeed << std::endl;
        ss << "  pose_covariance: [" << to_string(pose_covariance) << "]" << std::endl;
        ss << "  twist_covariance: [" << to_string(twist_covariance) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << x;                             // offset: 8
        map << y;                             // offset: 12
        map << z;                             // offset: 16
        map << q;                             // offset: 20
        map << vx;                            // offset: 36
        map << vy;                            // offset: 40
        map << vz;                            // offset: 44
        map << rollspeed;                     // offset: 48
        map << pitchspeed;                    // offset: 52
        map << yawspeed;                      // offset: 56
        map << pose_covariance;               // offset: 60
        map << twist_covariance;              // offset: 144
        map << frame_id;                      // offset: 228
        map << child_frame_id;                // offset: 229
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> x;                             // offset: 8
        map >> y;                             // offset: 12
        map >> z;                             // offset: 16
        map >> q;                             // offset: 20
        map >> vx;                            // offset: 36
        map >> vy;                            // offset: 40
        map >> vz;                            // offset: 44
        map >> rollspeed;                     // offset: 48
        map >> pitchspeed;                    // offset: 52
        map >> yawspeed;                      // offset: 56
        map >> pose_covariance;               // offset: 60
        map >> twist_covariance;              // offset: 144
        map >> frame_id;                      // offset: 228
        map >> child_frame_id;                // offset: 229
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
