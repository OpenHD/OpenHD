// MESSAGE UAVIONIX_ADSB_OUT_DYNAMIC support class

#pragma once

namespace mavlink {
namespace uAvionix {
namespace msg {

/**
 * @brief UAVIONIX_ADSB_OUT_DYNAMIC message
 *
 * Dynamic data used to generate ADS-B out transponder data (send at 5Hz)
 */
struct UAVIONIX_ADSB_OUT_DYNAMIC : mavlink::Message {
    static constexpr msgid_t MSG_ID = 10002;
    static constexpr size_t LENGTH = 41;
    static constexpr size_t MIN_LENGTH = 41;
    static constexpr uint8_t CRC_EXTRA = 186;
    static constexpr auto NAME = "UAVIONIX_ADSB_OUT_DYNAMIC";


    uint32_t utcTime; /*< [s] UTC time in seconds since GPS epoch (Jan 6, 1980). If unknown set to UINT32_MAX */
    int32_t gpsLat; /*< [degE7] Latitude WGS84 (deg * 1E7). If unknown set to INT32_MAX */
    int32_t gpsLon; /*< [degE7] Longitude WGS84 (deg * 1E7). If unknown set to INT32_MAX */
    int32_t gpsAlt; /*< [mm] Altitude in mm (m * 1E-3) UP +ve. WGS84 altitude. If unknown set to INT32_MAX */
    uint8_t gpsFix; /*<  0-1: no fix, 2: 2D fix, 3: 3D fix, 4: DGPS, 5: RTK */
    uint8_t numSats; /*<  Number of satellites visible. If unknown set to UINT8_MAX */
    int32_t baroAltMSL; /*< [mbar] Barometric pressure altitude relative to a standard atmosphere of 1013.2 mBar and NOT bar corrected altitude (m * 1E-3). (up +ve). If unknown set to INT32_MAX */
    uint32_t accuracyHor; /*< [mm] Horizontal accuracy in mm (m * 1E-3). If unknown set to UINT32_MAX */
    uint16_t accuracyVert; /*< [cm] Vertical accuracy in cm. If unknown set to UINT16_MAX */
    uint16_t accuracyVel; /*< [mm/s] Velocity accuracy in mm/s (m * 1E-3). If unknown set to UINT16_MAX */
    int16_t velVert; /*< [cm/s] GPS vertical speed in cm/s. If unknown set to INT16_MAX */
    int16_t velNS; /*< [cm/s] North-South velocity over ground in cm/s North +ve. If unknown set to INT16_MAX */
    int16_t VelEW; /*< [cm/s] East-West velocity over ground in cm/s East +ve. If unknown set to INT16_MAX */
    uint8_t emergencyStatus; /*<  Emergency status */
    uint16_t state; /*<  ADS-B transponder dynamic input state flags */
    uint16_t squawk; /*<  Mode A code (typically 1200 [0x04B0] for VFR) */


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
        ss << "  utcTime: " << utcTime << std::endl;
        ss << "  gpsLat: " << gpsLat << std::endl;
        ss << "  gpsLon: " << gpsLon << std::endl;
        ss << "  gpsAlt: " << gpsAlt << std::endl;
        ss << "  gpsFix: " << +gpsFix << std::endl;
        ss << "  numSats: " << +numSats << std::endl;
        ss << "  baroAltMSL: " << baroAltMSL << std::endl;
        ss << "  accuracyHor: " << accuracyHor << std::endl;
        ss << "  accuracyVert: " << accuracyVert << std::endl;
        ss << "  accuracyVel: " << accuracyVel << std::endl;
        ss << "  velVert: " << velVert << std::endl;
        ss << "  velNS: " << velNS << std::endl;
        ss << "  VelEW: " << VelEW << std::endl;
        ss << "  emergencyStatus: " << +emergencyStatus << std::endl;
        ss << "  state: " << state << std::endl;
        ss << "  squawk: " << squawk << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << utcTime;                       // offset: 0
        map << gpsLat;                        // offset: 4
        map << gpsLon;                        // offset: 8
        map << gpsAlt;                        // offset: 12
        map << baroAltMSL;                    // offset: 16
        map << accuracyHor;                   // offset: 20
        map << accuracyVert;                  // offset: 24
        map << accuracyVel;                   // offset: 26
        map << velVert;                       // offset: 28
        map << velNS;                         // offset: 30
        map << VelEW;                         // offset: 32
        map << state;                         // offset: 34
        map << squawk;                        // offset: 36
        map << gpsFix;                        // offset: 38
        map << numSats;                       // offset: 39
        map << emergencyStatus;               // offset: 40
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> utcTime;                       // offset: 0
        map >> gpsLat;                        // offset: 4
        map >> gpsLon;                        // offset: 8
        map >> gpsAlt;                        // offset: 12
        map >> baroAltMSL;                    // offset: 16
        map >> accuracyHor;                   // offset: 20
        map >> accuracyVert;                  // offset: 24
        map >> accuracyVel;                   // offset: 26
        map >> velVert;                       // offset: 28
        map >> velNS;                         // offset: 30
        map >> VelEW;                         // offset: 32
        map >> state;                         // offset: 34
        map >> squawk;                        // offset: 36
        map >> gpsFix;                        // offset: 38
        map >> numSats;                       // offset: 39
        map >> emergencyStatus;               // offset: 40
    }
};

} // namespace msg
} // namespace uAvionix
} // namespace mavlink
