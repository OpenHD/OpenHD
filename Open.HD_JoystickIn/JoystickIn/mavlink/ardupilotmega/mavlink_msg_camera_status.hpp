// MESSAGE CAMERA_STATUS support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief CAMERA_STATUS message
 *
 * Camera Event.
 */
struct CAMERA_STATUS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 179;
    static constexpr size_t LENGTH = 29;
    static constexpr size_t MIN_LENGTH = 29;
    static constexpr uint8_t CRC_EXTRA = 189;
    static constexpr auto NAME = "CAMERA_STATUS";


    uint64_t time_usec; /*< [us] Image timestamp (since UNIX epoch, according to camera clock). */
    uint8_t target_system; /*<  System ID. */
    uint8_t cam_idx; /*<  Camera ID. */
    uint16_t img_idx; /*<  Image index. */
    uint8_t event_id; /*<  Event type. */
    float p1; /*<  Parameter 1 (meaning depends on event_id, see CAMERA_STATUS_TYPES enum). */
    float p2; /*<  Parameter 2 (meaning depends on event_id, see CAMERA_STATUS_TYPES enum). */
    float p3; /*<  Parameter 3 (meaning depends on event_id, see CAMERA_STATUS_TYPES enum). */
    float p4; /*<  Parameter 4 (meaning depends on event_id, see CAMERA_STATUS_TYPES enum). */


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
        ss << "  time_usec: " << time_usec << std::endl;
        ss << "  target_system: " << +target_system << std::endl;
        ss << "  cam_idx: " << +cam_idx << std::endl;
        ss << "  img_idx: " << img_idx << std::endl;
        ss << "  event_id: " << +event_id << std::endl;
        ss << "  p1: " << p1 << std::endl;
        ss << "  p2: " << p2 << std::endl;
        ss << "  p3: " << p3 << std::endl;
        ss << "  p4: " << p4 << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << p1;                            // offset: 8
        map << p2;                            // offset: 12
        map << p3;                            // offset: 16
        map << p4;                            // offset: 20
        map << img_idx;                       // offset: 24
        map << target_system;                 // offset: 26
        map << cam_idx;                       // offset: 27
        map << event_id;                      // offset: 28
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> p1;                            // offset: 8
        map >> p2;                            // offset: 12
        map >> p3;                            // offset: 16
        map >> p4;                            // offset: 20
        map >> img_idx;                       // offset: 24
        map >> target_system;                 // offset: 26
        map >> cam_idx;                       // offset: 27
        map >> event_id;                      // offset: 28
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
