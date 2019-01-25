// MESSAGE TERRAIN_CHECK support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief TERRAIN_CHECK message
 *
 * Request that the vehicle report terrain height at the given location. Used by GCS to check if vehicle has all terrain data needed for a mission.
 */
struct TERRAIN_CHECK : mavlink::Message {
    static constexpr msgid_t MSG_ID = 135;
    static constexpr size_t LENGTH = 8;
    static constexpr size_t MIN_LENGTH = 8;
    static constexpr uint8_t CRC_EXTRA = 203;
    static constexpr auto NAME = "TERRAIN_CHECK";


    int32_t lat; /*< [degE7] Latitude */
    int32_t lon; /*< [degE7] Longitude */


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

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << lat;                           // offset: 0
        map << lon;                           // offset: 4
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> lat;                           // offset: 0
        map >> lon;                           // offset: 4
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
