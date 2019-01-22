// MESSAGE SERIAL_CONTROL support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief SERIAL_CONTROL message
 *
 * Control a serial port. This can be used for raw access to an onboard serial peripheral such as a GPS or telemetry radio. It is designed to make it possible to update the devices firmware via MAVLink messages or change the devices settings. A message with zero bytes can be used to change just the baudrate.
 */
struct SERIAL_CONTROL : mavlink::Message {
    static constexpr msgid_t MSG_ID = 126;
    static constexpr size_t LENGTH = 79;
    static constexpr size_t MIN_LENGTH = 79;
    static constexpr uint8_t CRC_EXTRA = 220;
    static constexpr auto NAME = "SERIAL_CONTROL";


    uint8_t device; /*<  Serial control device type. */
    uint8_t flags; /*<  Bitmap of serial control flags. */
    uint16_t timeout; /*< [ms] Timeout for reply data */
    uint32_t baudrate; /*< [bits/s] Baudrate of transfer. Zero means no change. */
    uint8_t count; /*< [bytes] how many bytes in this transfer */
    std::array<uint8_t, 70> data; /*<  serial data */


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
        ss << "  device: " << +device << std::endl;
        ss << "  flags: " << +flags << std::endl;
        ss << "  timeout: " << timeout << std::endl;
        ss << "  baudrate: " << baudrate << std::endl;
        ss << "  count: " << +count << std::endl;
        ss << "  data: [" << to_string(data) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << baudrate;                      // offset: 0
        map << timeout;                       // offset: 4
        map << device;                        // offset: 6
        map << flags;                         // offset: 7
        map << count;                         // offset: 8
        map << data;                          // offset: 9
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> baudrate;                      // offset: 0
        map >> timeout;                       // offset: 4
        map >> device;                        // offset: 6
        map >> flags;                         // offset: 7
        map >> count;                         // offset: 8
        map >> data;                          // offset: 9
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
