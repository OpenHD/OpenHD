// MESSAGE BUTTON_CHANGE support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief BUTTON_CHANGE message
 *
 * Report button state change.
 */
struct BUTTON_CHANGE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 257;
    static constexpr size_t LENGTH = 9;
    static constexpr size_t MIN_LENGTH = 9;
    static constexpr uint8_t CRC_EXTRA = 131;
    static constexpr auto NAME = "BUTTON_CHANGE";


    uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot). */
    uint32_t last_change_ms; /*< [ms] Time of last change of button state. */
    uint8_t state; /*<  Bitmap for state of buttons. */


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
        ss << "  last_change_ms: " << last_change_ms << std::endl;
        ss << "  state: " << +state << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_boot_ms;                  // offset: 0
        map << last_change_ms;                // offset: 4
        map << state;                         // offset: 8
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_boot_ms;                  // offset: 0
        map >> last_change_ms;                // offset: 4
        map >> state;                         // offset: 8
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
