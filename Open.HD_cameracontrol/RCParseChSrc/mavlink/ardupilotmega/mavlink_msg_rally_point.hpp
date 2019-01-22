// MESSAGE RALLY_POINT support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief RALLY_POINT message
 *
 * A rally point. Used to set a point when from GCS -> MAV. Also used to return a point from MAV -> GCS.
 */
struct RALLY_POINT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 175;
    static constexpr size_t LENGTH = 19;
    static constexpr size_t MIN_LENGTH = 19;
    static constexpr uint8_t CRC_EXTRA = 138;
    static constexpr auto NAME = "RALLY_POINT";


    uint8_t target_system; /*<  System ID. */
    uint8_t target_component; /*<  Component ID. */
    uint8_t idx; /*<  Point index (first point is 0). */
    uint8_t count; /*<  Total number of points (for sanity checking). */
    int32_t lat; /*< [degE7] Latitude of point. */
    int32_t lng; /*< [degE7] Longitude of point. */
    int16_t alt; /*< [m] Transit / loiter altitude relative to home. */
    int16_t break_alt; /*< [m] Break altitude relative to home. */
    uint16_t land_dir; /*< [cdeg] Heading to aim for when landing. */
    uint8_t flags; /*<  Configuration flags. */


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
        ss << "  alt: " << alt << std::endl;
        ss << "  break_alt: " << break_alt << std::endl;
        ss << "  land_dir: " << land_dir << std::endl;
        ss << "  flags: " << +flags << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << lat;                           // offset: 0
        map << lng;                           // offset: 4
        map << alt;                           // offset: 8
        map << break_alt;                     // offset: 10
        map << land_dir;                      // offset: 12
        map << target_system;                 // offset: 14
        map << target_component;              // offset: 15
        map << idx;                           // offset: 16
        map << count;                         // offset: 17
        map << flags;                         // offset: 18
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> lat;                           // offset: 0
        map >> lng;                           // offset: 4
        map >> alt;                           // offset: 8
        map >> break_alt;                     // offset: 10
        map >> land_dir;                      // offset: 12
        map >> target_system;                 // offset: 14
        map >> target_component;              // offset: 15
        map >> idx;                           // offset: 16
        map >> count;                         // offset: 17
        map >> flags;                         // offset: 18
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
