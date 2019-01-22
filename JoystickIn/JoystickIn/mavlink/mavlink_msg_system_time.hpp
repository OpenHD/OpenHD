// MESSAGE SYSTEM_TIME support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief SYSTEM_TIME message
 *
 * The system time is the time of the master clock, typically the computer clock of the main onboard computer.
 */
struct SYSTEM_TIME : mavlink::Message {
    static constexpr msgid_t MSG_ID = 2;
    static constexpr size_t LENGTH = 12;
    static constexpr size_t MIN_LENGTH = 12;
    static constexpr uint8_t CRC_EXTRA = 137;
    static constexpr auto NAME = "SYSTEM_TIME";


    uint64_t time_unix_usec; /*< [us] Timestamp (UNIX epoch time). */
    uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot). */


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
        ss << "  time_unix_usec: " << time_unix_usec << std::endl;
        ss << "  time_boot_ms: " << time_boot_ms << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_unix_usec;                // offset: 0
        map << time_boot_ms;                  // offset: 8
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_unix_usec;                // offset: 0
        map >> time_boot_ms;                  // offset: 8
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
