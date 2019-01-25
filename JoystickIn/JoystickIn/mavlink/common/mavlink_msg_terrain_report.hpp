// MESSAGE TERRAIN_REPORT support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief TERRAIN_REPORT message
 *
 * Response from a TERRAIN_CHECK request
 */
struct TERRAIN_REPORT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 136;
    static constexpr size_t LENGTH = 22;
    static constexpr size_t MIN_LENGTH = 22;
    static constexpr uint8_t CRC_EXTRA = 1;
    static constexpr auto NAME = "TERRAIN_REPORT";


    int32_t lat; /*< [degE7] Latitude */
    int32_t lon; /*< [degE7] Longitude */
    uint16_t spacing; /*<  grid spacing (zero if terrain at this location unavailable) */
    float terrain_height; /*< [m] Terrain height AMSL */
    float current_height; /*< [m] Current vehicle height above lat/lon terrain height */
    uint16_t pending; /*<  Number of 4x4 terrain blocks waiting to be received or read from disk */
    uint16_t loaded; /*<  Number of 4x4 terrain blocks in memory */


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
        ss << "  lat: " << lat << std::endl;
        ss << "  lon: " << lon << std::endl;
        ss << "  spacing: " << spacing << std::endl;
        ss << "  terrain_height: " << terrain_height << std::endl;
        ss << "  current_height: " << current_height << std::endl;
        ss << "  pending: " << pending << std::endl;
        ss << "  loaded: " << loaded << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << lat;                           // offset: 0
        map << lon;                           // offset: 4
        map << terrain_height;                // offset: 8
        map << current_height;                // offset: 12
        map << spacing;                       // offset: 16
        map << pending;                       // offset: 18
        map << loaded;                        // offset: 20
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> lat;                           // offset: 0
        map >> lon;                           // offset: 4
        map >> terrain_height;                // offset: 8
        map >> current_height;                // offset: 12
        map >> spacing;                       // offset: 16
        map >> pending;                       // offset: 18
        map >> loaded;                        // offset: 20
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
