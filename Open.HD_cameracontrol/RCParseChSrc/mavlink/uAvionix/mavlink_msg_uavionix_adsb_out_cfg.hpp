// MESSAGE UAVIONIX_ADSB_OUT_CFG support class

#pragma once

namespace mavlink {
namespace uAvionix {
namespace msg {

/**
 * @brief UAVIONIX_ADSB_OUT_CFG message
 *
 * Static data to configure the ADS-B transponder (send within 10 sec of a POR and every 10 sec thereafter)
 */
struct UAVIONIX_ADSB_OUT_CFG : mavlink::Message {
    static constexpr msgid_t MSG_ID = 10001;
    static constexpr size_t LENGTH = 20;
    static constexpr size_t MIN_LENGTH = 20;
    static constexpr uint8_t CRC_EXTRA = 209;
    static constexpr auto NAME = "UAVIONIX_ADSB_OUT_CFG";


    uint32_t ICAO; /*<  Vehicle address (24 bit) */
    std::array<char, 9> callsign; /*<  Vehicle identifier (8 characters, null terminated, valid characters are A-Z, 0-9, " " only) */
    uint8_t emitterType; /*<  Transmitting vehicle type. See ADSB_EMITTER_TYPE enum */
    uint8_t aircraftSize; /*<  Aircraft length and width encoding (table 2-35 of DO-282B) */
    uint8_t gpsOffsetLat; /*<  GPS antenna lateral offset (table 2-36 of DO-282B) */
    uint8_t gpsOffsetLon; /*<  GPS antenna longitudinal offset from nose [if non-zero, take position (in meters) divide by 2 and add one] (table 2-37 DO-282B) */
    uint16_t stallSpeed; /*< [cm/s] Aircraft stall speed in cm/s */
    uint8_t rfSelect; /*<  ADS-B transponder reciever and transmit enable flags */


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
        ss << "  ICAO: " << ICAO << std::endl;
        ss << "  callsign: \"" << to_string(callsign) << "\"" << std::endl;
        ss << "  emitterType: " << +emitterType << std::endl;
        ss << "  aircraftSize: " << +aircraftSize << std::endl;
        ss << "  gpsOffsetLat: " << +gpsOffsetLat << std::endl;
        ss << "  gpsOffsetLon: " << +gpsOffsetLon << std::endl;
        ss << "  stallSpeed: " << stallSpeed << std::endl;
        ss << "  rfSelect: " << +rfSelect << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << ICAO;                          // offset: 0
        map << stallSpeed;                    // offset: 4
        map << callsign;                      // offset: 6
        map << emitterType;                   // offset: 15
        map << aircraftSize;                  // offset: 16
        map << gpsOffsetLat;                  // offset: 17
        map << gpsOffsetLon;                  // offset: 18
        map << rfSelect;                      // offset: 19
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> ICAO;                          // offset: 0
        map >> stallSpeed;                    // offset: 4
        map >> callsign;                      // offset: 6
        map >> emitterType;                   // offset: 15
        map >> aircraftSize;                  // offset: 16
        map >> gpsOffsetLat;                  // offset: 17
        map >> gpsOffsetLon;                  // offset: 18
        map >> rfSelect;                      // offset: 19
    }
};

} // namespace msg
} // namespace uAvionix
} // namespace mavlink
