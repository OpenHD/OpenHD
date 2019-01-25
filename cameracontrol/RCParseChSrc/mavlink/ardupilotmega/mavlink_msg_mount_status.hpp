// MESSAGE MOUNT_STATUS support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief MOUNT_STATUS message
 *
 * Message with some status from APM to GCS about camera or antenna mount.
 */
struct MOUNT_STATUS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 158;
    static constexpr size_t LENGTH = 14;
    static constexpr size_t MIN_LENGTH = 14;
    static constexpr uint8_t CRC_EXTRA = 134;
    static constexpr auto NAME = "MOUNT_STATUS";


    uint8_t target_system; /*<  System ID. */
    uint8_t target_component; /*<  Component ID. */
    int32_t pointing_a; /*< [cdeg] Pitch. */
    int32_t pointing_b; /*< [cdeg] Roll. */
    int32_t pointing_c; /*< [cdeg] Yaw. */


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
        ss << "  target_system: " << +target_system << std::endl;
        ss << "  target_component: " << +target_component << std::endl;
        ss << "  pointing_a: " << pointing_a << std::endl;
        ss << "  pointing_b: " << pointing_b << std::endl;
        ss << "  pointing_c: " << pointing_c << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << pointing_a;                    // offset: 0
        map << pointing_b;                    // offset: 4
        map << pointing_c;                    // offset: 8
        map << target_system;                 // offset: 12
        map << target_component;              // offset: 13
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> pointing_a;                    // offset: 0
        map >> pointing_b;                    // offset: 4
        map >> pointing_c;                    // offset: 8
        map >> target_system;                 // offset: 12
        map >> target_component;              // offset: 13
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
