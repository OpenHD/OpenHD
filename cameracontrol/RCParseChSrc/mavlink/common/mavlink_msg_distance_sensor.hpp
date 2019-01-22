// MESSAGE DISTANCE_SENSOR support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief DISTANCE_SENSOR message
 *
 * 
 */
struct DISTANCE_SENSOR : mavlink::Message {
    static constexpr msgid_t MSG_ID = 132;
    static constexpr size_t LENGTH = 14;
    static constexpr size_t MIN_LENGTH = 14;
    static constexpr uint8_t CRC_EXTRA = 85;
    static constexpr auto NAME = "DISTANCE_SENSOR";


    uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot). */
    uint16_t min_distance; /*< [cm] Minimum distance the sensor can measure */
    uint16_t max_distance; /*< [cm] Maximum distance the sensor can measure */
    uint16_t current_distance; /*< [cm] Current distance reading */
    uint8_t type; /*<  Type of distance sensor. */
    uint8_t id; /*<  Onboard ID of the sensor */
    uint8_t orientation; /*<  Direction the sensor faces. downward-facing: ROTATION_PITCH_270, upward-facing: ROTATION_PITCH_90, backward-facing: ROTATION_PITCH_180, forward-facing: ROTATION_NONE, left-facing: ROTATION_YAW_90, right-facing: ROTATION_YAW_270 */
    uint8_t covariance; /*< [cm] Measurement covariance, 0 for unknown / invalid readings */


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
        ss << "  min_distance: " << min_distance << std::endl;
        ss << "  max_distance: " << max_distance << std::endl;
        ss << "  current_distance: " << current_distance << std::endl;
        ss << "  type: " << +type << std::endl;
        ss << "  id: " << +id << std::endl;
        ss << "  orientation: " << +orientation << std::endl;
        ss << "  covariance: " << +covariance << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_boot_ms;                  // offset: 0
        map << min_distance;                  // offset: 4
        map << max_distance;                  // offset: 6
        map << current_distance;              // offset: 8
        map << type;                          // offset: 10
        map << id;                            // offset: 11
        map << orientation;                   // offset: 12
        map << covariance;                    // offset: 13
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_boot_ms;                  // offset: 0
        map >> min_distance;                  // offset: 4
        map >> max_distance;                  // offset: 6
        map >> current_distance;              // offset: 8
        map >> type;                          // offset: 10
        map >> id;                            // offset: 11
        map >> orientation;                   // offset: 12
        map >> covariance;                    // offset: 13
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
