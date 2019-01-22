// MESSAGE FENCE_POINT support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief FENCE_POINT message
 *
 * A fence point. Used to set a point when from GCS -> MAV. Also used to return a point from MAV -> GCS.
 */
struct FENCE_POINT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 160;
    static constexpr size_t LENGTH = 12;
    static constexpr size_t MIN_LENGTH = 12;
    static constexpr uint8_t CRC_EXTRA = 78;
    static constexpr auto NAME = "FENCE_POINT";


    uint8_t target_system; /*<  System ID. */
    uint8_t target_component; /*<  Component ID. */
    uint8_t idx; /*<  Point index (first point is 1, 0 is for return point). */
    uint8_t count; /*<  Total number of points (for sanity checking). */
    float lat; /*< [deg] Latitude of point. */
    float lng; /*< [deg] Longitude of point. */


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
        ss << "  target_system: " << +target_system << std::endl;
        ss << "  target_component: " << +target_component << std::endl;
        ss << "  idx: " << +idx << std::endl;
        ss << "  count: " << +count << std::endl;
        ss << "  lat: " << lat << std::endl;
        ss << "  lng: " << lng << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << lat;                           // offset: 0
        map << lng;                           // offset: 4
        map << target_system;                 // offset: 8
        map << target_component;              // offset: 9
        map << idx;                           // offset: 10
        map << count;                         // offset: 11
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> lat;                           // offset: 0
        map >> lng;                           // offset: 4
        map >> target_system;                 // offset: 8
        map >> target_component;              // offset: 9
        map >> idx;                           // offset: 10
        map >> count;                         // offset: 11
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
