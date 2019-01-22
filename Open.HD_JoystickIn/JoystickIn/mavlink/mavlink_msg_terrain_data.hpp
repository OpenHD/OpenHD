// MESSAGE TERRAIN_DATA support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief TERRAIN_DATA message
 *
 * Terrain data sent from GCS. The lat/lon and grid_spacing must be the same as a lat/lon from a TERRAIN_REQUEST
 */
struct TERRAIN_DATA : mavlink::Message {
    static constexpr msgid_t MSG_ID = 134;
    static constexpr size_t LENGTH = 43;
    static constexpr size_t MIN_LENGTH = 43;
    static constexpr uint8_t CRC_EXTRA = 229;
    static constexpr auto NAME = "TERRAIN_DATA";


    int32_t lat; /*< [degE7] Latitude of SW corner of first grid */
    int32_t lon; /*< [degE7] Longitude of SW corner of first grid */
    uint16_t grid_spacing; /*< [m] Grid spacing */
    uint8_t gridbit; /*<  bit within the terrain request mask */
    std::array<int16_t, 16> data; /*< [m] Terrain data AMSL */


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
        ss << "  gridbit: " << +gridbit << std::endl;
        ss << "  data: [" << to_string(data) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << lat;                           // offset: 0
        map << lon;                           // offset: 4
        map << grid_spacing;                  // offset: 8
        map << data;                          // offset: 10
        map << gridbit;                       // offset: 42
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> lat;                           // offset: 0
        map >> lon;                           // offset: 4
        map >> grid_spacing;                  // offset: 8
        map >> data;                          // offset: 10
        map >> gridbit;                       // offset: 42
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
