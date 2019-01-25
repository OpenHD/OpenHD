// MESSAGE GLOBAL_POSITION_INT support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief GLOBAL_POSITION_INT message
 *
 * The filtered global position (e.g. fused GPS and accelerometers). The position is in GPS-frame (right-handed, Z-up). It
               is designed as scaled integer message since the resolution of float is not sufficient.
 */
struct GLOBAL_POSITION_INT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 33;
    static constexpr size_t LENGTH = 28;
    static constexpr size_t MIN_LENGTH = 28;
    static constexpr uint8_t CRC_EXTRA = 104;
    static constexpr auto NAME = "GLOBAL_POSITION_INT";


    uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot). */
    int32_t lat; /*< [degE7] Latitude, expressed */
    int32_t lon; /*< [degE7] Longitude, expressed */
    int32_t alt; /*< [mm] Altitude (AMSL). Note that virtually all GPS modules provide both WGS84 and AMSL. */
    int32_t relative_alt; /*< [mm] Altitude above ground */
    int16_t vx; /*< [cm/s] Ground X Speed (Latitude, positive north) */
    int16_t vy; /*< [cm/s] Ground Y Speed (Longitude, positive east) */
    int16_t vz; /*< [cm/s] Ground Z Speed (Altitude, positive down) */
    uint16_t hdg; /*< [cdeg] Vehicle heading (yaw angle), 0.0..359.99 degrees. If unknown, set to: UINT16_MAX */


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
        ss << "  lat: " << lat << std::endl;
        ss << "  lon: " << lon << std::endl;
        ss << "  alt: " << alt << std::endl;
        ss << "  relative_alt: " << relative_alt << std::endl;
        ss << "  vx: " << vx << std::endl;
        ss << "  vy: " << vy << std::endl;
        ss << "  vz: " << vz << std::endl;
        ss << "  hdg: " << hdg << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_boot_ms;                  // offset: 0
        map << lat;                           // offset: 4
        map << lon;                           // offset: 8
        map << alt;                           // offset: 12
        map << relative_alt;                  // offset: 16
        map << vx;                            // offset: 20
        map << vy;                            // offset: 22
        map << vz;                            // offset: 24
        map << hdg;                           // offset: 26
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_boot_ms;                  // offset: 0
        map >> lat;                           // offset: 4
        map >> lon;                           // offset: 8
        map >> alt;                           // offset: 12
        map >> relative_alt;                  // offset: 16
        map >> vx;                            // offset: 20
        map >> vy;                            // offset: 22
        map >> vz;                            // offset: 24
        map >> hdg;                           // offset: 26
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
