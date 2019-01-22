// MESSAGE AUTOPILOT_VERSION support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief AUTOPILOT_VERSION message
 *
 * Version and capability of autopilot software
 */
struct AUTOPILOT_VERSION : mavlink::Message {
    static constexpr msgid_t MSG_ID = 148;
    static constexpr size_t LENGTH = 78;
    static constexpr size_t MIN_LENGTH = 60;
    static constexpr uint8_t CRC_EXTRA = 178;
    static constexpr auto NAME = "AUTOPILOT_VERSION";


    uint64_t capabilities; /*<  Bitmap of capabilities */
    uint32_t flight_sw_version; /*<  Firmware version number */
    uint32_t middleware_sw_version; /*<  Middleware version number */
    uint32_t os_sw_version; /*<  Operating system version number */
    uint32_t board_version; /*<  HW / board version (last 8 bytes should be silicon ID, if any) */
    std::array<uint8_t, 8> flight_custom_version; /*<  Custom version field, commonly the first 8 bytes of the git hash. This is not an unique identifier, but should allow to identify the commit using the main version number even for very large code bases. */
    std::array<uint8_t, 8> middleware_custom_version; /*<  Custom version field, commonly the first 8 bytes of the git hash. This is not an unique identifier, but should allow to identify the commit using the main version number even for very large code bases. */
    std::array<uint8_t, 8> os_custom_version; /*<  Custom version field, commonly the first 8 bytes of the git hash. This is not an unique identifier, but should allow to identify the commit using the main version number even for very large code bases. */
    uint16_t vendor_id; /*<  ID of the board vendor */
    uint16_t product_id; /*<  ID of the product */
    uint64_t uid; /*<  UID if provided by hardware (see uid2) */
    std::array<uint8_t, 18> uid2; /*<  UID if provided by hardware (supersedes the uid field. If this is non-zero, use this field, otherwise use uid) */


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
        ss << "  capabilities: " << capabilities << std::endl;
        ss << "  flight_sw_version: " << flight_sw_version << std::endl;
        ss << "  middleware_sw_version: " << middleware_sw_version << std::endl;
        ss << "  os_sw_version: " << os_sw_version << std::endl;
        ss << "  board_version: " << board_version << std::endl;
        ss << "  flight_custom_version: [" << to_string(flight_custom_version) << "]" << std::endl;
        ss << "  middleware_custom_version: [" << to_string(middleware_custom_version) << "]" << std::endl;
        ss << "  os_custom_version: [" << to_string(os_custom_version) << "]" << std::endl;
        ss << "  vendor_id: " << vendor_id << std::endl;
        ss << "  product_id: " << product_id << std::endl;
        ss << "  uid: " << uid << std::endl;
        ss << "  uid2: [" << to_string(uid2) << "]" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << capabilities;                  // offset: 0
        map << uid;                           // offset: 8
        map << flight_sw_version;             // offset: 16
        map << middleware_sw_version;         // offset: 20
        map << os_sw_version;                 // offset: 24
        map << board_version;                 // offset: 28
        map << vendor_id;                     // offset: 32
        map << product_id;                    // offset: 34
        map << flight_custom_version;         // offset: 36
        map << middleware_custom_version;     // offset: 44
        map << os_custom_version;             // offset: 52
        map << uid2;                          // offset: 60
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> capabilities;                  // offset: 0
        map >> uid;                           // offset: 8
        map >> flight_sw_version;             // offset: 16
        map >> middleware_sw_version;         // offset: 20
        map >> os_sw_version;                 // offset: 24
        map >> board_version;                 // offset: 28
        map >> vendor_id;                     // offset: 32
        map >> product_id;                    // offset: 34
        map >> flight_custom_version;         // offset: 36
        map >> middleware_custom_version;     // offset: 44
        map >> os_custom_version;             // offset: 52
        map >> uid2;                          // offset: 60
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
