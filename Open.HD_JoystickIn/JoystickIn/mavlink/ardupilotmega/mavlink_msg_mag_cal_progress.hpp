// MESSAGE MAG_CAL_PROGRESS support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief MAG_CAL_PROGRESS message
 *
 * Reports progress of compass calibration.
 */
struct MAG_CAL_PROGRESS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 191;
    static constexpr size_t LENGTH = 27;
    static constexpr size_t MIN_LENGTH = 27;
    static constexpr uint8_t CRC_EXTRA = 92;
    static constexpr auto NAME = "MAG_CAL_PROGRESS";


    uint8_t compass_id; /*<  Compass being calibrated. */
    uint8_t cal_mask; /*<  Bitmask of compasses being calibrated. */
    uint8_t cal_status; /*<  Calibration Status. */
    uint8_t attempt; /*<  Attempt number. */
    uint8_t completion_pct; /*< [%] Completion percentage. */
    std::array<uint8_t, 10> completion_mask; /*<  Bitmask of sphere sections (see http://en.wikipedia.org/wiki/Geodesic_grid). */
    float direction_x; /*<  Body frame direction vector for display. */
    float direction_y; /*<  Body frame direction vector for display. */
    float direction_z; /*<  Body frame direction vector for display. */


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
        ss << "  compass_id: " << +compass_id << std::endl;
        ss << "  cal_mask: " << +cal_mask << std::endl;
        ss << "  cal_status: " << +cal_status << std::endl;
        ss << "  attempt: " << +attempt << std::endl;
        ss << "  completion_pct: " << +completion_pct << std::endl;
        ss << "  completion_mask: [" << to_string(completion_mask) << "]" << std::endl;
        ss << "  direction_x: " << direction_x << std::endl;
        ss << "  direction_y: " << direction_y << std::endl;
        ss << "  direction_z: " << direction_z << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << direction_x;                   // offset: 0
        map << direction_y;                   // offset: 4
        map << direction_z;                   // offset: 8
        map << compass_id;                    // offset: 12
        map << cal_mask;                      // offset: 13
        map << cal_status;                    // offset: 14
        map << attempt;                       // offset: 15
        map << completion_pct;                // offset: 16
        map << completion_mask;               // offset: 17
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> direction_x;                   // offset: 0
        map >> direction_y;                   // offset: 4
        map >> direction_z;                   // offset: 8
        map >> compass_id;                    // offset: 12
        map >> cal_mask;                      // offset: 13
        map >> cal_status;                    // offset: 14
        map >> attempt;                       // offset: 15
        map >> completion_pct;                // offset: 16
        map >> completion_mask;               // offset: 17
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
