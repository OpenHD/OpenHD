// MESSAGE AOA_SSA support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief AOA_SSA message
 *
 * Angle of Attack and Side Slip Angle.
 */
struct AOA_SSA : mavlink::Message {
    static constexpr msgid_t MSG_ID = 11020;
    static constexpr size_t LENGTH = 16;
    static constexpr size_t MIN_LENGTH = 16;
    static constexpr uint8_t CRC_EXTRA = 205;
    static constexpr auto NAME = "AOA_SSA";


    uint64_t time_usec; /*< [us] Timestamp (since boot or Unix epoch). */
    float AOA; /*< [deg] Angle of Attack. */
    float SSA; /*< [deg] Side Slip Angle. */


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
        ss << "  AOA: " << AOA << std::endl;
        ss << "  SSA: " << SSA << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << AOA;                           // offset: 8
        map << SSA;                           // offset: 12
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> AOA;                           // offset: 8
        map >> SSA;                           // offset: 12
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
