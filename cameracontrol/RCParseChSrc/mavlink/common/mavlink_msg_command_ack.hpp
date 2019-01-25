// MESSAGE COMMAND_ACK support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief COMMAND_ACK message
 *
 * Report status of a command. Includes feedback whether the command was executed.
 */
struct COMMAND_ACK : mavlink::Message {
    static constexpr msgid_t MSG_ID = 77;
    static constexpr size_t LENGTH = 3;
    static constexpr size_t MIN_LENGTH = 3;
    static constexpr uint8_t CRC_EXTRA = 143;
    static constexpr auto NAME = "COMMAND_ACK";


    uint16_t command; /*<  Command ID (of acknowledged command). */
    uint8_t result; /*<  Result of command. */


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
        ss << "  command: " << command << std::endl;
        ss << "  result: " << +result << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << command;                       // offset: 0
        map << result;                        // offset: 2
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> command;                       // offset: 0
        map >> result;                        // offset: 2
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
