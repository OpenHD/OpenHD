// MESSAGE VISION_POSITION_DELTA support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief VISION_POSITION_DELTA message
 *
 * Camera vision based attitude and position deltas.
 */
struct VISION_POSITION_DELTA : mavlink::Message {
    static constexpr msgid_t MSG_ID = 11011;
    static constexpr size_t LENGTH = 44;
    static constexpr size_t MIN_LENGTH = 44;
    static constexpr uint8_t CRC_EXTRA = 106;
    static constexpr auto NAME = "VISION_POSITION_DELTA";


    uint64_t time_usec; /*< [us] Timestamp (synced to UNIX time or since system boot). */
    uint64_t time_delta_usec; /*< [us] Time since the last reported camera frame. */
    std::array<float, 3> angle_delta; /*<  Defines a rotation vector in body frame that rotates the vehicle from the previous to the current orientation. */
    std::array<float, 3> position_delta; /*< [m] Change in position from previous to current frame rotated into body frame (0=forward, 1=right, 2=down). */
    float confidence; /*< [%] Normalised confidence value from 0 to 100. */


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
        ss << "  time_delta_usec: " << time_delta_usec << std::endl;
        ss << "  angle_delta: [" << to_string(angle_delta) << "]" << std::endl;
        ss << "  position_delta: [" << to_string(position_delta) << "]" << std::endl;
        ss << "  confidence: " << confidence << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << time_delta_usec;               // offset: 8
        map << angle_delta;                   // offset: 16
        map << position_delta;                // offset: 28
        map << confidence;                    // offset: 40
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> time_delta_usec;               // offset: 8
        map >> angle_delta;                   // offset: 16
        map >> position_delta;                // offset: 28
        map >> confidence;                    // offset: 40
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
