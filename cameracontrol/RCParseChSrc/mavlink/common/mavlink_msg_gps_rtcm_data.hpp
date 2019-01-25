// MESSAGE GPS_RTCM_DATA support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief GPS_RTCM_DATA message
 *
 * RTCM message for injecting into the onboard GPS (used for DGPS)
 */
struct GPS_RTCM_DATA : mavlink::Message {
    static constexpr msgid_t MSG_ID = 233;
    static constexpr size_t LENGTH = 182;
    static constexpr size_t MIN_LENGTH = 182;
    static constexpr uint8_t CRC_EXTRA = 35;
    static constexpr auto NAME = "GPS_RTCM_DATA";


    uint8_t flags; /*<  LSB: 1 means message is fragmented, next 2 bits are the fragment ID, the remaining 5 bits are used for the sequence ID. Messages are only to be flushed to the GPS when the entire message has been reconstructed on the autopilot. The fragment ID specifies which order the fragments should be assembled into a buffer, while the sequence ID is used to detect a mismatch between different buffers. The buffer is considered fully reconstructed when either all 4 fragments are present, or all the fragments before the first fragment with a non full payload is received. This management is used to ensure that normal GPS operation doesn't corrupt RTCM data, and to recover from a unreliable transport delivery order. */
    uint8_t len; /*< [bytes] data length */
    std::array<uint8_t, 180> data; /*<  RTCM message (may be fragmented) */


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
        ss << "  flags: " << +flags << std::endl;
        ss << "  len: " << +len << std::endl;
        ss << "  data: [" << to_string(data) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << flags;                         // offset: 0
        map << len;                           // offset: 1
        map << data;                          // offset: 2
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> flags;                         // offset: 0
        map >> len;                           // offset: 1
        map >> data;                          // offset: 2
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
