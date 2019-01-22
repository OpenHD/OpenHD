// MESSAGE CAMERA_CAPTURE_STATUS support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief CAMERA_CAPTURE_STATUS message
 *
 * Information about the status of a capture.
 */
struct CAMERA_CAPTURE_STATUS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 262;
    static constexpr size_t LENGTH = 18;
    static constexpr size_t MIN_LENGTH = 18;
    static constexpr uint8_t CRC_EXTRA = 12;
    static constexpr auto NAME = "CAMERA_CAPTURE_STATUS";


    uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot). */
    uint8_t image_status; /*<  Current status of image capturing (0: idle, 1: capture in progress, 2: interval set but idle, 3: interval set and capture in progress) */
    uint8_t video_status; /*<  Current status of video capturing (0: idle, 1: capture in progress) */
    float image_interval; /*< [s] Image capture interval */
    uint32_t recording_time_ms; /*< [ms] Time since recording started */
    float available_capacity; /*< [MiB] Available storage capacity. */


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
        ss << "  time_boot_ms: " << time_boot_ms << std::endl;
        ss << "  image_status: " << +image_status << std::endl;
        ss << "  video_status: " << +video_status << std::endl;
        ss << "  image_interval: " << image_interval << std::endl;
        ss << "  recording_time_ms: " << recording_time_ms << std::endl;
        ss << "  available_capacity: " << available_capacity << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_boot_ms;                  // offset: 0
        map << image_interval;                // offset: 4
        map << recording_time_ms;             // offset: 8
        map << available_capacity;            // offset: 12
        map << image_status;                  // offset: 16
        map << video_status;                  // offset: 17
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_boot_ms;                  // offset: 0
        map >> image_interval;                // offset: 4
        map >> recording_time_ms;             // offset: 8
        map >> available_capacity;            // offset: 12
        map >> image_status;                  // offset: 16
        map >> video_status;                  // offset: 17
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
