// MESSAGE OPTICAL_FLOW support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief OPTICAL_FLOW message
 *
 * Optical flow from a flow sensor (e.g. optical mouse sensor)
 */
struct OPTICAL_FLOW : mavlink::Message {
    static constexpr msgid_t MSG_ID = 100;
    static constexpr size_t LENGTH = 34;
    static constexpr size_t MIN_LENGTH = 26;
    static constexpr uint8_t CRC_EXTRA = 175;
    static constexpr auto NAME = "OPTICAL_FLOW";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    uint8_t sensor_id; /*<  Sensor ID */
    int16_t flow_x; /*< [dpix] Flow in x-sensor direction */
    int16_t flow_y; /*< [dpix] Flow in y-sensor direction */
    float flow_comp_m_x; /*< [m] Flow in x-sensor direction, angular-speed compensated */
    float flow_comp_m_y; /*< [m] Flow in y-sensor direction, angular-speed compensated */
    uint8_t quality; /*<  Optical flow quality / confidence. 0: bad, 255: maximum quality */
    float ground_distance; /*< [m] Ground distance. Positive value: distance known. Negative value: Unknown distance */
    float flow_rate_x; /*< [rad/s] Flow rate about X axis */
    float flow_rate_y; /*< [rad/s] Flow rate about Y axis */


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
        ss << "  sensor_id: " << +sensor_id << std::endl;
        ss << "  flow_x: " << flow_x << std::endl;
        ss << "  flow_y: " << flow_y << std::endl;
        ss << "  flow_comp_m_x: " << flow_comp_m_x << std::endl;
        ss << "  flow_comp_m_y: " << flow_comp_m_y << std::endl;
        ss << "  quality: " << +quality << std::endl;
        ss << "  ground_distance: " << ground_distance << std::endl;
        ss << "  flow_rate_x: " << flow_rate_x << std::endl;
        ss << "  flow_rate_y: " << flow_rate_y << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << flow_comp_m_x;                 // offset: 8
        map << flow_comp_m_y;                 // offset: 12
        map << ground_distance;               // offset: 16
        map << flow_x;                        // offset: 20
        map << flow_y;                        // offset: 22
        map << sensor_id;                     // offset: 24
        map << quality;                       // offset: 25
        map << flow_rate_x;                   // offset: 26
        map << flow_rate_y;                   // offset: 30
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> flow_comp_m_x;                 // offset: 8
        map >> flow_comp_m_y;                 // offset: 12
        map >> ground_distance;               // offset: 16
        map >> flow_x;                        // offset: 20
        map >> flow_y;                        // offset: 22
        map >> sensor_id;                     // offset: 24
        map >> quality;                       // offset: 25
        map >> flow_rate_x;                   // offset: 26
        map >> flow_rate_y;                   // offset: 30
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
