// MESSAGE GPS_INPUT support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief GPS_INPUT message
 *
 * GPS sensor input message.  This is a raw sensor value sent by the GPS. This is NOT the global position estimate of the system.
 */
struct GPS_INPUT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 232;
    static constexpr size_t LENGTH = 63;
    static constexpr size_t MIN_LENGTH = 63;
    static constexpr uint8_t CRC_EXTRA = 151;
    static constexpr auto NAME = "GPS_INPUT";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    uint8_t gps_id; /*<  ID of the GPS for multiple GPS inputs */
    uint16_t ignore_flags; /*<  Bitmap indicating which GPS input flags fields to ignore.  All other fields must be provided. */
    uint32_t time_week_ms; /*< [ms] GPS time (from start of GPS week) */
    uint16_t time_week; /*<  GPS week number */
    uint8_t fix_type; /*<  0-1: no fix, 2: 2D fix, 3: 3D fix. 4: 3D with DGPS. 5: 3D with RTK */
    int32_t lat; /*< [degE7] Latitude (WGS84) */
    int32_t lon; /*< [degE7] Longitude (WGS84) */
    float alt; /*< [m] Altitude (AMSL). Positive for up. */
    float hdop; /*< [m] GPS HDOP horizontal dilution of position */
    float vdop; /*< [m] GPS VDOP vertical dilution of position */
    float vn; /*< [m/s] GPS velocity in NORTH direction in earth-fixed NED frame */
    float ve; /*< [m/s] GPS velocity in EAST direction in earth-fixed NED frame */
    float vd; /*< [m/s] GPS velocity in DOWN direction in earth-fixed NED frame */
    float speed_accuracy; /*< [m/s] GPS speed accuracy */
    float horiz_accuracy; /*< [m] GPS horizontal accuracy */
    float vert_accuracy; /*< [m] GPS vertical accuracy */
    uint8_t satellites_visible; /*<  Number of satellites visible. */


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
        ss << "  gps_id: " << +gps_id << std::endl;
        ss << "  ignore_flags: " << ignore_flags << std::endl;
        ss << "  time_week_ms: " << time_week_ms << std::endl;
        ss << "  time_week: " << time_week << std::endl;
        ss << "  fix_type: " << +fix_type << std::endl;
        ss << "  lat: " << lat << std::endl;
        ss << "  lon: " << lon << std::endl;
        ss << "  alt: " << alt << std::endl;
        ss << "  hdop: " << hdop << std::endl;
        ss << "  vdop: " << vdop << std::endl;
        ss << "  vn: " << vn << std::endl;
        ss << "  ve: " << ve << std::endl;
        ss << "  vd: " << vd << std::endl;
        ss << "  speed_accuracy: " << speed_accuracy << std::endl;
        ss << "  horiz_accuracy: " << horiz_accuracy << std::endl;
        ss << "  vert_accuracy: " << vert_accuracy << std::endl;
        ss << "  satellites_visible: " << +satellites_visible << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << time_week_ms;                  // offset: 8
        map << lat;                           // offset: 12
        map << lon;                           // offset: 16
        map << alt;                           // offset: 20
        map << hdop;                          // offset: 24
        map << vdop;                          // offset: 28
        map << vn;                            // offset: 32
        map << ve;                            // offset: 36
        map << vd;                            // offset: 40
        map << speed_accuracy;                // offset: 44
        map << horiz_accuracy;                // offset: 48
        map << vert_accuracy;                 // offset: 52
        map << ignore_flags;                  // offset: 56
        map << time_week;                     // offset: 58
        map << gps_id;                        // offset: 60
        map << fix_type;                      // offset: 61
        map << satellites_visible;            // offset: 62
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> time_week_ms;                  // offset: 8
        map >> lat;                           // offset: 12
        map >> lon;                           // offset: 16
        map >> alt;                           // offset: 20
        map >> hdop;                          // offset: 24
        map >> vdop;                          // offset: 28
        map >> vn;                            // offset: 32
        map >> ve;                            // offset: 36
        map >> vd;                            // offset: 40
        map >> speed_accuracy;                // offset: 44
        map >> horiz_accuracy;                // offset: 48
        map >> vert_accuracy;                 // offset: 52
        map >> ignore_flags;                  // offset: 56
        map >> time_week;                     // offset: 58
        map >> gps_id;                        // offset: 60
        map >> fix_type;                      // offset: 61
        map >> satellites_visible;            // offset: 62
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
