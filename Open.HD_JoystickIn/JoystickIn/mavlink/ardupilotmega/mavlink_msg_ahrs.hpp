// MESSAGE AHRS support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief AHRS message
 *
 * Status of DCM attitude estimator.
 */
struct AHRS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 163;
    static constexpr size_t LENGTH = 28;
    static constexpr size_t MIN_LENGTH = 28;
    static constexpr uint8_t CRC_EXTRA = 127;
    static constexpr auto NAME = "AHRS";


    float omegaIx; /*< [rad/s] X gyro drift estimate. */
    float omegaIy; /*< [rad/s] Y gyro drift estimate. */
    float omegaIz; /*< [rad/s] Z gyro drift estimate. */
    float accel_weight; /*<  Average accel_weight. */
    float renorm_val; /*<  Average renormalisation value. */
    float error_rp; /*<  Average error_roll_pitch value. */
    float error_yaw; /*<  Average error_yaw value. */


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
        ss << "  omegaIx: " << omegaIx << std::endl;
        ss << "  omegaIy: " << omegaIy << std::endl;
        ss << "  omegaIz: " << omegaIz << std::endl;
        ss << "  accel_weight: " << accel_weight << std::endl;
        ss << "  renorm_val: " << renorm_val << std::endl;
        ss << "  error_rp: " << error_rp << std::endl;
        ss << "  error_yaw: " << error_yaw << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << omegaIx;                       // offset: 0
        map << omegaIy;                       // offset: 4
        map << omegaIz;                       // offset: 8
        map << accel_weight;                  // offset: 12
        map << renorm_val;                    // offset: 16
        map << error_rp;                      // offset: 20
        map << error_yaw;                     // offset: 24
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> omegaIx;                       // offset: 0
        map >> omegaIy;                       // offset: 4
        map >> omegaIz;                       // offset: 8
        map >> accel_weight;                  // offset: 12
        map >> renorm_val;                    // offset: 16
        map >> error_rp;                      // offset: 20
        map >> error_yaw;                     // offset: 24
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
