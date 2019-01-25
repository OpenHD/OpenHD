// MESSAGE ENCAPSULATED_DATA support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief ENCAPSULATED_DATA message
 *
 * 
 */
struct ENCAPSULATED_DATA : mavlink::Message {
    static constexpr msgid_t MSG_ID = 131;
    static constexpr size_t LENGTH = 255;
    static constexpr size_t MIN_LENGTH = 255;
    static constexpr uint8_t CRC_EXTRA = 223;
    static constexpr auto NAME = "ENCAPSULATED_DATA";


    uint16_t seqnr; /*<  sequence number (starting with 0 on every transmission) */
    std::array<uint8_t, 253> data; /*<  image data bytes */


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
        ss << "  seqnr: " << seqnr << std::endl;
        ss << "  data: [" << to_string(data) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << seqnr;                         // offset: 0
        map << data;                          // offset: 2
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> seqnr;                         // offset: 0
        map >> data;                          // offset: 2
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
