// MESSAGE V2_EXTENSION support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief V2_EXTENSION message
 *
 * Message implementing parts of the V2 payload specs in V1 frames for transitional support.
 */
struct V2_EXTENSION : mavlink::Message {
    static constexpr msgid_t MSG_ID = 248;
    static constexpr size_t LENGTH = 254;
    static constexpr size_t MIN_LENGTH = 254;
    static constexpr uint8_t CRC_EXTRA = 8;
    static constexpr auto NAME = "V2_EXTENSION";


    uint8_t target_network; /*<  Network ID (0 for broadcast) */
    uint8_t target_system; /*<  System ID (0 for broadcast) */
    uint8_t target_component; /*<  Component ID (0 for broadcast) */
    uint16_t message_type; /*<  A code that identifies the software component that understands this message (analogous to USB device classes or mime type strings).  If this code is less than 32768, it is considered a 'registered' protocol extension and the corresponding entry should be added to https://github.com/mavlink/mavlink/extension-message-ids.xml.  Software creators can register blocks of message IDs as needed (useful for GCS specific metadata, etc...). Message_types greater than 32767 are considered local experiments and should not be checked in to any widely distributed codebase. */
    std::array<uint8_t, 249> payload; /*<  Variable length payload. The length is defined by the remaining message length when subtracting the header and other fields.  The entire content of this block is opaque unless you understand any the encoding message_type.  The particular encoding used can be extension specific and might not always be documented as part of the mavlink specification. */


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
        ss << "  message_type: " << message_type << std::endl;
        ss << "  payload: [" << to_string(payload) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << message_type;                  // offset: 0
        map << target_network;                // offset: 2
        map << target_system;                 // offset: 3
        map << target_component;              // offset: 4
        map << payload;                       // offset: 5
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> message_type;                  // offset: 0
        map >> target_network;                // offset: 2
        map >> target_system;                 // offset: 3
        map >> target_component;              // offset: 4
        map >> payload;                       // offset: 5
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
