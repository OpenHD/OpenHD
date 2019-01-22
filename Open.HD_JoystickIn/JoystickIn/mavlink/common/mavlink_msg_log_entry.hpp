// MESSAGE LOG_ENTRY support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief LOG_ENTRY message
 *
 * Reply to LOG_REQUEST_LIST
 */
struct LOG_ENTRY : mavlink::Message {
    static constexpr msgid_t MSG_ID = 118;
    static constexpr size_t LENGTH = 14;
    static constexpr size_t MIN_LENGTH = 14;
    static constexpr uint8_t CRC_EXTRA = 56;
    static constexpr auto NAME = "LOG_ENTRY";


    uint16_t id; /*<  Log id */
    uint16_t num_logs; /*<  Total number of logs */
    uint16_t last_log_num; /*<  High log number */
    uint32_t time_utc; /*< [s] UTC timestamp of log since 1970, or 0 if not available */
    uint32_t size; /*< [bytes] Size of the log (may be approximate) */


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
        ss << "  num_logs: " << num_logs << std::endl;
        ss << "  last_log_num: " << last_log_num << std::endl;
        ss << "  time_utc: " << time_utc << std::endl;
        ss << "  size: " << size << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_utc;                      // offset: 0
        map << size;                          // offset: 4
        map << id;                            // offset: 8
        map << num_logs;                      // offset: 10
        map << last_log_num;                  // offset: 12
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_utc;                      // offset: 0
        map >> size;                          // offset: 4
        map >> id;                            // offset: 8
        map >> num_logs;                      // offset: 10
        map >> last_log_num;                  // offset: 12
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
