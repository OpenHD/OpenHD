// MESSAGE CHANGE_OPERATOR_CONTROL support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief CHANGE_OPERATOR_CONTROL message
 *
 * Request to control this MAV
 */
struct CHANGE_OPERATOR_CONTROL : mavlink::Message {
    static constexpr msgid_t MSG_ID = 5;
    static constexpr size_t LENGTH = 28;
    static constexpr size_t MIN_LENGTH = 28;
    static constexpr uint8_t CRC_EXTRA = 217;
    static constexpr auto NAME = "CHANGE_OPERATOR_CONTROL";


    uint8_t target_system; /*<  System the GCS requests control for */
    uint8_t control_request; /*<  0: request control of this MAV, 1: Release control of this MAV */
    uint8_t version; /*< [rad] 0: key as plaintext, 1-255: future, different hashing/encryption variants. The GCS should in general use the safest mode possible initially and then gradually move down the encryption level if it gets a NACK message indicating an encryption mismatch. */
    std::array<char, 25> passkey; /*<  Password / Key, depending on version plaintext or encrypted. 25 or less characters, NULL terminated. The characters may involve A-Z, a-z, 0-9, and "!?,.-" */


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
        ss << "  control_request: " << +control_request << std::endl;
        ss << "  version: " << +version << std::endl;
        ss << "  passkey: \"" << to_string(passkey) << "\"" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << target_system;                 // offset: 0
        map << control_request;               // offset: 1
        map << version;                       // offset: 2
        map << passkey;                       // offset: 3
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> target_system;                 // offset: 0
        map >> control_request;               // offset: 1
        map >> version;                       // offset: 2
        map >> passkey;                       // offset: 3
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
