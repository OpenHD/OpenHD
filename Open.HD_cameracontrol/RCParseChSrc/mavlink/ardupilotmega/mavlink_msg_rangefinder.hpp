// MESSAGE RANGEFINDER support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief RANGEFINDER message
 *
 * Rangefinder reporting.
 */
struct RANGEFINDER : mavlink::Message {
    static constexpr msgid_t MSG_ID = 173;
    static constexpr size_t LENGTH = 8;
    static constexpr size_t MIN_LENGTH = 8;
    static constexpr uint8_t CRC_EXTRA = 83;
    static constexpr auto NAME = "RANGEFINDER";


    float distance; /*< [m] Distance. */
    float voltage; /*< [V] Raw voltage if available, zero otherwise. */


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
        ss << "  distance: " << distance << std::endl;
        ss << "  voltage: " << voltage << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << distance;                      // offset: 0
        map << voltage;                       // offset: 4
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> distance;                      // offset: 0
        map >> voltage;                       // offset: 4
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
