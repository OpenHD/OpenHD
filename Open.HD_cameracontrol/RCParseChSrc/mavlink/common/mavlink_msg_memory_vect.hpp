// MESSAGE MEMORY_VECT support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief MEMORY_VECT message
 *
 * Send raw controller memory. The use of this message is discouraged for normal packets, but a quite efficient way for testing new messages and getting experimental debug output.
 */
struct MEMORY_VECT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 249;
    static constexpr size_t LENGTH = 36;
    static constexpr size_t MIN_LENGTH = 36;
    static constexpr uint8_t CRC_EXTRA = 204;
    static constexpr auto NAME = "MEMORY_VECT";


    uint16_t address; /*<  Starting address of the debug variables */
    uint8_t ver; /*<  Version code of the type variable. 0=unknown, type ignored and assumed int16_t. 1=as below */
    uint8_t type; /*<  Type code of the memory variables. for ver = 1: 0=16 x int16_t, 1=16 x uint16_t, 2=16 x Q15, 3=16 x 1Q14 */
    std::array<int8_t, 32> value; /*<  Memory contents at specified address */


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
        ss << "  address: " << address << std::endl;
        ss << "  ver: " << +ver << std::endl;
        ss << "  type: " << +type << std::endl;
        ss << "  value: [" << to_string(value) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << address;                       // offset: 0
        map << ver;                           // offset: 2
        map << type;                          // offset: 3
        map << value;                         // offset: 4
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> address;                       // offset: 0
        map >> ver;                           // offset: 2
        map >> type;                          // offset: 3
        map >> value;                         // offset: 4
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
