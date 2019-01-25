// MESSAGE TERRAIN_REQUEST support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief TERRAIN_REQUEST message
 *
 * Request for terrain data and terrain status
 */
struct TERRAIN_REQUEST : mavlink::Message {
    static constexpr msgid_t MSG_ID = 133;
    static constexpr size_t LENGTH = 18;
    static constexpr size_t MIN_LENGTH = 18;
    static constexpr uint8_t CRC_EXTRA = 6;
    static constexpr auto NAME = "TERRAIN_REQUEST";


    int32_t lat; /*< [degE7] Latitude of SW corner of first grid */
    int32_t lon; /*< [degE7] Longitude of SW corner of first grid */
    uint16_t grid_spacing; /*< [m] Grid spacing */
    uint64_t mask; /*<  Bitmask of requested 4x4 grids (row major 8x7 array of grids, 56 bits) */


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
        ss << "  grid_spacing: " << grid_spacing << std::endl;
        ss << "  mask: " << mask << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << mask;                          // offset: 0
        map << lat;                           // offset: 8
        map << lon;                           // offset: 12
        map << grid_spacing;                  // offset: 16
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> mask;                          // offset: 0
        map >> lat;                           // offset: 8
        map >> lon;                           // offset: 12
        map >> grid_spacing;                  // offset: 16
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
