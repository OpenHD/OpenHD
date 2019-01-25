// MESSAGE LOG_REQUEST_LIST support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief LOG_REQUEST_LIST message
 *
 * Request a list of available logs. On some systems calling this may stop on-board logging until LOG_REQUEST_END is called.
 */
struct LOG_REQUEST_LIST : mavlink::Message {
    static constexpr msgid_t MSG_ID = 117;
    static constexpr size_t LENGTH = 6;
    static constexpr size_t MIN_LENGTH = 6;
    static constexpr uint8_t CRC_EXTRA = 128;
    static constexpr auto NAME = "LOG_REQUEST_LIST";


    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
    uint16_t start; /*<  First log id (0 for first available) */
    uint16_t end; /*<  Last log id (0xffff for last available) */


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
        ss << "  start: " << start << std::endl;
        ss << "  end: " << end << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << start;                         // offset: 0
        map << end;                           // offset: 2
        map << target_system;                 // offset: 4
        map << target_component;              // offset: 5
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> start;                         // offset: 0
        map >> end;                           // offset: 2
        map >> target_system;                 // offset: 4
        map >> target_component;              // offset: 5
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
