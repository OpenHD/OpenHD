// MESSAGE MOUNT_ORIENTATION support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief MOUNT_ORIENTATION message
 *
 * Orientation of a mount
 */
struct MOUNT_ORIENTATION : mavlink::Message {
    static constexpr msgid_t MSG_ID = 265;
    static constexpr size_t LENGTH = 20;
    static constexpr size_t MIN_LENGTH = 16;
    static constexpr uint8_t CRC_EXTRA = 26;
    static constexpr auto NAME = "MOUNT_ORIENTATION";


    uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot). */
    float roll; /*< [deg] Roll in global frame (set to NaN for invalid). */
    float pitch; /*< [deg] Pitch in global frame (set to NaN for invalid). */
    float yaw; /*< [deg] Yaw relative to vehicle(set to NaN for invalid). */
    float yaw_absolute; /*< [deg] Yaw in absolute frame, North is 0 (set to NaN for invalid). */


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
        ss << "  roll: " << roll << std::endl;
        ss << "  pitch: " << pitch << std::endl;
        ss << "  yaw: " << yaw << std::endl;
        ss << "  yaw_absolute: " << yaw_absolute << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_boot_ms;                  // offset: 0
        map << roll;                          // offset: 4
        map << pitch;                         // offset: 8
        map << yaw;                           // offset: 12
        map << yaw_absolute;                  // offset: 16
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_boot_ms;                  // offset: 0
        map >> roll;                          // offset: 4
        map >> pitch;                         // offset: 8
        map >> yaw;                           // offset: 12
        map >> yaw_absolute;                  // offset: 16
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
