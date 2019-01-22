// MESSAGE RC_CHANNELS_RAW support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief RC_CHANNELS_RAW message
 *
 * The RAW values of the RC channels received. The standard PPM modulation is as follows: 1000 microseconds: 0%, 2000 microseconds: 100%. A value of UINT16_MAX implies the channel is unused. Individual receivers/transmitters might violate this specification.
 */
struct RC_CHANNELS_RAW : mavlink::Message {
    static constexpr msgid_t MSG_ID = 35;
    static constexpr size_t LENGTH = 22;
    static constexpr size_t MIN_LENGTH = 22;
    static constexpr uint8_t CRC_EXTRA = 244;
    static constexpr auto NAME = "RC_CHANNELS_RAW";


    uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot). */
    uint8_t port; /*<  Servo output port (set of 8 outputs = 1 port). Most MAVs will just use one, but this allows for more than 8 servos. */
    uint16_t chan1_raw; /*< [us] RC channel 1 value. */
    uint16_t chan2_raw; /*< [us] RC channel 2 value. */
    uint16_t chan3_raw; /*< [us] RC channel 3 value. */
    uint16_t chan4_raw; /*< [us] RC channel 4 value. */
    uint16_t chan5_raw; /*< [us] RC channel 5 value. */
    uint16_t chan6_raw; /*< [us] RC channel 6 value. */
    uint16_t chan7_raw; /*< [us] RC channel 7 value. */
    uint16_t chan8_raw; /*< [us] RC channel 8 value. */
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
        ss << "  chan1_raw: " << chan1_raw << std::endl;
        ss << "  chan2_raw: " << chan2_raw << std::endl;
        ss << "  chan3_raw: " << chan3_raw << std::endl;
        ss << "  chan4_raw: " << chan4_raw << std::endl;
        ss << "  chan5_raw: " << chan5_raw << std::endl;
        ss << "  chan6_raw: " << chan6_raw << std::endl;
        ss << "  chan7_raw: " << chan7_raw << std::endl;
        ss << "  chan8_raw: " << chan8_raw << std::endl;
        ss << "  rssi: " << +rssi << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_boot_ms;                  // offset: 0
        map << chan1_raw;                     // offset: 4
        map << chan2_raw;                     // offset: 6
        map << chan3_raw;                     // offset: 8
        map << chan4_raw;                     // offset: 10
        map << chan5_raw;                     // offset: 12
        map << chan6_raw;                     // offset: 14
        map << chan7_raw;                     // offset: 16
        map << chan8_raw;                     // offset: 18
        map << port;                          // offset: 20
        map << rssi;                          // offset: 21
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_boot_ms;                  // offset: 0
        map >> chan1_raw;                     // offset: 4
        map >> chan2_raw;                     // offset: 6
        map >> chan3_raw;                     // offset: 8
        map >> chan4_raw;                     // offset: 10
        map >> chan5_raw;                     // offset: 12
        map >> chan6_raw;                     // offset: 14
        map >> chan7_raw;                     // offset: 16
        map >> chan8_raw;                     // offset: 18
        map >> port;                          // offset: 20
        map >> rssi;                          // offset: 21
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
