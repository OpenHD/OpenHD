// MESSAGE ESC_TELEMETRY_5_TO_8 support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief ESC_TELEMETRY_5_TO_8 message
 *
 * ESC Telemetry Data for ESCs 5 to 8, matching data sent by BLHeli ESCs.
 */
struct ESC_TELEMETRY_5_TO_8 : mavlink::Message {
    static constexpr msgid_t MSG_ID = 11031;
    static constexpr size_t LENGTH = 44;
    static constexpr size_t MIN_LENGTH = 44;
    static constexpr uint8_t CRC_EXTRA = 133;
    static constexpr auto NAME = "ESC_TELEMETRY_5_TO_8";


    std::array<uint8_t, 4> temperature; /*< [degC] Temperature. */
    std::array<uint16_t, 4> voltage; /*< [cV] Voltage. */
    std::array<uint16_t, 4> current; /*< [cA] Current. */
    std::array<uint16_t, 4> totalcurrent; /*< [mAh] Total current. */
    std::array<uint16_t, 4> rpm; /*< [rpm] RPM (eRPM). */
    std::array<uint16_t, 4> count; /*<  count of telemetry packets received (wraps at 65535). */


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
        ss << "  temperature: [" << to_string(temperature) << "]" << std::endl;
        ss << "  voltage: [" << to_string(voltage) << "]" << std::endl;
        ss << "  current: [" << to_string(current) << "]" << std::endl;
        ss << "  totalcurrent: [" << to_string(totalcurrent) << "]" << std::endl;
        ss << "  rpm: [" << to_string(rpm) << "]" << std::endl;
        ss << "  count: [" << to_string(count) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << voltage;                       // offset: 0
        map << current;                       // offset: 8
        map << totalcurrent;                  // offset: 16
        map << rpm;                           // offset: 24
        map << count;                         // offset: 32
        map << temperature;                   // offset: 40
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> voltage;                       // offset: 0
        map >> current;                       // offset: 8
        map >> totalcurrent;                  // offset: 16
        map >> rpm;                           // offset: 24
        map >> count;                         // offset: 32
        map >> temperature;                   // offset: 40
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
