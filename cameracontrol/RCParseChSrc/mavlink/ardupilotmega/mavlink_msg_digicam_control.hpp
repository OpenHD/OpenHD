// MESSAGE DIGICAM_CONTROL support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief DIGICAM_CONTROL message
 *
 * Control on-board Camera Control System to take shots.
 */
struct DIGICAM_CONTROL : mavlink::Message {
    static constexpr msgid_t MSG_ID = 155;
    static constexpr size_t LENGTH = 13;
    static constexpr size_t MIN_LENGTH = 13;
    static constexpr uint8_t CRC_EXTRA = 22;
    static constexpr auto NAME = "DIGICAM_CONTROL";


    uint8_t target_system; /*<  System ID. */
    uint8_t target_component; /*<  Component ID. */
    uint8_t session; /*<  0: stop, 1: start or keep it up //Session control e.g. show/hide lens. */
    uint8_t zoom_pos; /*<  1 to N //Zoom's absolute position (0 means ignore). */
    int8_t zoom_step; /*<  -100 to 100 //Zooming step value to offset zoom from the current position. */
    uint8_t focus_lock; /*<  0: unlock focus or keep unlocked, 1: lock focus or keep locked, 3: re-lock focus. */
    uint8_t shot; /*<  0: ignore, 1: shot or start filming. */
    uint8_t command_id; /*<  Command Identity (incremental loop: 0 to 255)//A command sent multiple times will be executed or pooled just once. */
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
        ss << "  session: " << +session << std::endl;
        ss << "  zoom_pos: " << +zoom_pos << std::endl;
        ss << "  zoom_step: " << +zoom_step << std::endl;
        ss << "  focus_lock: " << +focus_lock << std::endl;
        ss << "  shot: " << +shot << std::endl;
        ss << "  command_id: " << +command_id << std::endl;
        ss << "  extra_param: " << +extra_param << std::endl;
        ss << "  extra_value: " << extra_value << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << extra_value;                   // offset: 0
        map << target_system;                 // offset: 4
        map << target_component;              // offset: 5
        map << session;                       // offset: 6
        map << zoom_pos;                      // offset: 7
        map << zoom_step;                     // offset: 8
        map << focus_lock;                    // offset: 9
        map << shot;                          // offset: 10
        map << command_id;                    // offset: 11
        map << extra_param;                   // offset: 12
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> extra_value;                   // offset: 0
        map >> target_system;                 // offset: 4
        map >> target_component;              // offset: 5
        map >> session;                       // offset: 6
        map >> zoom_pos;                      // offset: 7
        map >> zoom_step;                     // offset: 8
        map >> focus_lock;                    // offset: 9
        map >> shot;                          // offset: 10
        map >> command_id;                    // offset: 11
        map >> extra_param;                   // offset: 12
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
