// MESSAGE HWSTATUS support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief HWSTATUS message
 *
 * Status of key hardware.
 */
struct HWSTATUS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 165;
    static constexpr size_t LENGTH = 3;
    static constexpr size_t MIN_LENGTH = 3;
    static constexpr uint8_t CRC_EXTRA = 21;
    static constexpr auto NAME = "HWSTATUS";


    uint16_t Vcc; /*< [mV] Board voltage. */
    uint8_t I2Cerr; /*<  I2C error count. */


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
        ss << "  I2Cerr: " << +I2Cerr << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << Vcc;                           // offset: 0
        map << I2Cerr;                        // offset: 2
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> Vcc;                           // offset: 0
        map >> I2Cerr;                        // offset: 2
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
