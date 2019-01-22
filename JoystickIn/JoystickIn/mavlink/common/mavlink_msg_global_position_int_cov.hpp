// MESSAGE GLOBAL_POSITION_INT_COV support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief GLOBAL_POSITION_INT_COV message
 *
 * The filtered global position (e.g. fused GPS and accelerometers). The position is in GPS-frame (right-handed, Z-up). It  is designed as scaled integer message since the resolution of float is not sufficient. NOTE: This message is intended for onboard networks / companion computers and higher-bandwidth links and optimized for accuracy and completeness. Please use the GLOBAL_POSITION_INT message for a minimal subset.
 */
struct GLOBAL_POSITION_INT_COV : mavlink::Message {
    static constexpr msgid_t MSG_ID = 63;
    static constexpr size_t LENGTH = 181;
    static constexpr size_t MIN_LENGTH = 181;
    static constexpr uint8_t CRC_EXTRA = 119;
    static constexpr auto NAME = "GLOBAL_POSITION_INT_COV";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    uint8_t estimator_type; /*<  Class id of the estimator this estimate originated from. */
    int32_t lat; /*< [degE7] Latitude */
    int32_t lon; /*< [degE7] Longitude */
    int32_t alt; /*< [mm] Altitude in meters above MSL */
    int32_t relative_alt; /*< [mm] Altitude above ground */
    float vx; /*< [m/s] Ground X Speed (Latitude) */
    float vy; /*< [m/s] Ground Y Speed (Longitude) */
    float vz; /*< [m/s] Ground Z Speed (Altitude) */
    std::array<float, 36> covariance; /*<  Covariance matrix (first six entries are the first ROW, next six entries are the second row, etc.) */


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
        ss << "  estimator_type: " << +estimator_type << std::endl;
        ss << "  lat: " << lat << std::endl;
        ss << "  lon: " << lon << std::endl;
        ss << "  alt: " << alt << std::endl;
        ss << "  relative_alt: " << relative_alt << std::endl;
        ss << "  vx: " << vx << std::endl;
        ss << "  vy: " << vy << std::endl;
        ss << "  vz: " << vz << std::endl;
        ss << "  covariance: [" << to_string(covariance) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << lat;                           // offset: 8
        map << lon;                           // offset: 12
        map << alt;                           // offset: 16
        map << relative_alt;                  // offset: 20
        map << vx;                            // offset: 24
        map << vy;                            // offset: 28
        map << vz;                            // offset: 32
        map << covariance;                    // offset: 36
        map << estimator_type;                // offset: 180
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> lat;                           // offset: 8
        map >> lon;                           // offset: 12
        map >> alt;                           // offset: 16
        map >> relative_alt;                  // offset: 20
        map >> vx;                            // offset: 24
        map >> vy;                            // offset: 28
        map >> vz;                            // offset: 32
        map >> covariance;                    // offset: 36
        map >> estimator_type;                // offset: 180
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
