// MESSAGE RPM support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief RPM message
 *
 * RPM sensor output.
 */
struct RPM : mavlink::Message {
    static constexpr msgid_t MSG_ID = 226;
    static constexpr size_t LENGTH = 8;
    static constexpr size_t MIN_LENGTH = 8;
    static constexpr uint8_t CRC_EXTRA = 207;
    static constexpr auto NAME = "RPM";


    float rpm1; /*<  RPM Sensor1. */
    float rpm2; /*<  RPM Sensor2. */


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
        ss << "  rpm1: " << rpm1 << std::endl;
        ss << "  rpm2: " << rpm2 << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << rpm1;                          // offset: 0
        map << rpm2;                          // offset: 4
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> rpm1;                          // offset: 0
        map >> rpm2;                          // offset: 4
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
