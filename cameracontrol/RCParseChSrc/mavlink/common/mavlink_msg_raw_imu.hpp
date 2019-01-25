// MESSAGE RAW_IMU support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief RAW_IMU message
 *
 * The RAW IMU readings for the usual 9DOF sensor setup. This message should always contain the true raw values without any scaling to allow data capture and system debugging.
 */
struct RAW_IMU : mavlink::Message {
    static constexpr msgid_t MSG_ID = 27;
    static constexpr size_t LENGTH = 26;
    static constexpr size_t MIN_LENGTH = 26;
    static constexpr uint8_t CRC_EXTRA = 144;
    static constexpr auto NAME = "RAW_IMU";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    int16_t xacc; /*<  X acceleration (raw) */
    int16_t yacc; /*<  Y acceleration (raw) */
    int16_t zacc; /*<  Z acceleration (raw) */
    int16_t xgyro; /*<  Angular speed around X axis (raw) */
    int16_t ygyro; /*<  Angular speed around Y axis (raw) */
    int16_t zgyro; /*<  Angular speed around Z axis (raw) */
    int16_t xmag; /*<  X Magnetic field (raw) */
    int16_t ymag; /*<  Y Magnetic field (raw) */
    int16_t zmag; /*<  Z Magnetic field (raw) */


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
        ss << "  xacc: " << xacc << std::endl;
        ss << "  yacc: " << yacc << std::endl;
        ss << "  zacc: " << zacc << std::endl;
        ss << "  xgyro: " << xgyro << std::endl;
        ss << "  ygyro: " << ygyro << std::endl;
        ss << "  zgyro: " << zgyro << std::endl;
        ss << "  xmag: " << xmag << std::endl;
        ss << "  ymag: " << ymag << std::endl;
        ss << "  zmag: " << zmag << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << xacc;                          // offset: 8
        map << yacc;                          // offset: 10
        map << zacc;                          // offset: 12
        map << xgyro;                         // offset: 14
        map << ygyro;                         // offset: 16
        map << zgyro;                         // offset: 18
        map << xmag;                          // offset: 20
        map << ymag;                          // offset: 22
        map << zmag;                          // offset: 24
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> xacc;                          // offset: 8
        map >> yacc;                          // offset: 10
        map >> zacc;                          // offset: 12
        map >> xgyro;                         // offset: 14
        map >> ygyro;                         // offset: 16
        map >> zgyro;                         // offset: 18
        map >> xmag;                          // offset: 20
        map >> ymag;                          // offset: 22
        map >> zmag;                          // offset: 24
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
