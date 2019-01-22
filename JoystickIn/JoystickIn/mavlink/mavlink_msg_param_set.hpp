// MESSAGE PARAM_SET support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief PARAM_SET message
 *
 * Set a parameter value TEMPORARILY to RAM. It will be reset to default on system reboot. Send the ACTION MAV_ACTION_STORAGE_WRITE to PERMANENTLY write the RAM contents to EEPROM. IMPORTANT: The receiving component should acknowledge the new parameter value by sending a param_value message to all communication partners. This will also ensure that multiple GCS all have an up-to-date list of all parameters. If the sending GCS did not receive a PARAM_VALUE message within its timeout time, it should re-send the PARAM_SET message.
 */
struct PARAM_SET : mavlink::Message {
    static constexpr msgid_t MSG_ID = 23;
    static constexpr size_t LENGTH = 23;
    static constexpr size_t MIN_LENGTH = 23;
    static constexpr uint8_t CRC_EXTRA = 168;
    static constexpr auto NAME = "PARAM_SET";


    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
    std::array<char, 16> param_id; /*<  Onboard parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string */
    float param_value; /*<  Onboard parameter value */
    uint8_t param_type; /*<  Onboard parameter type. */


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
        ss << "  param_value: " << param_value << std::endl;
        ss << "  param_type: " << +param_type << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << param_value;                   // offset: 0
        map << target_system;                 // offset: 4
        map << target_component;              // offset: 5
        map << param_id;                      // offset: 6
        map << param_type;                    // offset: 22
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> param_value;                   // offset: 0
        map >> target_system;                 // offset: 4
        map >> target_component;              // offset: 5
        map >> param_id;                      // offset: 6
        map >> param_type;                    // offset: 22
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
