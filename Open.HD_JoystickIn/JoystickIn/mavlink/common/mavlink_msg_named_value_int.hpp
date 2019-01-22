// MESSAGE NAMED_VALUE_INT support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief NAMED_VALUE_INT message
 *
 * Send a key-value pair as integer. The use of this message is discouraged for normal packets, but a quite efficient way for testing new messages and getting experimental debug output.
 */
struct NAMED_VALUE_INT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 252;
    static constexpr size_t LENGTH = 18;
    static constexpr size_t MIN_LENGTH = 18;
    static constexpr uint8_t CRC_EXTRA = 44;
    static constexpr auto NAME = "NAMED_VALUE_INT";


    uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot). */
    std::array<char, 10> name; /*<  Name of the debug variable */
    int32_t value; /*<  Signed integer value */


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
        ss << "  name: \"" << to_string(name) << "\"" << std::endl;
        ss << "  value: " << value << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_boot_ms;                  // offset: 0
        map << value;                         // offset: 4
        map << name;                          // offset: 8
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_boot_ms;                  // offset: 0
        map >> value;                         // offset: 4
        map >> name;                          // offset: 8
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
