// MESSAGE GIMBAL_CONTROL support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief GIMBAL_CONTROL message
 *
 * Control message for rate gimbal.
 */
struct GIMBAL_CONTROL : mavlink::Message {
    static constexpr msgid_t MSG_ID = 201;
    static constexpr size_t LENGTH = 14;
    static constexpr size_t MIN_LENGTH = 14;
    static constexpr uint8_t CRC_EXTRA = 205;
    static constexpr auto NAME = "GIMBAL_CONTROL";


    uint8_t target_system; /*<  System ID. */
    uint8_t target_component; /*<  Component ID. */
    float demanded_rate_x; /*< [rad/s] Demanded angular rate X. */
    float demanded_rate_y; /*< [rad/s] Demanded angular rate Y. */
    float demanded_rate_z; /*< [rad/s] Demanded angular rate Z. */


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
        ss << "  demanded_rate_x: " << demanded_rate_x << std::endl;
        ss << "  demanded_rate_y: " << demanded_rate_y << std::endl;
        ss << "  demanded_rate_z: " << demanded_rate_z << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << demanded_rate_x;               // offset: 0
        map << demanded_rate_y;               // offset: 4
        map << demanded_rate_z;               // offset: 8
        map << target_system;                 // offset: 12
        map << target_component;              // offset: 13
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> demanded_rate_x;               // offset: 0
        map >> demanded_rate_y;               // offset: 4
        map >> demanded_rate_z;               // offset: 8
        map >> target_system;                 // offset: 12
        map >> target_component;              // offset: 13
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
