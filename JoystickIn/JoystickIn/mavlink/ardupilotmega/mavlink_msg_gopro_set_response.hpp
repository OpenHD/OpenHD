// MESSAGE GOPRO_SET_RESPONSE support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief GOPRO_SET_RESPONSE message
 *
 * Response from a GOPRO_COMMAND set request.
 */
struct GOPRO_SET_RESPONSE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 219;
    static constexpr size_t LENGTH = 2;
    static constexpr size_t MIN_LENGTH = 2;
    static constexpr uint8_t CRC_EXTRA = 162;
    static constexpr auto NAME = "GOPRO_SET_RESPONSE";


    uint8_t cmd_id; /*<  Command ID. */
    uint8_t status; /*<  Status. */


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
        ss << "  cmd_id: " << +cmd_id << std::endl;
        ss << "  status: " << +status << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << cmd_id;                        // offset: 0
        map << status;                        // offset: 1
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> cmd_id;                        // offset: 0
        map >> status;                        // offset: 1
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
