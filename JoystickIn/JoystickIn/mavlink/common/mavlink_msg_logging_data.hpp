// MESSAGE LOGGING_DATA support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief LOGGING_DATA message
 *
 * A message containing logged data (see also MAV_CMD_LOGGING_START)
 */
struct LOGGING_DATA : mavlink::Message {
    static constexpr msgid_t MSG_ID = 266;
    static constexpr size_t LENGTH = 255;
    static constexpr size_t MIN_LENGTH = 255;
    static constexpr uint8_t CRC_EXTRA = 193;
    static constexpr auto NAME = "LOGGING_DATA";


    uint8_t target_system; /*<  system ID of the target */
    uint8_t target_component; /*<  component ID of the target */
    uint16_t sequence; /*<  sequence number (can wrap) */
    uint8_t length; /*< [bytes] data length */
    uint8_t first_message_offset; /*< [bytes] offset into data where first message starts. This can be used for recovery, when a previous message got lost (set to 255 if no start exists). */
    std::array<uint8_t, 249> data; /*<  logged data */


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
        ss << "  length: " << +length << std::endl;
        ss << "  first_message_offset: " << +first_message_offset << std::endl;
        ss << "  data: [" << to_string(data) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << sequence;                      // offset: 0
        map << target_system;                 // offset: 2
        map << target_component;              // offset: 3
        map << length;                        // offset: 4
        map << first_message_offset;          // offset: 5
        map << data;                          // offset: 6
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> sequence;                      // offset: 0
        map >> target_system;                 // offset: 2
        map >> target_component;              // offset: 3
        map >> length;                        // offset: 4
        map >> first_message_offset;          // offset: 5
        map >> data;                          // offset: 6
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
