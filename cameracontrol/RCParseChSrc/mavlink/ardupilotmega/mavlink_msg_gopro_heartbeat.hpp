// MESSAGE GOPRO_HEARTBEAT support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief GOPRO_HEARTBEAT message
 *
 * Heartbeat from a HeroBus attached GoPro.
 */
struct GOPRO_HEARTBEAT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 215;
    static constexpr size_t LENGTH = 3;
    static constexpr size_t MIN_LENGTH = 3;
    static constexpr uint8_t CRC_EXTRA = 101;
    static constexpr auto NAME = "GOPRO_HEARTBEAT";


    uint8_t status; /*<  Status. */
    uint8_t capture_mode; /*<  Current capture mode. */
    uint8_t flags; /*<  Additional status bits. */


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
        ss << "  capture_mode: " << +capture_mode << std::endl;
        ss << "  flags: " << +flags << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << status;                        // offset: 0
        map << capture_mode;                  // offset: 1
        map << flags;                         // offset: 2
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> status;                        // offset: 0
        map >> capture_mode;                  // offset: 1
        map >> flags;                         // offset: 2
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
