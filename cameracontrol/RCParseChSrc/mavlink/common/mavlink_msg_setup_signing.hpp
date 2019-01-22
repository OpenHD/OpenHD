// MESSAGE SETUP_SIGNING support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief SETUP_SIGNING message
 *
 * Setup a MAVLink2 signing key. If called with secret_key of all zero and zero initial_timestamp will disable signing
 */
struct SETUP_SIGNING : mavlink::Message {
    static constexpr msgid_t MSG_ID = 256;
    static constexpr size_t LENGTH = 42;
    static constexpr size_t MIN_LENGTH = 42;
    static constexpr uint8_t CRC_EXTRA = 71;
    static constexpr auto NAME = "SETUP_SIGNING";


    uint8_t target_system; /*<  system id of the target */
    uint8_t target_component; /*<  component ID of the target */
    std::array<uint8_t, 32> secret_key; /*<  signing key */
    uint64_t initial_timestamp; /*<  initial timestamp */


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
        ss << "  secret_key: [" << to_string(secret_key) << "]" << std::endl;
        ss << "  initial_timestamp: " << initial_timestamp << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << initial_timestamp;             // offset: 0
        map << target_system;                 // offset: 8
        map << target_component;              // offset: 9
        map << secret_key;                    // offset: 10
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> initial_timestamp;             // offset: 0
        map >> target_system;                 // offset: 8
        map >> target_component;              // offset: 9
        map >> secret_key;                    // offset: 10
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
