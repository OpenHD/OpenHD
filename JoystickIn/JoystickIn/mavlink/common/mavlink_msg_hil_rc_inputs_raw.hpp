// MESSAGE HIL_RC_INPUTS_RAW support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief HIL_RC_INPUTS_RAW message
 *
 * Sent from simulation to autopilot. The RAW values of the RC channels received. The standard PPM modulation is as follows: 1000 microseconds: 0%, 2000 microseconds: 100%. Individual receivers/transmitters might violate this specification.
 */
struct HIL_RC_INPUTS_RAW : mavlink::Message {
    static constexpr msgid_t MSG_ID = 92;
    static constexpr size_t LENGTH = 33;
    static constexpr size_t MIN_LENGTH = 33;
    static constexpr uint8_t CRC_EXTRA = 54;
    static constexpr auto NAME = "HIL_RC_INPUTS_RAW";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    uint16_t chan1_raw; /*< [us] RC channel 1 value */
    uint16_t chan2_raw; /*< [us] RC channel 2 value */
    uint16_t chan3_raw; /*< [us] RC channel 3 value */
    uint16_t chan4_raw; /*< [us] RC channel 4 value */
    uint16_t chan5_raw; /*< [us] RC channel 5 value */
    uint16_t chan6_raw; /*< [us] RC channel 6 value */
    uint16_t chan7_raw; /*< [us] RC channel 7 value */
    uint16_t chan8_raw; /*< [us] RC channel 8 value */
    uint16_t chan9_raw; /*< [us] RC channel 9 value */
    uint16_t chan10_raw; /*< [us] RC channel 10 value */
    uint16_t chan11_raw; /*< [us] RC channel 11 value */
    uint16_t chan12_raw; /*< [us] RC channel 12 value */
    uint8_t rssi; /*<  Receive signal strength indicator. Values: [0-100], 255: invalid/unknown. */


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
        ss << "  chan1_raw: " << chan1_raw << std::endl;
        ss << "  chan2_raw: " << chan2_raw << std::endl;
        ss << "  chan3_raw: " << chan3_raw << std::endl;
        ss << "  chan4_raw: " << chan4_raw << std::endl;
        ss << "  chan5_raw: " << chan5_raw << std::endl;
        ss << "  chan6_raw: " << chan6_raw << std::endl;
        ss << "  chan7_raw: " << chan7_raw << std::endl;
        ss << "  chan8_raw: " << chan8_raw << std::endl;
        ss << "  chan9_raw: " << chan9_raw << std::endl;
        ss << "  chan10_raw: " << chan10_raw << std::endl;
        ss << "  chan11_raw: " << chan11_raw << std::endl;
        ss << "  chan12_raw: " << chan12_raw << std::endl;
        ss << "  rssi: " << +rssi << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << chan1_raw;                     // offset: 8
        map << chan2_raw;                     // offset: 10
        map << chan3_raw;                     // offset: 12
        map << chan4_raw;                     // offset: 14
        map << chan5_raw;                     // offset: 16
        map << chan6_raw;                     // offset: 18
        map << chan7_raw;                     // offset: 20
        map << chan8_raw;                     // offset: 22
        map << chan9_raw;                     // offset: 24
        map << chan10_raw;                    // offset: 26
        map << chan11_raw;                    // offset: 28
        map << chan12_raw;                    // offset: 30
        map << rssi;                          // offset: 32
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> chan1_raw;                     // offset: 8
        map >> chan2_raw;                     // offset: 10
        map >> chan3_raw;                     // offset: 12
        map >> chan4_raw;                     // offset: 14
        map >> chan5_raw;                     // offset: 16
        map >> chan6_raw;                     // offset: 18
        map >> chan7_raw;                     // offset: 20
        map >> chan8_raw;                     // offset: 22
        map >> chan9_raw;                     // offset: 24
        map >> chan10_raw;                    // offset: 26
        map >> chan11_raw;                    // offset: 28
        map >> chan12_raw;                    // offset: 30
        map >> rssi;                          // offset: 32
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
