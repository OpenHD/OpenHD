// MESSAGE DEVICE_OP_WRITE_REPLY support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief DEVICE_OP_WRITE_REPLY message
 *
 * Write registers reply.
 */
struct DEVICE_OP_WRITE_REPLY : mavlink::Message {
    static constexpr msgid_t MSG_ID = 11003;
    static constexpr size_t LENGTH = 5;
    static constexpr size_t MIN_LENGTH = 5;
    static constexpr uint8_t CRC_EXTRA = 64;
    static constexpr auto NAME = "DEVICE_OP_WRITE_REPLY";


    uint32_t request_id; /*<  Request ID - copied from request. */
    uint8_t result; /*<  0 for success, anything else is failure code. */


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
        ss << "  request_id: " << request_id << std::endl;
        ss << "  result: " << +result << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << request_id;                    // offset: 0
        map << result;                        // offset: 4
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> request_id;                    // offset: 0
        map >> result;                        // offset: 4
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
