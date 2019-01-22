// MESSAGE ICAROUS_HEARTBEAT support class

#pragma once

namespace mavlink {
namespace icarous {
namespace msg {

/**
 * @brief ICAROUS_HEARTBEAT message
 *
 * ICAROUS heartbeat
 */
struct ICAROUS_HEARTBEAT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 42000;
    static constexpr size_t LENGTH = 1;
    static constexpr size_t MIN_LENGTH = 1;
    static constexpr uint8_t CRC_EXTRA = 227;
    static constexpr auto NAME = "ICAROUS_HEARTBEAT";


    uint8_t status; /*<  See the FMS_STATE enum. */


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
        ss << "  status: " << +status << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << status;                        // offset: 0
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> status;                        // offset: 0
    }
};

} // namespace msg
} // namespace icarous
} // namespace mavlink
