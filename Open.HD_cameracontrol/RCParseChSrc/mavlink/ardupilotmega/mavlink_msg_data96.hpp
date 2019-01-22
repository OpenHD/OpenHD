// MESSAGE DATA96 support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief DATA96 message
 *
 * Data packet, size 96.
 */
struct DATA96 : mavlink::Message {
    static constexpr msgid_t MSG_ID = 172;
    static constexpr size_t LENGTH = 98;
    static constexpr size_t MIN_LENGTH = 98;
    static constexpr uint8_t CRC_EXTRA = 22;
    static constexpr auto NAME = "DATA96";


    uint8_t type; /*<  Data type. */
    uint8_t len; /*< [bytes] Data length. */
    std::array<uint8_t, 96> data; /*<  Raw data. */


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
