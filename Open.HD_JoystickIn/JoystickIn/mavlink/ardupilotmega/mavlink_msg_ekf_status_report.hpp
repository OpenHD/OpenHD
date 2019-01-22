// MESSAGE EKF_STATUS_REPORT support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief EKF_STATUS_REPORT message
 *
 * EKF Status message including flags and variances.
 */
struct EKF_STATUS_REPORT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 193;
    static constexpr size_t LENGTH = 26;
    static constexpr size_t MIN_LENGTH = 22;
    static constexpr uint8_t CRC_EXTRA = 71;
    static constexpr auto NAME = "EKF_STATUS_REPORT";


    uint16_t flags; /*<  Flags. */
    float velocity_variance; /*<  Velocity variance. */
    float pos_horiz_variance; /*<  Horizontal Position variance. */
    float pos_vert_variance; /*<  Vertical Position variance. */
    float compass_variance; /*<  Compass variance. */
    float terrain_alt_variance; /*<  Terrain Altitude variance. */
    float airspeed_variance; /*<  Airspeed variance. */


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
        ss << "  flags: " << flags << std::endl;
        ss << "  velocity_variance: " << velocity_variance << std::endl;
        ss << "  pos_horiz_variance: " << pos_horiz_variance << std::endl;
        ss << "  pos_vert_variance: " << pos_vert_variance << std::endl;
        ss << "  compass_variance: " << compass_variance << std::endl;
        ss << "  terrain_alt_variance: " << terrain_alt_variance << std::endl;
        ss << "  airspeed_variance: " << airspeed_variance << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << velocity_variance;             // offset: 0
        map << pos_horiz_variance;            // offset: 4
        map << pos_vert_variance;             // offset: 8
        map << compass_variance;              // offset: 12
        map << terrain_alt_variance;          // offset: 16
        map << flags;                         // offset: 20
        map << airspeed_variance;             // offset: 22
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> velocity_variance;             // offset: 0
        map >> pos_horiz_variance;            // offset: 4
        map >> pos_vert_variance;             // offset: 8
        map >> compass_variance;              // offset: 12
        map >> terrain_alt_variance;          // offset: 16
        map >> flags;                         // offset: 20
        map >> airspeed_variance;             // offset: 22
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
