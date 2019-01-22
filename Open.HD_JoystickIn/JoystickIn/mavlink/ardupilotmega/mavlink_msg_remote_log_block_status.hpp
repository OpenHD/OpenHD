// MESSAGE REMOTE_LOG_BLOCK_STATUS support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief REMOTE_LOG_BLOCK_STATUS message
 *
 * Send Status of each log block that autopilot board might have sent.
 */
struct REMOTE_LOG_BLOCK_STATUS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 185;
    static constexpr size_t LENGTH = 7;
    static constexpr size_t MIN_LENGTH = 7;
    static constexpr uint8_t CRC_EXTRA = 186;
    static constexpr auto NAME = "REMOTE_LOG_BLOCK_STATUS";


    uint8_t target_system; /*<  System ID. */
    uint8_t target_component; /*<  Component ID. */
    uint32_t seqno; /*<  Log data block sequence number. */
    uint8_t status; /*<  Log data block status. */


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
        ss << "  status: " << +status << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << seqno;                         // offset: 0
        map << target_system;                 // offset: 4
        map << target_component;              // offset: 5
        map << status;                        // offset: 6
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> seqno;                         // offset: 0
        map >> target_system;                 // offset: 4
        map >> target_component;              // offset: 5
        map >> status;                        // offset: 6
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
