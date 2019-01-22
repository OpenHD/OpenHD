// MESSAGE LED_CONTROL support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief LED_CONTROL message
 *
 * Control vehicle LEDs.
 */
struct LED_CONTROL : mavlink::Message {
    static constexpr msgid_t MSG_ID = 186;
    static constexpr size_t LENGTH = 29;
    static constexpr size_t MIN_LENGTH = 29;
    static constexpr uint8_t CRC_EXTRA = 72;
    static constexpr auto NAME = "LED_CONTROL";


    uint8_t target_system; /*<  System ID. */
    uint8_t target_component; /*<  Component ID. */
    uint8_t instance; /*<  Instance (LED instance to control or 255 for all LEDs). */
    uint8_t pattern; /*<  Pattern (see LED_PATTERN_ENUM). */
    uint8_t custom_len; /*<  Custom Byte Length. */
    std::array<uint8_t, 24> custom_bytes; /*<  Custom Bytes. */


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
        ss << "  instance: " << +instance << std::endl;
        ss << "  pattern: " << +pattern << std::endl;
        ss << "  custom_len: " << +custom_len << std::endl;
        ss << "  custom_bytes: [" << to_string(custom_bytes) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << target_system;                 // offset: 0
        map << target_component;              // offset: 1
        map << instance;                      // offset: 2
        map << pattern;                       // offset: 3
        map << custom_len;                    // offset: 4
        map << custom_bytes;                  // offset: 5
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> target_system;                 // offset: 0
        map >> target_component;              // offset: 1
        map >> instance;                      // offset: 2
        map >> pattern;                       // offset: 3
        map >> custom_len;                    // offset: 4
        map >> custom_bytes;                  // offset: 5
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
