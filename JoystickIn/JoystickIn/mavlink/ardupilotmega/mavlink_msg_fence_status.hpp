// MESSAGE FENCE_STATUS support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief FENCE_STATUS message
 *
 * Status of geo-fencing. Sent in extended status stream when fencing enabled.
 */
struct FENCE_STATUS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 162;
    static constexpr size_t LENGTH = 8;
    static constexpr size_t MIN_LENGTH = 8;
    static constexpr uint8_t CRC_EXTRA = 189;
    static constexpr auto NAME = "FENCE_STATUS";


    uint8_t breach_status; /*<  Breach status (0 if currently inside fence, 1 if outside). */
    uint16_t breach_count; /*<  Number of fence breaches. */
    uint8_t breach_type; /*<  Last breach type. */
    uint32_t breach_time; /*< [ms] Time (since boot) of last breach. */


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
        ss << "  breach_status: " << +breach_status << std::endl;
        ss << "  breach_count: " << breach_count << std::endl;
        ss << "  breach_type: " << +breach_type << std::endl;
        ss << "  breach_time: " << breach_time << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << breach_time;                   // offset: 0
        map << breach_count;                  // offset: 4
        map << breach_status;                 // offset: 6
        map << breach_type;                   // offset: 7
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> breach_time;                   // offset: 0
        map >> breach_count;                  // offset: 4
        map >> breach_status;                 // offset: 6
        map >> breach_type;                   // offset: 7
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
