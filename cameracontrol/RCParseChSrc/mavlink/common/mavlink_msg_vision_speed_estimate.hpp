// MESSAGE VISION_SPEED_ESTIMATE support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief VISION_SPEED_ESTIMATE message
 *
 * 
 */
struct VISION_SPEED_ESTIMATE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 103;
    static constexpr size_t LENGTH = 56;
    static constexpr size_t MIN_LENGTH = 20;
    static constexpr uint8_t CRC_EXTRA = 208;
    static constexpr auto NAME = "VISION_SPEED_ESTIMATE";


    uint64_t usec; /*< [us] Timestamp (UNIX time or time since system boot) */
    float x; /*< [m/s] Global X speed */
    float y; /*< [m/s] Global Y speed */
    float z; /*< [m/s] Global Z speed */
    std::array<float, 9> covariance; /*<  Linear velocity covariance matrix (1st three entries - 1st row, etc.) */


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
        ss << "  usec: " << usec << std::endl;
        ss << "  x: " << x << std::endl;
        ss << "  y: " << y << std::endl;
        ss << "  z: " << z << std::endl;
        ss << "  covariance: [" << to_string(covariance) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << usec;                          // offset: 0
        map << x;                             // offset: 8
        map << y;                             // offset: 12
        map << z;                             // offset: 16
        map << covariance;                    // offset: 20
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> usec;                          // offset: 0
        map >> x;                             // offset: 8
        map >> y;                             // offset: 12
        map >> z;                             // offset: 16
        map >> covariance;                    // offset: 20
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
