// MESSAGE SET_ATTITUDE_TARGET support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief SET_ATTITUDE_TARGET message
 *
 * Sets a desired vehicle attitude. Used by an external controller to command the vehicle (manual controller or other system).
 */
struct SET_ATTITUDE_TARGET : mavlink::Message {
    static constexpr msgid_t MSG_ID = 82;
    static constexpr size_t LENGTH = 39;
    static constexpr size_t MIN_LENGTH = 39;
    static constexpr uint8_t CRC_EXTRA = 49;
    static constexpr auto NAME = "SET_ATTITUDE_TARGET";


    uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot). */
    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
    uint8_t type_mask; /*<  Mappings: If any of these bits are set, the corresponding input should be ignored: bit 1: body roll rate, bit 2: body pitch rate, bit 3: body yaw rate. bit 4-bit 6: reserved, bit 7: throttle, bit 8: attitude */
    std::array<float, 4> q; /*<  Attitude quaternion (w, x, y, z order, zero-rotation is 1, 0, 0, 0) */
    float body_roll_rate; /*< [rad/s] Body roll rate */
    float body_pitch_rate; /*< [rad/s] Body pitch rate */
    float body_yaw_rate; /*< [rad/s] Body yaw rate */
    float thrust; /*<  Collective thrust, normalized to 0 .. 1 (-1 .. 1 for vehicles capable of reverse trust) */


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
        ss << "  target_system: " << +target_system << std::endl;
        ss << "  target_component: " << +target_component << std::endl;
        ss << "  type_mask: " << +type_mask << std::endl;
        ss << "  q: [" << to_string(q) << "]" << std::endl;
        ss << "  body_roll_rate: " << body_roll_rate << std::endl;
        ss << "  body_pitch_rate: " << body_pitch_rate << std::endl;
        ss << "  body_yaw_rate: " << body_yaw_rate << std::endl;
        ss << "  thrust: " << thrust << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_boot_ms;                  // offset: 0
        map << q;                             // offset: 4
        map << body_roll_rate;                // offset: 20
        map << body_pitch_rate;               // offset: 24
        map << body_yaw_rate;                 // offset: 28
        map << thrust;                        // offset: 32
        map << target_system;                 // offset: 36
        map << target_component;              // offset: 37
        map << type_mask;                     // offset: 38
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_boot_ms;                  // offset: 0
        map >> q;                             // offset: 4
        map >> body_roll_rate;                // offset: 20
        map >> body_pitch_rate;               // offset: 24
        map >> body_yaw_rate;                 // offset: 28
        map >> thrust;                        // offset: 32
        map >> target_system;                 // offset: 36
        map >> target_component;              // offset: 37
        map >> type_mask;                     // offset: 38
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
