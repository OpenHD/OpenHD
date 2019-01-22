// MESSAGE CAMERA_TRIGGER support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief CAMERA_TRIGGER message
 *
 * Camera-IMU triggering and synchronisation message.
 */
struct CAMERA_TRIGGER : mavlink::Message {
    static constexpr msgid_t MSG_ID = 112;
    static constexpr size_t LENGTH = 12;
    static constexpr size_t MIN_LENGTH = 12;
    static constexpr uint8_t CRC_EXTRA = 174;
    static constexpr auto NAME = "CAMERA_TRIGGER";


    uint64_t time_usec; /*< [us] Timestamp for image frame (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    uint32_t seq; /*<  Image frame sequence */


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

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << seq;                           // offset: 8
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> seq;                           // offset: 8
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
