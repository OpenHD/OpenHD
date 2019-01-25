// MESSAGE GPS_STATUS support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief GPS_STATUS message
 *
 * The positioning status, as reported by GPS. This message is intended to display status information about each satellite visible to the receiver. See message GLOBAL_POSITION for the global position estimate. This message can contain information for up to 20 satellites.
 */
struct GPS_STATUS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 25;
    static constexpr size_t LENGTH = 101;
    static constexpr size_t MIN_LENGTH = 101;
    static constexpr uint8_t CRC_EXTRA = 23;
    static constexpr auto NAME = "GPS_STATUS";


    uint8_t satellites_visible; /*<  Number of satellites visible */
    std::array<uint8_t, 20> satellite_prn; /*<  Global satellite ID */
    std::array<uint8_t, 20> satellite_used; /*<  0: Satellite not used, 1: used for localization */
    std::array<uint8_t, 20> satellite_elevation; /*< [deg] Elevation (0: right on top of receiver, 90: on the horizon) of satellite */
    std::array<uint8_t, 20> satellite_azimuth; /*< [deg] Direction of satellite, 0: 0 deg, 255: 360 deg. */
    std::array<uint8_t, 20> satellite_snr; /*< [dB] Signal to noise ratio of satellite */


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
        ss << "  satellites_visible: " << +satellites_visible << std::endl;
        ss << "  satellite_prn: [" << to_string(satellite_prn) << "]" << std::endl;
        ss << "  satellite_used: [" << to_string(satellite_used) << "]" << std::endl;
        ss << "  satellite_elevation: [" << to_string(satellite_elevation) << "]" << std::endl;
        ss << "  satellite_azimuth: [" << to_string(satellite_azimuth) << "]" << std::endl;
        ss << "  satellite_snr: [" << to_string(satellite_snr) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << satellites_visible;            // offset: 0
        map << satellite_prn;                 // offset: 1
        map << satellite_used;                // offset: 21
        map << satellite_elevation;           // offset: 41
        map << satellite_azimuth;             // offset: 61
        map << satellite_snr;                 // offset: 81
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> satellites_visible;            // offset: 0
        map >> satellite_prn;                 // offset: 1
        map >> satellite_used;                // offset: 21
        map >> satellite_elevation;           // offset: 41
        map >> satellite_azimuth;             // offset: 61
        map >> satellite_snr;                 // offset: 81
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
