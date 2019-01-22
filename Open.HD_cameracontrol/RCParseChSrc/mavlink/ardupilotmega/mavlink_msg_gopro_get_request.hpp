// MESSAGE GOPRO_GET_REQUEST support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief GOPRO_GET_REQUEST message
 *
 * Request a GOPRO_COMMAND response from the GoPro.
 */
struct GOPRO_GET_REQUEST : mavlink::Message {
    static constexpr msgid_t MSG_ID = 216;
    static constexpr size_t LENGTH = 3;
    static constexpr size_t MIN_LENGTH = 3;
    static constexpr uint8_t CRC_EXTRA = 50;
    static constexpr auto NAME = "GOPRO_GET_REQUEST";


    uint8_t target_system; /*<  System ID. */
    uint8_t target_component; /*<  Component ID. */
    uint8_t cmd_id; /*<  Command ID. */


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
        ss << "  cmd_id: " << +cmd_id << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << target_system;                 // offset: 0
        map << target_component;              // offset: 1
        map << cmd_id;                        // offset: 2
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> target_system;                 // offset: 0
        map >> target_component;              // offset: 1
        map >> cmd_id;                        // offset: 2
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
