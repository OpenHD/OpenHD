// MESSAGE AUTH_KEY support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief AUTH_KEY message
 *
 * Emit an encrypted signature / key identifying this system. PLEASE NOTE: This protocol has been kept simple, so transmitting the key requires an encrypted channel for true safety.
 */
struct AUTH_KEY : mavlink::Message {
    static constexpr msgid_t MSG_ID = 7;
    static constexpr size_t LENGTH = 32;
    static constexpr size_t MIN_LENGTH = 32;
    static constexpr uint8_t CRC_EXTRA = 119;
    static constexpr auto NAME = "AUTH_KEY";


    std::array<char, 32> key; /*<  key */


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
        ss << "  key: \"" << to_string(key) << "\"" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << key;                           // offset: 0
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> key;                           // offset: 0
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
