// MESSAGE MAG_CAL_REPORT support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief MAG_CAL_REPORT message
 *
 * Reports results of completed compass calibration. Sent until MAG_CAL_ACK received.
 */
struct MAG_CAL_REPORT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 192;
    static constexpr size_t LENGTH = 50;
    static constexpr size_t MIN_LENGTH = 44;
    static constexpr uint8_t CRC_EXTRA = 36;
    static constexpr auto NAME = "MAG_CAL_REPORT";


    uint8_t compass_id; /*<  Compass being calibrated. */
    uint8_t cal_mask; /*<  Bitmask of compasses being calibrated. */
    uint8_t cal_status; /*<  Calibration Status. */
    uint8_t autosaved; /*<  0=requires a MAV_CMD_DO_ACCEPT_MAG_CAL, 1=saved to parameters. */
    float fitness; /*< [mgauss] RMS milligauss residuals. */
    float ofs_x; /*<  X offset. */
    float ofs_y; /*<  Y offset. */
    float ofs_z; /*<  Z offset. */
    float diag_x; /*<  X diagonal (matrix 11). */
    float diag_y; /*<  Y diagonal (matrix 22). */
    float diag_z; /*<  Z diagonal (matrix 33). */
    float offdiag_x; /*<  X off-diagonal (matrix 12 and 21). */
    float offdiag_y; /*<  Y off-diagonal (matrix 13 and 31). */
    float offdiag_z; /*<  Z off-diagonal (matrix 32 and 23). */
    float orientation_confidence; /*<  Confidence in orientation (higher is better). */
    uint8_t old_orientation; /*<  orientation before calibration. */
    uint8_t new_orientation; /*<  orientation after calibration. */


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
        ss << "  autosaved: " << +autosaved << std::endl;
        ss << "  fitness: " << fitness << std::endl;
        ss << "  ofs_x: " << ofs_x << std::endl;
        ss << "  ofs_y: " << ofs_y << std::endl;
        ss << "  ofs_z: " << ofs_z << std::endl;
        ss << "  diag_x: " << diag_x << std::endl;
        ss << "  diag_y: " << diag_y << std::endl;
        ss << "  diag_z: " << diag_z << std::endl;
        ss << "  offdiag_x: " << offdiag_x << std::endl;
        ss << "  offdiag_y: " << offdiag_y << std::endl;
        ss << "  offdiag_z: " << offdiag_z << std::endl;
        ss << "  orientation_confidence: " << orientation_confidence << std::endl;
        ss << "  old_orientation: " << +old_orientation << std::endl;
        ss << "  new_orientation: " << +new_orientation << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << fitness;                       // offset: 0
        map << ofs_x;                         // offset: 4
        map << ofs_y;                         // offset: 8
        map << ofs_z;                         // offset: 12
        map << diag_x;                        // offset: 16
        map << diag_y;                        // offset: 20
        map << diag_z;                        // offset: 24
        map << offdiag_x;                     // offset: 28
        map << offdiag_y;                     // offset: 32
        map << offdiag_z;                     // offset: 36
        map << compass_id;                    // offset: 40
        map << cal_mask;                      // offset: 41
        map << cal_status;                    // offset: 42
        map << autosaved;                     // offset: 43
        map << orientation_confidence;        // offset: 44
        map << old_orientation;               // offset: 48
        map << new_orientation;               // offset: 49
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> fitness;                       // offset: 0
        map >> ofs_x;                         // offset: 4
        map >> ofs_y;                         // offset: 8
        map >> ofs_z;                         // offset: 12
        map >> diag_x;                        // offset: 16
        map >> diag_y;                        // offset: 20
        map >> diag_z;                        // offset: 24
        map >> offdiag_x;                     // offset: 28
        map >> offdiag_y;                     // offset: 32
        map >> offdiag_z;                     // offset: 36
        map >> compass_id;                    // offset: 40
        map >> cal_mask;                      // offset: 41
        map >> cal_status;                    // offset: 42
        map >> autosaved;                     // offset: 43
        map >> orientation_confidence;        // offset: 44
        map >> old_orientation;               // offset: 48
        map >> new_orientation;               // offset: 49
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
