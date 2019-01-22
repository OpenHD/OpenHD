// MESSAGE LOG_REQUEST_DATA support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief LOG_REQUEST_DATA message
 *
 * Request a chunk of a log
 */
struct LOG_REQUEST_DATA : mavlink::Message {
    static constexpr msgid_t MSG_ID = 119;
    static constexpr size_t LENGTH = 12;
    static constexpr size_t MIN_LENGTH = 12;
    static constexpr uint8_t CRC_EXTRA = 116;
    static constexpr auto NAME = "LOG_REQUEST_DATA";


    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
    uint16_t id; /*<  Log id (from LOG_ENTRY reply) */
    uint32_t ofs; /*<  Offset into the log */
    uint32_t count; /*< [bytes] Number of bytes */


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
        ss << "  id: " << id << std::endl;
        ss << "  ofs: " << ofs << std::endl;
        ss << "  count: " << count << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << ofs;                           // offset: 0
        map << count;                         // offset: 4
        map << id;                            // offset: 8
        map << target_system;                 // offset: 10
        map << target_component;              // offset: 11
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> ofs;                           // offset: 0
        map >> count;                         // offset: 4
        map >> id;                            // offset: 8
        map >> target_system;                 // offset: 10
        map >> target_component;              // offset: 11
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
