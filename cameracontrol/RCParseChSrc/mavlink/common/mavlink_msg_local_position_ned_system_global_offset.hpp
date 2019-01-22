// MESSAGE LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET message
 *
 * The offset in X, Y, Z and yaw between the LOCAL_POSITION_NED messages of MAV X and the global coordinate frame in NED coordinates. Coordinate frame is right-handed, Z-axis down (aeronautical frame, NED / north-east-down convention)
 */
struct LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET : mavlink::Message {
    static constexpr msgid_t MSG_ID = 89;
    static constexpr size_t LENGTH = 28;
    static constexpr size_t MIN_LENGTH = 28;
    static constexpr uint8_t CRC_EXTRA = 231;
    static constexpr auto NAME = "LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET";


    uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot). */
    float x; /*< [m] X Position */
    float y; /*< [m] Y Position */
    float z; /*< [m] Z Position */
    float roll; /*< [rad] Roll */
    float pitch; /*< [rad] Pitch */
    float yaw; /*< [rad] Yaw */


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
        ss << "  x: " << x << std::endl;
        ss << "  y: " << y << std::endl;
        ss << "  z: " << z << std::endl;
        ss << "  roll: " << roll << std::endl;
        ss << "  pitch: " << pitch << std::endl;
        ss << "  yaw: " << yaw << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_boot_ms;                  // offset: 0
        map << x;                             // offset: 4
        map << y;                             // offset: 8
        map << z;                             // offset: 12
        map << roll;                          // offset: 16
        map << pitch;                         // offset: 20
        map << yaw;                           // offset: 24
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_boot_ms;                  // offset: 0
        map >> x;                             // offset: 4
        map >> y;                             // offset: 8
        map >> z;                             // offset: 12
        map >> roll;                          // offset: 16
        map >> pitch;                         // offset: 20
        map >> yaw;                           // offset: 24
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
