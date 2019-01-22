// MESSAGE DATA16 support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief DATA16 message
 *
 * Data packet, size 16.
 */
struct DATA16 : mavlink::Message {
    static constexpr msgid_t MSG_ID = 169;
    static constexpr size_t LENGTH = 18;
    static constexpr size_t MIN_LENGTH = 18;
    static constexpr uint8_t CRC_EXTRA = 234;
    static constexpr auto NAME = "DATA16";


    uint8_t type; /*<  Data type. */
    uint8_t len; /*< [bytes] Data length. */
    std::array<uint8_t, 16> data; /*<  Raw data. */


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
        ss << "  type: " << +type << std::endl;
        ss << "  len: " << +len << std::endl;
        ss << "  data: [" << to_string(data) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << type;                          // offset: 0
        map << len;                           // offset: 1
        map << data;                          // offset: 2
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> type;                          // offset: 0
        map >> len;                           // offset: 1
        map >> data;                          // offset: 2
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
