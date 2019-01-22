// MESSAGE OBSTACLE_DISTANCE support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief OBSTACLE_DISTANCE message
 *
 * Obstacle distances in front of the sensor, starting from the left in increment degrees to the right
 */
struct OBSTACLE_DISTANCE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 330;
    static constexpr size_t LENGTH = 158;
    static constexpr size_t MIN_LENGTH = 158;
    static constexpr uint8_t CRC_EXTRA = 23;
    static constexpr auto NAME = "OBSTACLE_DISTANCE";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    uint8_t sensor_type; /*<  Class id of the distance sensor type. */
    std::array<uint16_t, 72> distances; /*< [cm] Distance of obstacles around the UAV with index 0 corresponding to local North. A value of 0 means that the obstacle is right in front of the sensor. A value of max_distance +1 means no obstacle is present. A value of UINT16_MAX for unknown/not used. In a array element, one unit corresponds to 1cm. */
    uint8_t increment; /*< [deg] Angular width in degrees of each array element. */
    uint16_t min_distance; /*< [cm] Minimum distance the sensor can measure. */
    uint16_t max_distance; /*< [cm] Maximum distance the sensor can measure. */


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
        ss << "  sensor_type: " << +sensor_type << std::endl;
        ss << "  distances: [" << to_string(distances) << "]" << std::endl;
        ss << "  increment: " << +increment << std::endl;
        ss << "  min_distance: " << min_distance << std::endl;
        ss << "  max_distance: " << max_distance << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << distances;                     // offset: 8
        map << min_distance;                  // offset: 152
        map << max_distance;                  // offset: 154
        map << sensor_type;                   // offset: 156
        map << increment;                     // offset: 157
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> distances;                     // offset: 8
        map >> min_distance;                  // offset: 152
        map >> max_distance;                  // offset: 154
        map >> sensor_type;                   // offset: 156
        map >> increment;                     // offset: 157
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
