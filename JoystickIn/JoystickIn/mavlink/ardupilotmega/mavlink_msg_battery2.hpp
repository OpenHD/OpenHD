// MESSAGE BATTERY2 support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief BATTERY2 message
 *
 * 2nd Battery status
 */
struct BATTERY2 : mavlink::Message {
    static constexpr msgid_t MSG_ID = 181;
    static constexpr size_t LENGTH = 4;
    static constexpr size_t MIN_LENGTH = 4;
    static constexpr uint8_t CRC_EXTRA = 174;
    static constexpr auto NAME = "BATTERY2";


    uint16_t voltage; /*< [mV] Voltage. */
    int16_t current_battery; /*< [cA] Battery current, -1: autopilot does not measure the current. */


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
        ss << "  voltage: " << voltage << std::endl;
        ss << "  current_battery: " << current_battery << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << voltage;                       // offset: 0
        map << current_battery;               // offset: 2
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> voltage;                       // offset: 0
        map >> current_battery;               // offset: 2
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
