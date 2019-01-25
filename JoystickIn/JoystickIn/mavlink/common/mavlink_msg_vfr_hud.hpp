// MESSAGE VFR_HUD support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief VFR_HUD message
 *
 * Metrics typically displayed on a HUD for fixed wing aircraft
 */
struct VFR_HUD : mavlink::Message {
    static constexpr msgid_t MSG_ID = 74;
    static constexpr size_t LENGTH = 20;
    static constexpr size_t MIN_LENGTH = 20;
    static constexpr uint8_t CRC_EXTRA = 20;
    static constexpr auto NAME = "VFR_HUD";


    float airspeed; /*< [m/s] Current airspeed */
    float groundspeed; /*< [m/s] Current ground speed */
    int16_t heading; /*< [deg] Current heading in degrees, in compass units (0..360, 0=north) */
    uint16_t throttle; /*< [%] Current throttle setting in integer percent, 0 to 100 */
    float alt; /*< [m] Current altitude (MSL) */
    float climb; /*< [m/s] Current climb rate */


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
        ss << "  airspeed: " << airspeed << std::endl;
        ss << "  groundspeed: " << groundspeed << std::endl;
        ss << "  heading: " << heading << std::endl;
        ss << "  throttle: " << throttle << std::endl;
        ss << "  alt: " << alt << std::endl;
        ss << "  climb: " << climb << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << airspeed;                      // offset: 0
        map << groundspeed;                   // offset: 4
        map << alt;                           // offset: 8
        map << climb;                         // offset: 12
        map << heading;                       // offset: 16
        map << throttle;                      // offset: 18
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> airspeed;                      // offset: 0
        map >> groundspeed;                   // offset: 4
        map >> alt;                           // offset: 8
        map >> climb;                         // offset: 12
        map >> heading;                       // offset: 16
        map >> throttle;                      // offset: 18
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
