// MESSAGE FILE_TRANSFER_PROTOCOL support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief FILE_TRANSFER_PROTOCOL message
 *
 * File transfer message
 */
struct FILE_TRANSFER_PROTOCOL : mavlink::Message {
    static constexpr msgid_t MSG_ID = 110;
    static constexpr size_t LENGTH = 254;
    static constexpr size_t MIN_LENGTH = 254;
    static constexpr uint8_t CRC_EXTRA = 84;
    static constexpr auto NAME = "FILE_TRANSFER_PROTOCOL";


    uint8_t target_network; /*<  Network ID (0 for broadcast) */
    uint8_t target_system; /*<  System ID (0 for broadcast) */
    uint8_t target_component; /*<  Component ID (0 for broadcast) */
    std::array<uint8_t, 251> payload; /*<  Variable length payload. The length is defined by the remaining message length when subtracting the header and other fields.  The entire content of this block is opaque unless you understand any the encoding message_type.  The particular encoding used can be extension specific and might not always be documented as part of the mavlink specification. */


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
        ss << "  target_network: " << +target_network << std::endl;
        ss << "  target_system: " << +target_system << std::endl;
        ss << "  target_component: " << +target_component << std::endl;
        ss << "  payload: [" << to_string(payload) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << target_network;                // offset: 0
        map << target_system;                 // offset: 1
        map << target_component;              // offset: 2
        map << payload;                       // offset: 3
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> target_network;                // offset: 0
        map >> target_system;                 // offset: 1
        map >> target_component;              // offset: 2
        map >> payload;                       // offset: 3
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
