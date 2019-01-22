// MESSAGE WIFI_CONFIG_AP support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief WIFI_CONFIG_AP message
 *
 * Configure AP SSID and Password.
 */
struct WIFI_CONFIG_AP : mavlink::Message {
    static constexpr msgid_t MSG_ID = 299;
    static constexpr size_t LENGTH = 96;
    static constexpr size_t MIN_LENGTH = 96;
    static constexpr uint8_t CRC_EXTRA = 19;
    static constexpr auto NAME = "WIFI_CONFIG_AP";


    std::array<char, 32> ssid; /*<  Name of Wi-Fi network (SSID). Leave it blank to leave it unchanged. */
    std::array<char, 64> password; /*<  Password. Leave it blank for an open AP. */


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
        ss << "  ssid: \"" << to_string(ssid) << "\"" << std::endl;
        ss << "  password: \"" << to_string(password) << "\"" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << ssid;                          // offset: 0
        map << password;                      // offset: 32
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> ssid;                          // offset: 0
        map >> password;                      // offset: 32
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
