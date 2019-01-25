// MESSAGE CAMERA_FEEDBACK support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief CAMERA_FEEDBACK message
 *
 * Camera Capture Feedback.
 */
struct CAMERA_FEEDBACK : mavlink::Message {
    static constexpr msgid_t MSG_ID = 180;
    static constexpr size_t LENGTH = 47;
    static constexpr size_t MIN_LENGTH = 45;
    static constexpr uint8_t CRC_EXTRA = 52;
    static constexpr auto NAME = "CAMERA_FEEDBACK";


    uint64_t time_usec; /*< [us] Image timestamp (since UNIX epoch), as passed in by CAMERA_STATUS message (or autopilot if no CCB). */
    uint8_t target_system; /*<  System ID. */
    uint8_t cam_idx; /*<  Camera ID. */
    uint16_t img_idx; /*<  Image index. */
    int32_t lat; /*< [degE7] Latitude. */
    int32_t lng; /*< [degE7] Longitude. */
    float alt_msl; /*< [m] Altitude Absolute (AMSL). */
    float alt_rel; /*< [m] Altitude Relative (above HOME location). */
    float roll; /*< [deg] Camera Roll angle (earth frame, +-180). */
    float pitch; /*< [deg] Camera Pitch angle (earth frame, +-180). */
    float yaw; /*< [deg] Camera Yaw (earth frame, 0-360, true). */
    float foc_len; /*< [mm] Focal Length. */
    uint8_t flags; /*<  Feedback flags. */
    uint16_t completed_captures; /*<  Completed image captures. */


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
        ss << "  lat: " << lat << std::endl;
        ss << "  lng: " << lng << std::endl;
        ss << "  alt_msl: " << alt_msl << std::endl;
        ss << "  alt_rel: " << alt_rel << std::endl;
        ss << "  roll: " << roll << std::endl;
        ss << "  pitch: " << pitch << std::endl;
        ss << "  yaw: " << yaw << std::endl;
        ss << "  foc_len: " << foc_len << std::endl;
        ss << "  flags: " << +flags << std::endl;
        ss << "  completed_captures: " << completed_captures << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << lat;                           // offset: 8
        map << lng;                           // offset: 12
        map << alt_msl;                       // offset: 16
        map << alt_rel;                       // offset: 20
        map << roll;                          // offset: 24
        map << pitch;                         // offset: 28
        map << yaw;                           // offset: 32
        map << foc_len;                       // offset: 36
        map << img_idx;                       // offset: 40
        map << target_system;                 // offset: 42
        map << cam_idx;                       // offset: 43
        map << flags;                         // offset: 44
        map << completed_captures;            // offset: 45
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> lat;                           // offset: 8
        map >> lng;                           // offset: 12
        map >> alt_msl;                       // offset: 16
        map >> alt_rel;                       // offset: 20
        map >> roll;                          // offset: 24
        map >> pitch;                         // offset: 28
        map >> yaw;                           // offset: 32
        map >> foc_len;                       // offset: 36
        map >> img_idx;                       // offset: 40
        map >> target_system;                 // offset: 42
        map >> cam_idx;                       // offset: 43
        map >> flags;                         // offset: 44
        map >> completed_captures;            // offset: 45
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
