// MESSAGE ESTIMATOR_STATUS support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief ESTIMATOR_STATUS message
 *
 * Estimator status message including flags, innovation test ratios and estimated accuracies. The flags message is an integer bitmask containing information on which EKF outputs are valid. See the ESTIMATOR_STATUS_FLAGS enum definition for further information. The innovation test ratios show the magnitude of the sensor innovation divided by the innovation check threshold. Under normal operation the innovation test ratios should be below 0.5 with occasional values up to 1.0. Values greater than 1.0 should be rare under normal operation and indicate that a measurement has been rejected by the filter. The user should be notified if an innovation test ratio greater than 1.0 is recorded. Notifications for values in the range between 0.5 and 1.0 should be optional and controllable by the user.
 */
struct ESTIMATOR_STATUS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 230;
    static constexpr size_t LENGTH = 42;
    static constexpr size_t MIN_LENGTH = 42;
    static constexpr uint8_t CRC_EXTRA = 163;
    static constexpr auto NAME = "ESTIMATOR_STATUS";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    uint16_t flags; /*<  Bitmap indicating which EKF outputs are valid. */
    float vel_ratio; /*<  Velocity innovation test ratio */
    float pos_horiz_ratio; /*<  Horizontal position innovation test ratio */
    float pos_vert_ratio; /*<  Vertical position innovation test ratio */
    float mag_ratio; /*<  Magnetometer innovation test ratio */
    float hagl_ratio; /*<  Height above terrain innovation test ratio */
    float tas_ratio; /*<  True airspeed innovation test ratio */
    float pos_horiz_accuracy; /*< [m] Horizontal position 1-STD accuracy relative to the EKF local origin */
    float pos_vert_accuracy; /*< [m] Vertical position 1-STD accuracy relative to the EKF local origin */


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
        ss << "  flags: " << flags << std::endl;
        ss << "  vel_ratio: " << vel_ratio << std::endl;
        ss << "  pos_horiz_ratio: " << pos_horiz_ratio << std::endl;
        ss << "  pos_vert_ratio: " << pos_vert_ratio << std::endl;
        ss << "  mag_ratio: " << mag_ratio << std::endl;
        ss << "  hagl_ratio: " << hagl_ratio << std::endl;
        ss << "  tas_ratio: " << tas_ratio << std::endl;
        ss << "  pos_horiz_accuracy: " << pos_horiz_accuracy << std::endl;
        ss << "  pos_vert_accuracy: " << pos_vert_accuracy << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << vel_ratio;                     // offset: 8
        map << pos_horiz_ratio;               // offset: 12
        map << pos_vert_ratio;                // offset: 16
        map << mag_ratio;                     // offset: 20
        map << hagl_ratio;                    // offset: 24
        map << tas_ratio;                     // offset: 28
        map << pos_horiz_accuracy;            // offset: 32
        map << pos_vert_accuracy;             // offset: 36
        map << flags;                         // offset: 40
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> vel_ratio;                     // offset: 8
        map >> pos_horiz_ratio;               // offset: 12
        map >> pos_vert_ratio;                // offset: 16
        map >> mag_ratio;                     // offset: 20
        map >> hagl_ratio;                    // offset: 24
        map >> tas_ratio;                     // offset: 28
        map >> pos_horiz_accuracy;            // offset: 32
        map >> pos_vert_accuracy;             // offset: 36
        map >> flags;                         // offset: 40
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
