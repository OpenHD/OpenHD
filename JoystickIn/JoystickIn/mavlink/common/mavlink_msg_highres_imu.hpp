// MESSAGE HIGHRES_IMU support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief HIGHRES_IMU message
 *
 * The IMU readings in SI units in NED body frame
 */
struct HIGHRES_IMU : mavlink::Message {
    static constexpr msgid_t MSG_ID = 105;
    static constexpr size_t LENGTH = 62;
    static constexpr size_t MIN_LENGTH = 62;
    static constexpr uint8_t CRC_EXTRA = 93;
    static constexpr auto NAME = "HIGHRES_IMU";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    float xacc; /*< [m/s/s] X acceleration */
    float yacc; /*< [m/s/s] Y acceleration */
    float zacc; /*< [m/s/s] Z acceleration */
    float xgyro; /*< [rad/s] Angular speed around X axis */
    float ygyro; /*< [rad/s] Angular speed around Y axis */
    float zgyro; /*< [rad/s] Angular speed around Z axis */
    float xmag; /*< [gauss] X Magnetic field */
    float ymag; /*< [gauss] Y Magnetic field */
    float zmag; /*< [gauss] Z Magnetic field */
    float abs_pressure; /*< [mbar] Absolute pressure */
    float diff_pressure; /*< [mbar] Differential pressure */
    float pressure_alt; /*<  Altitude calculated from pressure */
    float temperature; /*< [degC] Temperature */
    uint16_t fields_updated; /*<  Bitmap for fields that have updated since last message, bit 0 = xacc, bit 12: temperature */


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
        ss << "  abs_pressure: " << abs_pressure << std::endl;
        ss << "  diff_pressure: " << diff_pressure << std::endl;
        ss << "  pressure_alt: " << pressure_alt << std::endl;
        ss << "  temperature: " << temperature << std::endl;
        ss << "  fields_updated: " << fields_updated << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << xacc;                          // offset: 8
        map << yacc;                          // offset: 12
        map << zacc;                          // offset: 16
        map << xgyro;                         // offset: 20
        map << ygyro;                         // offset: 24
        map << zgyro;                         // offset: 28
        map << xmag;                          // offset: 32
        map << ymag;                          // offset: 36
        map << zmag;                          // offset: 40
        map << abs_pressure;                  // offset: 44
        map << diff_pressure;                 // offset: 48
        map << pressure_alt;                  // offset: 52
        map << temperature;                   // offset: 56
        map << fields_updated;                // offset: 60
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> xacc;                          // offset: 8
        map >> yacc;                          // offset: 12
        map >> zacc;                          // offset: 16
        map >> xgyro;                         // offset: 20
        map >> ygyro;                         // offset: 24
        map >> zgyro;                         // offset: 28
        map >> xmag;                          // offset: 32
        map >> ymag;                          // offset: 36
        map >> zmag;                          // offset: 40
        map >> abs_pressure;                  // offset: 44
        map >> diff_pressure;                 // offset: 48
        map >> pressure_alt;                  // offset: 52
        map >> temperature;                   // offset: 56
        map >> fields_updated;                // offset: 60
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
