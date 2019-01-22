// MESSAGE MOUNT_CONTROL support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief MOUNT_CONTROL message
 *
 * Message to control a camera mount, directional antenna, etc.
 */
struct MOUNT_CONTROL : mavlink::Message {
    static constexpr msgid_t MSG_ID = 157;
    static constexpr size_t LENGTH = 15;
    static constexpr size_t MIN_LENGTH = 15;
    static constexpr uint8_t CRC_EXTRA = 21;
    static constexpr auto NAME = "MOUNT_CONTROL";


    uint8_t target_system; /*<  System ID. */
    uint8_t target_component; /*<  Component ID. */
    int32_t input_a; /*<  Pitch (centi-degrees) or lat (degE7), depending on mount mode. */
    int32_t input_b; /*<  Roll (centi-degrees) or lon (degE7) depending on mount mode. */
    int32_t input_c; /*<  Yaw (centi-degrees) or alt (cm) depending on mount mode. */
    uint8_t save_position; /*<  If "1" it will save current trimmed position on EEPROM (just valid for NEUTRAL and LANDING). */


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
        ss << "  input_a: " << input_a << std::endl;
        ss << "  input_b: " << input_b << std::endl;
        ss << "  input_c: " << input_c << std::endl;
        ss << "  save_position: " << +save_position << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << input_a;                       // offset: 0
        map << input_b;                       // offset: 4
        map << input_c;                       // offset: 8
        map << target_system;                 // offset: 12
        map << target_component;              // offset: 13
        map << save_position;                 // offset: 14
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> input_a;                       // offset: 0
        map >> input_b;                       // offset: 4
        map >> input_c;                       // offset: 8
        map >> target_system;                 // offset: 12
        map >> target_component;              // offset: 13
        map >> save_position;                 // offset: 14
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
