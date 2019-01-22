// MESSAGE AP_ADC support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief AP_ADC message
 *
 * Raw ADC output.
 */
struct AP_ADC : mavlink::Message {
    static constexpr msgid_t MSG_ID = 153;
    static constexpr size_t LENGTH = 12;
    static constexpr size_t MIN_LENGTH = 12;
    static constexpr uint8_t CRC_EXTRA = 188;
    static constexpr auto NAME = "AP_ADC";


    uint16_t adc1; /*<  ADC output 1. */
    uint16_t adc2; /*<  ADC output 2. */
    uint16_t adc3; /*<  ADC output 3. */
    uint16_t adc4; /*<  ADC output 4. */
    uint16_t adc5; /*<  ADC output 5. */
    uint16_t adc6; /*<  ADC output 6. */


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
        ss << "  adc1: " << adc1 << std::endl;
        ss << "  adc2: " << adc2 << std::endl;
        ss << "  adc3: " << adc3 << std::endl;
        ss << "  adc4: " << adc4 << std::endl;
        ss << "  adc5: " << adc5 << std::endl;
        ss << "  adc6: " << adc6 << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << adc1;                          // offset: 0
        map << adc2;                          // offset: 2
        map << adc3;                          // offset: 4
        map << adc4;                          // offset: 6
        map << adc5;                          // offset: 8
        map << adc6;                          // offset: 10
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> adc1;                          // offset: 0
        map >> adc2;                          // offset: 2
        map >> adc3;                          // offset: 4
        map >> adc4;                          // offset: 6
        map >> adc5;                          // offset: 8
        map >> adc6;                          // offset: 10
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
