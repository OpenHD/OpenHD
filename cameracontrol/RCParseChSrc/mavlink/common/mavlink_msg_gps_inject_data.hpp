// MESSAGE GPS_INJECT_DATA support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief GPS_INJECT_DATA message
 *
 * data for injecting into the onboard GPS (used for DGPS)
 */
struct GPS_INJECT_DATA : mavlink::Message {
    static constexpr msgid_t MSG_ID = 123;
    static constexpr size_t LENGTH = 113;
    static constexpr size_t MIN_LENGTH = 113;
    static constexpr uint8_t CRC_EXTRA = 250;
    static constexpr auto NAME = "GPS_INJECT_DATA";


    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
    uint8_t len; /*< [bytes] data length */
    std::array<uint8_t, 110> data; /*<  raw data (110 is enough for 12 satellites of RTCMv2) */


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
        ss << "  len: " << +len << std::endl;
        ss << "  data: [" << to_string(data) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << target_system;                 // offset: 0
        map << target_component;              // offset: 1
        map << len;                           // offset: 2
        map << data;                          // offset: 3
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> target_system;                 // offset: 0
        map >> target_component;              // offset: 1
        map >> len;                           // offset: 2
        map >> data;                          // offset: 3
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
