// MESSAGE RAW_PRESSURE support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief RAW_PRESSURE message
 *
 * The RAW pressure readings for the typical setup of one absolute pressure and one differential pressure sensor. The sensor values should be the raw, UNSCALED ADC values.
 */
struct RAW_PRESSURE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 28;
    static constexpr size_t LENGTH = 16;
    static constexpr size_t MIN_LENGTH = 16;
    static constexpr uint8_t CRC_EXTRA = 67;
    static constexpr auto NAME = "RAW_PRESSURE";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    int16_t press_abs; /*<  Absolute pressure (raw) */
    int16_t press_diff1; /*<  Differential pressure 1 (raw, 0 if nonexistent) */
    int16_t press_diff2; /*<  Differential pressure 2 (raw, 0 if nonexistent) */
    int16_t temperature; /*<  Raw Temperature measurement (raw) */


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
        ss << "  time_usec: " << time_usec << std::endl;
        ss << "  press_abs: " << press_abs << std::endl;
        ss << "  press_diff1: " << press_diff1 << std::endl;
        ss << "  press_diff2: " << press_diff2 << std::endl;
        ss << "  temperature: " << temperature << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << press_abs;                     // offset: 8
        map << press_diff1;                   // offset: 10
        map << press_diff2;                   // offset: 12
        map << temperature;                   // offset: 14
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> press_abs;                     // offset: 8
        map >> press_diff1;                   // offset: 10
        map >> press_diff2;                   // offset: 12
        map >> temperature;                   // offset: 14
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
