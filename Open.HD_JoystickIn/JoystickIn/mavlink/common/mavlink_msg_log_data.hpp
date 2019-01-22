// MESSAGE LOG_DATA support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief LOG_DATA message
 *
 * Reply to LOG_REQUEST_DATA
 */
struct LOG_DATA : mavlink::Message {
    static constexpr msgid_t MSG_ID = 120;
    static constexpr size_t LENGTH = 97;
    static constexpr size_t MIN_LENGTH = 97;
    static constexpr uint8_t CRC_EXTRA = 134;
    static constexpr auto NAME = "LOG_DATA";


    uint16_t id; /*<  Log id (from LOG_ENTRY reply) */
    uint32_t ofs; /*<  Offset into the log */
    uint8_t count; /*< [bytes] Number of bytes (zero for end of log) */
    std::array<uint8_t, 90> data; /*<  log data */


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
        ss << "  id: " << id << std::endl;
        ss << "  ofs: " << ofs << std::endl;
        ss << "  count: " << +count << std::endl;
        ss << "  data: [" << to_string(data) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << ofs;                           // offset: 0
        map << id;                            // offset: 4
        map << count;                         // offset: 6
        map << data;                          // offset: 7
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> ofs;                           // offset: 0
        map >> id;                            // offset: 4
        map >> count;                         // offset: 6
        map >> data;                          // offset: 7
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
