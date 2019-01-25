// MESSAGE REMOTE_LOG_DATA_BLOCK support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief REMOTE_LOG_DATA_BLOCK message
 *
 * Send a block of log data to remote location.
 */
struct REMOTE_LOG_DATA_BLOCK : mavlink::Message {
    static constexpr msgid_t MSG_ID = 184;
    static constexpr size_t LENGTH = 206;
    static constexpr size_t MIN_LENGTH = 206;
    static constexpr uint8_t CRC_EXTRA = 159;
    static constexpr auto NAME = "REMOTE_LOG_DATA_BLOCK";


    uint8_t target_system; /*<  System ID. */
    uint8_t target_component; /*<  Component ID. */
    uint32_t seqno; /*<  Log data block sequence number. */
    std::array<uint8_t, 200> data; /*<  Log data block. */


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
        ss << "  seqno: " << seqno << std::endl;
        ss << "  data: [" << to_string(data) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << seqno;                         // offset: 0
        map << target_system;                 // offset: 4
        map << target_component;              // offset: 5
        map << data;                          // offset: 6
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> seqno;                         // offset: 0
        map >> target_system;                 // offset: 4
        map >> target_component;              // offset: 5
        map >> data;                          // offset: 6
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
