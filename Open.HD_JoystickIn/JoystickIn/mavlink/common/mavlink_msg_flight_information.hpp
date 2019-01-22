// MESSAGE FLIGHT_INFORMATION support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief FLIGHT_INFORMATION message
 *
 * Information about flight since last arming.
 */
struct FLIGHT_INFORMATION : mavlink::Message {
    static constexpr msgid_t MSG_ID = 264;
    static constexpr size_t LENGTH = 28;
    static constexpr size_t MIN_LENGTH = 28;
    static constexpr uint8_t CRC_EXTRA = 49;
    static constexpr auto NAME = "FLIGHT_INFORMATION";


    uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot). */
    uint64_t arming_time_utc; /*< [us] Timestamp at arming (time since UNIX epoch) in UTC, 0 for unknown */
    uint64_t takeoff_time_utc; /*< [us] Timestamp at takeoff (time since UNIX epoch) in UTC, 0 for unknown */
    uint64_t flight_uuid; /*<  Universally unique identifier (UUID) of flight, should correspond to name of log files */


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
        ss << "  arming_time_utc: " << arming_time_utc << std::endl;
        ss << "  takeoff_time_utc: " << takeoff_time_utc << std::endl;
        ss << "  flight_uuid: " << flight_uuid << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << arming_time_utc;               // offset: 0
        map << takeoff_time_utc;              // offset: 8
        map << flight_uuid;                   // offset: 16
        map << time_boot_ms;                  // offset: 24
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> arming_time_utc;               // offset: 0
        map >> takeoff_time_utc;              // offset: 8
        map >> flight_uuid;                   // offset: 16
        map >> time_boot_ms;                  // offset: 24
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
