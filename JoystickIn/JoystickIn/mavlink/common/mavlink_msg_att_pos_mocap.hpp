// MESSAGE ATT_POS_MOCAP support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief ATT_POS_MOCAP message
 *
 * Motion capture attitude and position
 */
struct ATT_POS_MOCAP : mavlink::Message {
    static constexpr msgid_t MSG_ID = 138;
    static constexpr size_t LENGTH = 120;
    static constexpr size_t MIN_LENGTH = 36;
    static constexpr uint8_t CRC_EXTRA = 109;
    static constexpr auto NAME = "ATT_POS_MOCAP";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    std::array<float, 4> q; /*<  Attitude quaternion (w, x, y, z order, zero-rotation is 1, 0, 0, 0) */
    float x; /*< [m] X position (NED) */
    float y; /*< [m] Y position (NED) */
    float z; /*< [m] Z position (NED) */
    std::array<float, 21> covariance; /*<  Pose covariance matrix upper right triangular (first six entries are the first ROW, next five entries are the second ROW, etc.) */


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
        ss << "  x: " << x << std::endl;
        ss << "  y: " << y << std::endl;
        ss << "  z: " << z << std::endl;
        ss << "  covariance: [" << to_string(covariance) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << q;                             // offset: 8
        map << x;                             // offset: 24
        map << y;                             // offset: 28
        map << z;                             // offset: 32
        map << covariance;                    // offset: 36
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> q;                             // offset: 8
        map >> x;                             // offset: 24
        map >> y;                             // offset: 28
        map >> z;                             // offset: 32
        map >> covariance;                    // offset: 36
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
