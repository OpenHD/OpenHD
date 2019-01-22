// MESSAGE HIL_OPTICAL_FLOW support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief HIL_OPTICAL_FLOW message
 *
 * Simulated optical flow from a flow sensor (e.g. PX4FLOW or optical mouse sensor)
 */
struct HIL_OPTICAL_FLOW : mavlink::Message {
    static constexpr msgid_t MSG_ID = 114;
    static constexpr size_t LENGTH = 44;
    static constexpr size_t MIN_LENGTH = 44;
    static constexpr uint8_t CRC_EXTRA = 237;
    static constexpr auto NAME = "HIL_OPTICAL_FLOW";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    uint8_t sensor_id; /*<  Sensor ID */
    uint32_t integration_time_us; /*< [us] Integration time. Divide integrated_x and integrated_y by the integration time to obtain average flow. The integration time also indicates the. */
    float integrated_x; /*< [rad] Flow in radians around X axis (Sensor RH rotation about the X axis induces a positive flow. Sensor linear motion along the positive Y axis induces a negative flow.) */
    float integrated_y; /*< [rad] Flow in radians around Y axis (Sensor RH rotation about the Y axis induces a positive flow. Sensor linear motion along the positive X axis induces a positive flow.) */
    float integrated_xgyro; /*< [rad] RH rotation around X axis */
    float integrated_ygyro; /*< [rad] RH rotation around Y axis */
    float integrated_zgyro; /*< [rad] RH rotation around Z axis */
    int16_t temperature; /*< [cdegC] Temperature */
    uint8_t quality; /*<  Optical flow quality / confidence. 0: no valid flow, 255: maximum quality */
    uint32_t time_delta_distance_us; /*< [us] Time since the distance was sampled. */
    float distance; /*< [m] Distance to the center of the flow field. Positive value (including zero): distance known. Negative value: Unknown distance. */


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
        ss << "  integration_time_us: " << integration_time_us << std::endl;
        ss << "  integrated_x: " << integrated_x << std::endl;
        ss << "  integrated_y: " << integrated_y << std::endl;
        ss << "  integrated_xgyro: " << integrated_xgyro << std::endl;
        ss << "  integrated_ygyro: " << integrated_ygyro << std::endl;
        ss << "  integrated_zgyro: " << integrated_zgyro << std::endl;
        ss << "  temperature: " << temperature << std::endl;
        ss << "  quality: " << +quality << std::endl;
        ss << "  time_delta_distance_us: " << time_delta_distance_us << std::endl;
        ss << "  distance: " << distance << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << integration_time_us;           // offset: 8
        map << integrated_x;                  // offset: 12
        map << integrated_y;                  // offset: 16
        map << integrated_xgyro;              // offset: 20
        map << integrated_ygyro;              // offset: 24
        map << integrated_zgyro;              // offset: 28
        map << time_delta_distance_us;        // offset: 32
        map << distance;                      // offset: 36
        map << temperature;                   // offset: 40
        map << sensor_id;                     // offset: 42
        map << quality;                       // offset: 43
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> integration_time_us;           // offset: 8
        map >> integrated_x;                  // offset: 12
        map >> integrated_y;                  // offset: 16
        map >> integrated_xgyro;              // offset: 20
        map >> integrated_ygyro;              // offset: 24
        map >> integrated_zgyro;              // offset: 28
        map >> time_delta_distance_us;        // offset: 32
        map >> distance;                      // offset: 36
        map >> temperature;                   // offset: 40
        map >> sensor_id;                     // offset: 42
        map >> quality;                       // offset: 43
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
