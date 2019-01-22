// MESSAGE TIMESYNC support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief TIMESYNC message
 *
 * Time synchronization message.
 */
struct TIMESYNC : mavlink::Message {
    static constexpr msgid_t MSG_ID = 111;
    static constexpr size_t LENGTH = 16;
    static constexpr size_t MIN_LENGTH = 16;
    static constexpr uint8_t CRC_EXTRA = 34;
    static constexpr auto NAME = "TIMESYNC";


    int64_t tc1; /*<  Time sync timestamp 1 */
    int64_t ts1; /*<  Time sync timestamp 2 */


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
        ss << "  tc1: " << tc1 << std::endl;
        ss << "  ts1: " << ts1 << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << tc1;                           // offset: 0
        map << ts1;                           // offset: 8
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> tc1;                           // offset: 0
        map >> ts1;                           // offset: 8
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
