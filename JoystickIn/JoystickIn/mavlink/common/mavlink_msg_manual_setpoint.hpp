// MESSAGE MANUAL_SETPOINT support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief MANUAL_SETPOINT message
 *
 * Setpoint in roll, pitch, yaw and thrust from the operator
 */
struct MANUAL_SETPOINT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 81;
    static constexpr size_t LENGTH = 22;
    static constexpr size_t MIN_LENGTH = 22;
    static constexpr uint8_t CRC_EXTRA = 106;
    static constexpr auto NAME = "MANUAL_SETPOINT";


    uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot). */
    float roll; /*< [rad/s] Desired roll rate */
    float pitch; /*< [rad/s] Desired pitch rate */
    float yaw; /*< [rad/s] Desired yaw rate */
    float thrust; /*<  Collective thrust, normalized to 0 .. 1 */
    uint8_t mode_switch; /*<  Flight mode switch position, 0.. 255 */
    uint8_t manual_override_switch; /*<  Override mode switch position, 0.. 255 */


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
        ss << "  roll: " << roll << std::endl;
        ss << "  pitch: " << pitch << std::endl;
        ss << "  yaw: " << yaw << std::endl;
        ss << "  thrust: " << thrust << std::endl;
        ss << "  mode_switch: " << +mode_switch << std::endl;
        ss << "  manual_override_switch: " << +manual_override_switch << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_boot_ms;                  // offset: 0
        map << roll;                          // offset: 4
        map << pitch;                         // offset: 8
        map << yaw;                           // offset: 12
        map << thrust;                        // offset: 16
        map << mode_switch;                   // offset: 20
        map << manual_override_switch;        // offset: 21
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_boot_ms;                  // offset: 0
        map >> roll;                          // offset: 4
        map >> pitch;                         // offset: 8
        map >> yaw;                           // offset: 12
        map >> thrust;                        // offset: 16
        map >> mode_switch;                   // offset: 20
        map >> manual_override_switch;        // offset: 21
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
