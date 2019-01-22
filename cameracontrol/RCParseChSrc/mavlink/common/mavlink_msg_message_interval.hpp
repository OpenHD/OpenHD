// MESSAGE MESSAGE_INTERVAL support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief MESSAGE_INTERVAL message
 *
 * The interval between messages for a particular MAVLink message ID. This interface replaces DATA_STREAM
 */
struct MESSAGE_INTERVAL : mavlink::Message {
    static constexpr msgid_t MSG_ID = 244;
    static constexpr size_t LENGTH = 6;
    static constexpr size_t MIN_LENGTH = 6;
    static constexpr uint8_t CRC_EXTRA = 95;
    static constexpr auto NAME = "MESSAGE_INTERVAL";


    uint16_t message_id; /*<  The ID of the requested MAVLink message. v1.0 is limited to 254 messages. */
    int32_t interval_us; /*< [us] The interval between two messages. A value of -1 indicates this stream is disabled, 0 indicates it is not available, > 0 indicates the interval at which it is sent. */


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
        ss << "  message_id: " << message_id << std::endl;
        ss << "  interval_us: " << interval_us << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << interval_us;                   // offset: 0
        map << message_id;                    // offset: 4
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> interval_us;                   // offset: 0
        map >> message_id;                    // offset: 4
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
