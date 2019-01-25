// MESSAGE RESOURCE_REQUEST support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief RESOURCE_REQUEST message
 *
 * The autopilot is requesting a resource (file, binary, other type of data)
 */
struct RESOURCE_REQUEST : mavlink::Message {
    static constexpr msgid_t MSG_ID = 142;
    static constexpr size_t LENGTH = 243;
    static constexpr size_t MIN_LENGTH = 243;
    static constexpr uint8_t CRC_EXTRA = 72;
    static constexpr auto NAME = "RESOURCE_REQUEST";


    uint8_t request_id; /*<  Request ID. This ID should be re-used when sending back URI contents */
    uint8_t uri_type; /*<  The type of requested URI. 0 = a file via URL. 1 = a UAVCAN binary */
    std::array<uint8_t, 120> uri; /*<  The requested unique resource identifier (URI). It is not necessarily a straight domain name (depends on the URI type enum) */
    uint8_t transfer_type; /*<  The way the autopilot wants to receive the URI. 0 = MAVLink FTP. 1 = binary stream. */
    std::array<uint8_t, 120> storage; /*<  The storage path the autopilot wants the URI to be stored in. Will only be valid if the transfer_type has a storage associated (e.g. MAVLink FTP). */


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
        ss << "  request_id: " << +request_id << std::endl;
        ss << "  uri_type: " << +uri_type << std::endl;
        ss << "  uri: [" << to_string(uri) << "]" << std::endl;
        ss << "  transfer_type: " << +transfer_type << std::endl;
        ss << "  storage: [" << to_string(storage) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << request_id;                    // offset: 0
        map << uri_type;                      // offset: 1
        map << uri;                           // offset: 2
        map << transfer_type;                 // offset: 122
        map << storage;                       // offset: 123
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> request_id;                    // offset: 0
        map >> uri_type;                      // offset: 1
        map >> uri;                           // offset: 2
        map >> transfer_type;                 // offset: 122
        map >> storage;                       // offset: 123
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
