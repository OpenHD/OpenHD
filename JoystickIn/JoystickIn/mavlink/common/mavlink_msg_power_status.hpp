// MESSAGE POWER_STATUS support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief POWER_STATUS message
 *
 * Power supply status
 */
struct POWER_STATUS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 125;
    static constexpr size_t LENGTH = 6;
    static constexpr size_t MIN_LENGTH = 6;
    static constexpr uint8_t CRC_EXTRA = 203;
    static constexpr auto NAME = "POWER_STATUS";


    uint16_t Vcc; /*< [mV] 5V rail voltage. */
    uint16_t Vservo; /*< [mV] Servo rail voltage. */
    uint16_t flags; /*<  Bitmap of power supply status flags. */


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
        ss << "  Vcc: " << Vcc << std::endl;
        ss << "  Vservo: " << Vservo << std::endl;
        ss << "  flags: " << flags << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << Vcc;                           // offset: 0
        map << Vservo;                        // offset: 2
        map << flags;                         // offset: 4
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> Vcc;                           // offset: 0
        map >> Vservo;                        // offset: 2
        map >> flags;                         // offset: 4
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
