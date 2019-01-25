// MESSAGE VIBRATION support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief VIBRATION message
 *
 * Vibration levels and accelerometer clipping
 */
struct VIBRATION : mavlink::Message {
    static constexpr msgid_t MSG_ID = 241;
    static constexpr size_t LENGTH = 32;
    static constexpr size_t MIN_LENGTH = 32;
    static constexpr uint8_t CRC_EXTRA = 90;
    static constexpr auto NAME = "VIBRATION";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    float vibration_x; /*<  Vibration levels on X-axis */
    float vibration_y; /*<  Vibration levels on Y-axis */
    float vibration_z; /*<  Vibration levels on Z-axis */
    uint32_t clipping_0; /*<  first accelerometer clipping count */
    uint32_t clipping_1; /*<  second accelerometer clipping count */
    uint32_t clipping_2; /*<  third accelerometer clipping count */


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
        ss << "  vibration_x: " << vibration_x << std::endl;
        ss << "  vibration_y: " << vibration_y << std::endl;
        ss << "  vibration_z: " << vibration_z << std::endl;
        ss << "  clipping_0: " << clipping_0 << std::endl;
        ss << "  clipping_1: " << clipping_1 << std::endl;
        ss << "  clipping_2: " << clipping_2 << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << vibration_x;                   // offset: 8
        map << vibration_y;                   // offset: 12
        map << vibration_z;                   // offset: 16
        map << clipping_0;                    // offset: 20
        map << clipping_1;                    // offset: 24
        map << clipping_2;                    // offset: 28
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> vibration_x;                   // offset: 8
        map >> vibration_y;                   // offset: 12
        map >> vibration_z;                   // offset: 16
        map >> clipping_0;                    // offset: 20
        map >> clipping_1;                    // offset: 24
        map >> clipping_2;                    // offset: 28
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
