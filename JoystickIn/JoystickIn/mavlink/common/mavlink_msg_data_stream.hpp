// MESSAGE DATA_STREAM support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief DATA_STREAM message
 *
 * Data stream status information.
 */
struct DATA_STREAM : mavlink::Message {
    static constexpr msgid_t MSG_ID = 67;
    static constexpr size_t LENGTH = 4;
    static constexpr size_t MIN_LENGTH = 4;
    static constexpr uint8_t CRC_EXTRA = 21;
    static constexpr auto NAME = "DATA_STREAM";


    uint8_t stream_id; /*<  The ID of the requested data stream */
    uint16_t message_rate; /*< [Hz] The message rate */
    uint8_t on_off; /*<  1 stream is enabled, 0 stream is stopped. */


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
        ss << "  stream_id: " << +stream_id << std::endl;
        ss << "  message_rate: " << message_rate << std::endl;
        ss << "  on_off: " << +on_off << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << message_rate;                  // offset: 0
        map << stream_id;                     // offset: 2
        map << on_off;                        // offset: 3
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> message_rate;                  // offset: 0
        map >> stream_id;                     // offset: 2
        map >> on_off;                        // offset: 3
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
