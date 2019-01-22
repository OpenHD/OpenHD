// MESSAGE RC_CHANNELS_SCALED support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief RC_CHANNELS_SCALED message
 *
 * The scaled values of the RC channels received: (-100%) -10000, (0%) 0, (100%) 10000. Channels that are inactive should be set to UINT16_MAX.
 */
struct RC_CHANNELS_SCALED : mavlink::Message {
    static constexpr msgid_t MSG_ID = 34;
    static constexpr size_t LENGTH = 22;
    static constexpr size_t MIN_LENGTH = 22;
    static constexpr uint8_t CRC_EXTRA = 237;
    static constexpr auto NAME = "RC_CHANNELS_SCALED";


    uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot). */
    uint8_t port; /*<  Servo output port (set of 8 outputs = 1 port). Most MAVs will just use one, but this allows for more than 8 servos. */
    int16_t chan1_scaled; /*<  RC channel 1 value scaled. */
    int16_t chan2_scaled; /*<  RC channel 2 value scaled. */
    int16_t chan3_scaled; /*<  RC channel 3 value scaled. */
    int16_t chan4_scaled; /*<  RC channel 4 value scaled. */
    int16_t chan5_scaled; /*<  RC channel 5 value scaled. */
    int16_t chan6_scaled; /*<  RC channel 6 value scaled. */
    int16_t chan7_scaled; /*<  RC channel 7 value scaled. */
    int16_t chan8_scaled; /*<  RC channel 8 value scaled. */
    uint8_t rssi; /*< [%] Receive signal strength indicator. Values: [0-100], 255: invalid/unknown. */


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
        ss << "  time_boot_ms: " << time_boot_ms << std::endl;
        ss << "  port: " << +port << std::endl;
        ss << "  chan1_scaled: " << chan1_scaled << std::endl;
        ss << "  chan2_scaled: " << chan2_scaled << std::endl;
        ss << "  chan3_scaled: " << chan3_scaled << std::endl;
        ss << "  chan4_scaled: " << chan4_scaled << std::endl;
        ss << "  chan5_scaled: " << chan5_scaled << std::endl;
        ss << "  chan6_scaled: " << chan6_scaled << std::endl;
        ss << "  chan7_scaled: " << chan7_scaled << std::endl;
        ss << "  chan8_scaled: " << chan8_scaled << std::endl;
        ss << "  rssi: " << +rssi << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_boot_ms;                  // offset: 0
        map << chan1_scaled;                  // offset: 4
        map << chan2_scaled;                  // offset: 6
        map << chan3_scaled;                  // offset: 8
        map << chan4_scaled;                  // offset: 10
        map << chan5_scaled;                  // offset: 12
        map << chan6_scaled;                  // offset: 14
        map << chan7_scaled;                  // offset: 16
        map << chan8_scaled;                  // offset: 18
        map << port;                          // offset: 20
        map << rssi;                          // offset: 21
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_boot_ms;                  // offset: 0
        map >> chan1_scaled;                  // offset: 4
        map >> chan2_scaled;                  // offset: 6
        map >> chan3_scaled;                  // offset: 8
        map >> chan4_scaled;                  // offset: 10
        map >> chan5_scaled;                  // offset: 12
        map >> chan6_scaled;                  // offset: 14
        map >> chan7_scaled;                  // offset: 16
        map >> chan8_scaled;                  // offset: 18
        map >> port;                          // offset: 20
        map >> rssi;                          // offset: 21
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
