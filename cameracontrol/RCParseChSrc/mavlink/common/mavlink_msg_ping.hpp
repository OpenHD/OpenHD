// MESSAGE PING support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief PING message
 *
 * A ping message either requesting or responding to a ping. This allows to measure the system latencies, including serial port, radio modem and UDP connections.
 */
struct PING : mavlink::Message {
    static constexpr msgid_t MSG_ID = 4;
    static constexpr size_t LENGTH = 14;
    static constexpr size_t MIN_LENGTH = 14;
    static constexpr uint8_t CRC_EXTRA = 237;
    static constexpr auto NAME = "PING";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    uint32_t seq; /*<  PING sequence */
    uint8_t target_system; /*<  0: request ping from all receiving systems, if greater than 0: message is a ping response and number is the system id of the requesting system */
    uint8_t target_component; /*<  0: request ping from all receiving components, if greater than 0: message is a ping response and number is the system id of the requesting system */


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
        ss << "  time_usec: " << time_usec << std::endl;
        ss << "  seq: " << seq << std::endl;
        ss << "  target_system: " << +target_system << std::endl;
        ss << "  target_component: " << +target_component << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << seq;                           // offset: 8
        map << target_system;                 // offset: 12
        map << target_component;              // offset: 13
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> seq;                           // offset: 8
        map >> target_system;                 // offset: 12
        map >> target_component;              // offset: 13
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
