// MESSAGE LOGGING_ACK support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief LOGGING_ACK message
 *
 * An ack for a LOGGING_DATA_ACKED message
 */
struct LOGGING_ACK : mavlink::Message {
    static constexpr msgid_t MSG_ID = 268;
    static constexpr size_t LENGTH = 4;
    static constexpr size_t MIN_LENGTH = 4;
    static constexpr uint8_t CRC_EXTRA = 14;
    static constexpr auto NAME = "LOGGING_ACK";


    uint8_t target_system; /*<  system ID of the target */
    uint8_t target_component; /*<  component ID of the target */
    uint16_t sequence; /*<  sequence number (must match the one in LOGGING_DATA_ACKED) */


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
        ss << "  sequence: " << sequence << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << sequence;                      // offset: 0
        map << target_system;                 // offset: 2
        map << target_component;              // offset: 3
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> sequence;                      // offset: 0
        map >> target_system;                 // offset: 2
        map >> target_component;              // offset: 3
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
