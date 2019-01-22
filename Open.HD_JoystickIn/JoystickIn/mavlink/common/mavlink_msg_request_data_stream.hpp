// MESSAGE REQUEST_DATA_STREAM support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief REQUEST_DATA_STREAM message
 *
 * Request a data stream.
 */
struct REQUEST_DATA_STREAM : mavlink::Message {
    static constexpr msgid_t MSG_ID = 66;
    static constexpr size_t LENGTH = 6;
    static constexpr size_t MIN_LENGTH = 6;
    static constexpr uint8_t CRC_EXTRA = 148;
    static constexpr auto NAME = "REQUEST_DATA_STREAM";


    uint8_t target_system; /*<  The target requested to send the message stream. */
    uint8_t target_component; /*<  The target requested to send the message stream. */
    uint8_t req_stream_id; /*<  The ID of the requested data stream */
    uint16_t req_message_rate; /*< [Hz] The requested message rate */
    uint8_t start_stop; /*<  1 to start sending, 0 to stop sending. */


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
        ss << "  req_stream_id: " << +req_stream_id << std::endl;
        ss << "  req_message_rate: " << req_message_rate << std::endl;
        ss << "  start_stop: " << +start_stop << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << req_message_rate;              // offset: 0
        map << target_system;                 // offset: 2
        map << target_component;              // offset: 3
        map << req_stream_id;                 // offset: 4
        map << start_stop;                    // offset: 5
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> req_message_rate;              // offset: 0
        map >> target_system;                 // offset: 2
        map >> target_component;              // offset: 3
        map >> req_stream_id;                 // offset: 4
        map >> start_stop;                    // offset: 5
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
