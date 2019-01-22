// MESSAGE DIGICAM_CONFIGURE support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief DIGICAM_CONFIGURE message
 *
 * Configure on-board Camera Control System.
 */
struct DIGICAM_CONFIGURE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 154;
    static constexpr size_t LENGTH = 15;
    static constexpr size_t MIN_LENGTH = 15;
    static constexpr uint8_t CRC_EXTRA = 84;
    static constexpr auto NAME = "DIGICAM_CONFIGURE";


    uint8_t target_system; /*<  System ID. */
    uint8_t target_component; /*<  Component ID. */
    uint8_t mode; /*<  Mode enumeration from 1 to N //P, TV, AV, M, etc. (0 means ignore). */
    uint16_t shutter_speed; /*<  Divisor number //e.g. 1000 means 1/1000 (0 means ignore). */
    uint8_t aperture; /*<  F stop number x 10 //e.g. 28 means 2.8 (0 means ignore). */
    uint8_t iso; /*<  ISO enumeration from 1 to N //e.g. 80, 100, 200, Etc (0 means ignore). */
    uint8_t exposure_type; /*<  Exposure type enumeration from 1 to N (0 means ignore). */
    uint8_t command_id; /*<  Command Identity (incremental loop: 0 to 255). //A command sent multiple times will be executed or pooled just once. */
    uint8_t engine_cut_off; /*< [ds] Main engine cut-off time before camera trigger (0 means no cut-off). */
    uint8_t extra_param; /*<  Extra parameters enumeration (0 means ignore). */
    float extra_value; /*<  Correspondent value to given extra_param. */


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
        ss << "  mode: " << +mode << std::endl;
        ss << "  shutter_speed: " << shutter_speed << std::endl;
        ss << "  aperture: " << +aperture << std::endl;
        ss << "  iso: " << +iso << std::endl;
        ss << "  exposure_type: " << +exposure_type << std::endl;
        ss << "  command_id: " << +command_id << std::endl;
        ss << "  engine_cut_off: " << +engine_cut_off << std::endl;
        ss << "  extra_param: " << +extra_param << std::endl;
        ss << "  extra_value: " << extra_value << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << extra_value;                   // offset: 0
        map << shutter_speed;                 // offset: 4
        map << target_system;                 // offset: 6
        map << target_component;              // offset: 7
        map << mode;                          // offset: 8
        map << aperture;                      // offset: 9
        map << iso;                           // offset: 10
        map << exposure_type;                 // offset: 11
        map << command_id;                    // offset: 12
        map << engine_cut_off;                // offset: 13
        map << extra_param;                   // offset: 14
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> extra_value;                   // offset: 0
        map >> shutter_speed;                 // offset: 4
        map >> target_system;                 // offset: 6
        map >> target_component;              // offset: 7
        map >> mode;                          // offset: 8
        map >> aperture;                      // offset: 9
        map >> iso;                           // offset: 10
        map >> exposure_type;                 // offset: 11
        map >> command_id;                    // offset: 12
        map >> engine_cut_off;                // offset: 13
        map >> extra_param;                   // offset: 14
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
