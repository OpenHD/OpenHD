// MESSAGE DATA_TRANSMISSION_HANDSHAKE support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief DATA_TRANSMISSION_HANDSHAKE message
 *
 * 
 */
struct DATA_TRANSMISSION_HANDSHAKE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 130;
    static constexpr size_t LENGTH = 13;
    static constexpr size_t MIN_LENGTH = 13;
    static constexpr uint8_t CRC_EXTRA = 29;
    static constexpr auto NAME = "DATA_TRANSMISSION_HANDSHAKE";


    uint8_t type; /*<  Type of requested/acknowledged data. */
    uint32_t size; /*< [bytes] total data size (set on ACK only). */
    uint16_t width; /*<  Width of a matrix or image. */
    uint16_t height; /*<  Height of a matrix or image. */
    uint16_t packets; /*<  Number of packets being sent (set on ACK only). */
    uint8_t payload; /*< [bytes] Payload size per packet (normally 253 byte, see DATA field size in message ENCAPSULATED_DATA) (set on ACK only). */
    uint8_t jpg_quality; /*< [%] JPEG quality. Values: [1-100]. */


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
        ss << "  size: " << size << std::endl;
        ss << "  width: " << width << std::endl;
        ss << "  height: " << height << std::endl;
        ss << "  packets: " << packets << std::endl;
        ss << "  payload: " << +payload << std::endl;
        ss << "  jpg_quality: " << +jpg_quality << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << size;                          // offset: 0
        map << width;                         // offset: 4
        map << height;                        // offset: 6
        map << packets;                       // offset: 8
        map << type;                          // offset: 10
        map << payload;                       // offset: 11
        map << jpg_quality;                   // offset: 12
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> size;                          // offset: 0
        map >> width;                         // offset: 4
        map >> height;                        // offset: 6
        map >> packets;                       // offset: 8
        map >> type;                          // offset: 10
        map >> payload;                       // offset: 11
        map >> jpg_quality;                   // offset: 12
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
