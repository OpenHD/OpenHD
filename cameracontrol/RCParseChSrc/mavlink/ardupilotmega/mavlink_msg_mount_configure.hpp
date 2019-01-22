// MESSAGE MOUNT_CONFIGURE support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief MOUNT_CONFIGURE message
 *
 * Message to configure a camera mount, directional antenna, etc.
 */
struct MOUNT_CONFIGURE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 156;
    static constexpr size_t LENGTH = 6;
    static constexpr size_t MIN_LENGTH = 6;
    static constexpr uint8_t CRC_EXTRA = 19;
    static constexpr auto NAME = "MOUNT_CONFIGURE";


    uint8_t target_system; /*<  System ID. */
    uint8_t target_component; /*<  Component ID. */
    uint8_t mount_mode; /*<  Mount operating mode. */
    uint8_t stab_roll; /*<  (1 = yes, 0 = no). */
    uint8_t stab_pitch; /*<  (1 = yes, 0 = no). */
    uint8_t stab_yaw; /*<  (1 = yes, 0 = no). */


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
        ss << "  mount_mode: " << +mount_mode << std::endl;
        ss << "  stab_roll: " << +stab_roll << std::endl;
        ss << "  stab_pitch: " << +stab_pitch << std::endl;
        ss << "  stab_yaw: " << +stab_yaw << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << target_system;                 // offset: 0
        map << target_component;              // offset: 1
        map << mount_mode;                    // offset: 2
        map << stab_roll;                     // offset: 3
        map << stab_pitch;                    // offset: 4
        map << stab_yaw;                      // offset: 5
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> target_system;                 // offset: 0
        map >> target_component;              // offset: 1
        map >> mount_mode;                    // offset: 2
        map >> stab_roll;                     // offset: 3
        map >> stab_pitch;                    // offset: 4
        map >> stab_yaw;                      // offset: 5
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
