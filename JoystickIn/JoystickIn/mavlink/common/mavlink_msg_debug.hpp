// MESSAGE DEBUG support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief DEBUG message
 *
 * Send a debug value. The index is used to discriminate between values. These values show up in the plot of QGroundControl as DEBUG N.
 */
struct DEBUG : mavlink::Message {
    static constexpr msgid_t MSG_ID = 254;
    static constexpr size_t LENGTH = 9;
    static constexpr size_t MIN_LENGTH = 9;
    static constexpr uint8_t CRC_EXTRA = 46;
    static constexpr auto NAME = "DEBUG";


    uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot). */
    uint8_t ind; /*<  index of debug variable */
    float value; /*<  DEBUG value */


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
        ss << "  ind: " << +ind << std::endl;
        ss << "  value: " << value << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_boot_ms;                  // offset: 0
        map << value;                         // offset: 4
        map << ind;                           // offset: 8
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_boot_ms;                  // offset: 0
        map >> value;                         // offset: 4
        map >> ind;                           // offset: 8
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
