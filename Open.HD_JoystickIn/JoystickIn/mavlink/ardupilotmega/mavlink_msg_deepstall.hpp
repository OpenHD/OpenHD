// MESSAGE DEEPSTALL support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief DEEPSTALL message
 *
 * Deepstall path planning.
 */
struct DEEPSTALL : mavlink::Message {
    static constexpr msgid_t MSG_ID = 195;
    static constexpr size_t LENGTH = 37;
    static constexpr size_t MIN_LENGTH = 37;
    static constexpr uint8_t CRC_EXTRA = 120;
    static constexpr auto NAME = "DEEPSTALL";


    int32_t landing_lat; /*< [degE7] Landing latitude. */
    int32_t landing_lon; /*< [degE7] Landing longitude. */
    int32_t path_lat; /*< [degE7] Final heading start point, latitude. */
    int32_t path_lon; /*< [degE7] Final heading start point, longitude. */
    int32_t arc_entry_lat; /*< [degE7] Arc entry point, latitude. */
    int32_t arc_entry_lon; /*< [degE7] Arc entry point, longitude. */
    float altitude; /*< [m] Altitude. */
    float expected_travel_distance; /*< [m] Distance the aircraft expects to travel during the deepstall. */
    float cross_track_error; /*< [m] Deepstall cross track error (only valid when in DEEPSTALL_STAGE_LAND). */
    uint8_t stage; /*<  Deepstall stage. */


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
        ss << "  landing_lat: " << landing_lat << std::endl;
        ss << "  landing_lon: " << landing_lon << std::endl;
        ss << "  path_lat: " << path_lat << std::endl;
        ss << "  path_lon: " << path_lon << std::endl;
        ss << "  arc_entry_lat: " << arc_entry_lat << std::endl;
        ss << "  arc_entry_lon: " << arc_entry_lon << std::endl;
        ss << "  altitude: " << altitude << std::endl;
        ss << "  expected_travel_distance: " << expected_travel_distance << std::endl;
        ss << "  cross_track_error: " << cross_track_error << std::endl;
        ss << "  stage: " << +stage << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << landing_lat;                   // offset: 0
        map << landing_lon;                   // offset: 4
        map << path_lat;                      // offset: 8
        map << path_lon;                      // offset: 12
        map << arc_entry_lat;                 // offset: 16
        map << arc_entry_lon;                 // offset: 20
        map << altitude;                      // offset: 24
        map << expected_travel_distance;      // offset: 28
        map << cross_track_error;             // offset: 32
        map << stage;                         // offset: 36
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> landing_lat;                   // offset: 0
        map >> landing_lon;                   // offset: 4
        map >> path_lat;                      // offset: 8
        map >> path_lon;                      // offset: 12
        map >> arc_entry_lat;                 // offset: 16
        map >> arc_entry_lon;                 // offset: 20
        map >> altitude;                      // offset: 24
        map >> expected_travel_distance;      // offset: 28
        map >> cross_track_error;             // offset: 32
        map >> stage;                         // offset: 36
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
