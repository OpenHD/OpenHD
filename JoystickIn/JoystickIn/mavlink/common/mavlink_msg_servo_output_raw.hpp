// MESSAGE SERVO_OUTPUT_RAW support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief SERVO_OUTPUT_RAW message
 *
 * The RAW values of the servo outputs (for RC input from the remote, use the RC_CHANNELS messages). The standard PPM modulation is as follows: 1000 microseconds: 0%, 2000 microseconds: 100%.
 */
struct SERVO_OUTPUT_RAW : mavlink::Message {
    static constexpr msgid_t MSG_ID = 36;
    static constexpr size_t LENGTH = 37;
    static constexpr size_t MIN_LENGTH = 21;
    static constexpr uint8_t CRC_EXTRA = 222;
    static constexpr auto NAME = "SERVO_OUTPUT_RAW";


    uint32_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    uint8_t port; /*<  Servo output port (set of 8 outputs = 1 port). Most MAVs will just use one, but this allows to encode more than 8 servos. */
    uint16_t servo1_raw; /*< [us] Servo output 1 value */
    uint16_t servo2_raw; /*< [us] Servo output 2 value */
    uint16_t servo3_raw; /*< [us] Servo output 3 value */
    uint16_t servo4_raw; /*< [us] Servo output 4 value */
    uint16_t servo5_raw; /*< [us] Servo output 5 value */
    uint16_t servo6_raw; /*< [us] Servo output 6 value */
    uint16_t servo7_raw; /*< [us] Servo output 7 value */
    uint16_t servo8_raw; /*< [us] Servo output 8 value */
    uint16_t servo9_raw; /*< [us] Servo output 9 value */
    uint16_t servo10_raw; /*< [us] Servo output 10 value */
    uint16_t servo11_raw; /*< [us] Servo output 11 value */
    uint16_t servo12_raw; /*< [us] Servo output 12 value */
    uint16_t servo13_raw; /*< [us] Servo output 13 value */
    uint16_t servo14_raw; /*< [us] Servo output 14 value */
    uint16_t servo15_raw; /*< [us] Servo output 15 value */
    uint16_t servo16_raw; /*< [us] Servo output 16 value */


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
        ss << "  port: " << +port << std::endl;
        ss << "  servo1_raw: " << servo1_raw << std::endl;
        ss << "  servo2_raw: " << servo2_raw << std::endl;
        ss << "  servo3_raw: " << servo3_raw << std::endl;
        ss << "  servo4_raw: " << servo4_raw << std::endl;
        ss << "  servo5_raw: " << servo5_raw << std::endl;
        ss << "  servo6_raw: " << servo6_raw << std::endl;
        ss << "  servo7_raw: " << servo7_raw << std::endl;
        ss << "  servo8_raw: " << servo8_raw << std::endl;
        ss << "  servo9_raw: " << servo9_raw << std::endl;
        ss << "  servo10_raw: " << servo10_raw << std::endl;
        ss << "  servo11_raw: " << servo11_raw << std::endl;
        ss << "  servo12_raw: " << servo12_raw << std::endl;
        ss << "  servo13_raw: " << servo13_raw << std::endl;
        ss << "  servo14_raw: " << servo14_raw << std::endl;
        ss << "  servo15_raw: " << servo15_raw << std::endl;
        ss << "  servo16_raw: " << servo16_raw << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << servo1_raw;                    // offset: 4
        map << servo2_raw;                    // offset: 6
        map << servo3_raw;                    // offset: 8
        map << servo4_raw;                    // offset: 10
        map << servo5_raw;                    // offset: 12
        map << servo6_raw;                    // offset: 14
        map << servo7_raw;                    // offset: 16
        map << servo8_raw;                    // offset: 18
        map << port;                          // offset: 20
        map << servo9_raw;                    // offset: 21
        map << servo10_raw;                   // offset: 23
        map << servo11_raw;                   // offset: 25
        map << servo12_raw;                   // offset: 27
        map << servo13_raw;                   // offset: 29
        map << servo14_raw;                   // offset: 31
        map << servo15_raw;                   // offset: 33
        map << servo16_raw;                   // offset: 35
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> servo1_raw;                    // offset: 4
        map >> servo2_raw;                    // offset: 6
        map >> servo3_raw;                    // offset: 8
        map >> servo4_raw;                    // offset: 10
        map >> servo5_raw;                    // offset: 12
        map >> servo6_raw;                    // offset: 14
        map >> servo7_raw;                    // offset: 16
        map >> servo8_raw;                    // offset: 18
        map >> port;                          // offset: 20
        map >> servo9_raw;                    // offset: 21
        map >> servo10_raw;                   // offset: 23
        map >> servo11_raw;                   // offset: 25
        map >> servo12_raw;                   // offset: 27
        map >> servo13_raw;                   // offset: 29
        map >> servo14_raw;                   // offset: 31
        map >> servo15_raw;                   // offset: 33
        map >> servo16_raw;                   // offset: 35
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
