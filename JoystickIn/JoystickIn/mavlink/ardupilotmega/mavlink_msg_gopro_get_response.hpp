// MESSAGE GOPRO_GET_RESPONSE support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief GOPRO_GET_RESPONSE message
 *
 * Response from a GOPRO_COMMAND get request.
 */
struct GOPRO_GET_RESPONSE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 217;
    static constexpr size_t LENGTH = 6;
    static constexpr size_t MIN_LENGTH = 6;
    static constexpr uint8_t CRC_EXTRA = 202;
    static constexpr auto NAME = "GOPRO_GET_RESPONSE";


    uint8_t cmd_id; /*<  Command ID. */
    uint8_t status; /*<  Status. */
    std::array<uint8_t, 4> value; /*<  Value. */


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
        ss << "  value: [" << to_string(value) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << cmd_id;                        // offset: 0
        map << status;                        // offset: 1
        map << value;                         // offset: 2
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> cmd_id;                        // offset: 0
        map >> status;                        // offset: 1
        map >> value;                         // offset: 2
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
