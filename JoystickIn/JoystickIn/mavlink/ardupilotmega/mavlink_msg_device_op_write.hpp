// MESSAGE DEVICE_OP_WRITE support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief DEVICE_OP_WRITE message
 *
 * Write registers for a device.
 */
struct DEVICE_OP_WRITE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 11002;
    static constexpr size_t LENGTH = 179;
    static constexpr size_t MIN_LENGTH = 179;
    static constexpr uint8_t CRC_EXTRA = 234;
    static constexpr auto NAME = "DEVICE_OP_WRITE";


    uint8_t target_system; /*<  System ID. */
    uint8_t target_component; /*<  Component ID. */
    uint32_t request_id; /*<  Request ID - copied to reply. */
    uint8_t bustype; /*<  The bus type. */
    uint8_t bus; /*<  Bus number. */
    uint8_t address; /*<  Bus address. */
    std::array<char, 40> busname; /*<  Name of device on bus (for SPI). */
    uint8_t regstart; /*<  First register to write. */
    uint8_t count; /*<  Count of registers to write. */
    std::array<uint8_t, 128> data; /*<  Write data. */


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
        ss << "  target_system: " << +target_system << std::endl;
        ss << "  target_component: " << +target_component << std::endl;
        ss << "  request_id: " << request_id << std::endl;
        ss << "  bustype: " << +bustype << std::endl;
        ss << "  bus: " << +bus << std::endl;
        ss << "  address: " << +address << std::endl;
        ss << "  busname: \"" << to_string(busname) << "\"" << std::endl;
        ss << "  regstart: " << +regstart << std::endl;
        ss << "  count: " << +count << std::endl;
        ss << "  data: [" << to_string(data) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << request_id;                    // offset: 0
        map << target_system;                 // offset: 4
        map << target_component;              // offset: 5
        map << bustype;                       // offset: 6
        map << bus;                           // offset: 7
        map << address;                       // offset: 8
        map << busname;                       // offset: 9
        map << regstart;                      // offset: 49
        map << count;                         // offset: 50
        map << data;                          // offset: 51
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> request_id;                    // offset: 0
        map >> target_system;                 // offset: 4
        map >> target_component;              // offset: 5
        map >> bustype;                       // offset: 6
        map >> bus;                           // offset: 7
        map >> address;                       // offset: 8
        map >> busname;                       // offset: 9
        map >> regstart;                      // offset: 49
        map >> count;                         // offset: 50
        map >> data;                          // offset: 51
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
