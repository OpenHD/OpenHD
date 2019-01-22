// MESSAGE WIND support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief WIND message
 *
 * Wind estimation.
 */
struct WIND : mavlink::Message {
    static constexpr msgid_t MSG_ID = 168;
    static constexpr size_t LENGTH = 12;
    static constexpr size_t MIN_LENGTH = 12;
    static constexpr uint8_t CRC_EXTRA = 1;
    static constexpr auto NAME = "WIND";


    float direction; /*< [deg] Wind direction (that wind is coming from). */
    float speed; /*< [m/s] Wind speed in ground plane. */
    float speed_z; /*< [m/s] Vertical wind speed. */


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
        ss << "  direction: " << direction << std::endl;
        ss << "  speed: " << speed << std::endl;
        ss << "  speed_z: " << speed_z << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << direction;                     // offset: 0
        map << speed;                         // offset: 4
        map << speed_z;                       // offset: 8
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> direction;                     // offset: 0
        map >> speed;                         // offset: 4
        map >> speed_z;                       // offset: 8
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
