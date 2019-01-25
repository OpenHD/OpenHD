// MESSAGE PARAM_REQUEST_READ support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief PARAM_REQUEST_READ message
 *
 * Request to read the onboard parameter with the param_id string id. Onboard parameters are stored as key[const char*] -> value[float]. This allows to send a parameter to any other component (such as the GCS) without the need of previous knowledge of possible parameter names. Thus the same GCS can store different parameters for different autopilots. See also https://mavlink.io/en/protocol/parameter.html for a full documentation of QGroundControl and IMU code.
 */
struct PARAM_REQUEST_READ : mavlink::Message {
    static constexpr msgid_t MSG_ID = 20;
    static constexpr size_t LENGTH = 20;
    static constexpr size_t MIN_LENGTH = 20;
    static constexpr uint8_t CRC_EXTRA = 214;
    static constexpr auto NAME = "PARAM_REQUEST_READ";


    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
    std::array<char, 16> param_id; /*<  Onboard parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string */
    int16_t param_index; /*<  Parameter index. Send -1 to use the param ID field as identifier (else the param id will be ignored) */


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
        ss << "  param_id: \"" << to_string(param_id) << "\"" << std::endl;
        ss << "  param_index: " << param_index << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << param_index;                   // offset: 0
        map << target_system;                 // offset: 2
        map << target_component;              // offset: 3
        map << param_id;                      // offset: 4
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> param_index;                   // offset: 0
        map >> target_system;                 // offset: 2
        map >> target_component;              // offset: 3
        map >> param_id;                      // offset: 4
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
