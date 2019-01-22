// MESSAGE LOCAL_POSITION_NED_COV support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief LOCAL_POSITION_NED_COV message
 *
 * The filtered local position (e.g. fused computer vision and accelerometers). Coordinate frame is right-handed, Z-axis down (aeronautical frame, NED / north-east-down convention)
 */
struct LOCAL_POSITION_NED_COV : mavlink::Message {
    static constexpr msgid_t MSG_ID = 64;
    static constexpr size_t LENGTH = 225;
    static constexpr size_t MIN_LENGTH = 225;
    static constexpr uint8_t CRC_EXTRA = 191;
    static constexpr auto NAME = "LOCAL_POSITION_NED_COV";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    uint8_t estimator_type; /*<  Class id of the estimator this estimate originated from. */
    float x; /*< [m] X Position */
    float y; /*< [m] Y Position */
    float z; /*< [m] Z Position */
    float vx; /*< [m/s] X Speed */
    float vy; /*< [m/s] Y Speed */
    float vz; /*< [m/s] Z Speed */
    float ax; /*< [m/s/s] X Acceleration */
    float ay; /*< [m/s/s] Y Acceleration */
    float az; /*< [m/s/s] Z Acceleration */
    std::array<float, 45> covariance; /*<  Covariance matrix upper right triangular (first nine entries are the first ROW, next eight entries are the second row, etc.) */


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
        ss << "  estimator_type: " << +estimator_type << std::endl;
        ss << "  x: " << x << std::endl;
        ss << "  y: " << y << std::endl;
        ss << "  z: " << z << std::endl;
        ss << "  vx: " << vx << std::endl;
        ss << "  vy: " << vy << std::endl;
        ss << "  vz: " << vz << std::endl;
        ss << "  ax: " << ax << std::endl;
        ss << "  ay: " << ay << std::endl;
        ss << "  az: " << az << std::endl;
        ss << "  covariance: [" << to_string(covariance) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << x;                             // offset: 8
        map << y;                             // offset: 12
        map << z;                             // offset: 16
        map << vx;                            // offset: 20
        map << vy;                            // offset: 24
        map << vz;                            // offset: 28
        map << ax;                            // offset: 32
        map << ay;                            // offset: 36
        map << az;                            // offset: 40
        map << covariance;                    // offset: 44
        map << estimator_type;                // offset: 224
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> x;                             // offset: 8
        map >> y;                             // offset: 12
        map >> z;                             // offset: 16
        map >> vx;                            // offset: 20
        map >> vy;                            // offset: 24
        map >> vz;                            // offset: 28
        map >> ax;                            // offset: 32
        map >> ay;                            // offset: 36
        map >> az;                            // offset: 40
        map >> covariance;                    // offset: 44
        map >> estimator_type;                // offset: 224
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
