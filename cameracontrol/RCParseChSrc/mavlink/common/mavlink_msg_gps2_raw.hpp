// MESSAGE GPS2_RAW support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief GPS2_RAW message
 *
 * Second GPS data.
 */
struct GPS2_RAW : mavlink::Message {
    static constexpr msgid_t MSG_ID = 124;
    static constexpr size_t LENGTH = 35;
    static constexpr size_t MIN_LENGTH = 35;
    static constexpr uint8_t CRC_EXTRA = 87;
    static constexpr auto NAME = "GPS2_RAW";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    uint8_t fix_type; /*<  GPS fix type. */
    int32_t lat; /*< [degE7] Latitude (WGS84) */
    int32_t lon; /*< [degE7] Longitude (WGS84) */
    int32_t alt; /*< [mm] Altitude (AMSL). Positive for up. */
    uint16_t eph; /*< [cm] GPS HDOP horizontal dilution of position. If unknown, set to: UINT16_MAX */
    uint16_t epv; /*< [cm] GPS VDOP vertical dilution of position. If unknown, set to: UINT16_MAX */
    uint16_t vel; /*< [cm/s] GPS ground speed. If unknown, set to: UINT16_MAX */
    uint16_t cog; /*< [cdeg] Course over ground (NOT heading, but direction of movement): 0.0..359.99 degrees. If unknown, set to: UINT16_MAX */
    uint8_t satellites_visible; /*<  Number of satellites visible. If unknown, set to 255 */
    uint8_t dgps_numch; /*<  Number of DGPS satellites */
    uint32_t dgps_age; /*< [ms] Age of DGPS info */


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
        ss << "  fix_type: " << +fix_type << std::endl;
        ss << "  lat: " << lat << std::endl;
        ss << "  lon: " << lon << std::endl;
        ss << "  alt: " << alt << std::endl;
        ss << "  eph: " << eph << std::endl;
        ss << "  epv: " << epv << std::endl;
        ss << "  vel: " << vel << std::endl;
        ss << "  cog: " << cog << std::endl;
        ss << "  satellites_visible: " << +satellites_visible << std::endl;
        ss << "  dgps_numch: " << +dgps_numch << std::endl;
        ss << "  dgps_age: " << dgps_age << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << lat;                           // offset: 8
        map << lon;                           // offset: 12
        map << alt;                           // offset: 16
        map << dgps_age;                      // offset: 20
        map << eph;                           // offset: 24
        map << epv;                           // offset: 26
        map << vel;                           // offset: 28
        map << cog;                           // offset: 30
        map << fix_type;                      // offset: 32
        map << satellites_visible;            // offset: 33
        map << dgps_numch;                    // offset: 34
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> lat;                           // offset: 8
        map >> lon;                           // offset: 12
        map >> alt;                           // offset: 16
        map >> dgps_age;                      // offset: 20
        map >> eph;                           // offset: 24
        map >> epv;                           // offset: 26
        map >> vel;                           // offset: 28
        map >> cog;                           // offset: 30
        map >> fix_type;                      // offset: 32
        map >> satellites_visible;            // offset: 33
        map >> dgps_numch;                    // offset: 34
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
