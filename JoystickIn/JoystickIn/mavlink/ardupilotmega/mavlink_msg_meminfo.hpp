// MESSAGE MEMINFO support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief MEMINFO message
 *
 * State of APM memory.
 */
struct MEMINFO : mavlink::Message {
    static constexpr msgid_t MSG_ID = 152;
    static constexpr size_t LENGTH = 8;
    static constexpr size_t MIN_LENGTH = 4;
    static constexpr uint8_t CRC_EXTRA = 208;
    static constexpr auto NAME = "MEMINFO";


    uint16_t brkval; /*<  Heap top. */
    uint16_t freemem; /*< [bytes] Free memory. */
    uint32_t freemem32; /*< [bytes] Free memory (32 bit). */


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
        ss << "  brkval: " << brkval << std::endl;
        ss << "  freemem: " << freemem << std::endl;
        ss << "  freemem32: " << freemem32 << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << brkval;                        // offset: 0
        map << freemem;                       // offset: 2
        map << freemem32;                     // offset: 4
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> brkval;                        // offset: 0
        map >> freemem;                       // offset: 2
        map >> freemem32;                     // offset: 4
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
