// MESSAGE PARAM_MAP_RC support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief PARAM_MAP_RC message
 *
 * Bind a RC channel to a parameter. The parameter should change according to the RC channel value.
 */
struct PARAM_MAP_RC : mavlink::Message {
    static constexpr msgid_t MSG_ID = 50;
    static constexpr size_t LENGTH = 37;
    static constexpr size_t MIN_LENGTH = 37;
    static constexpr uint8_t CRC_EXTRA = 78;
    static constexpr auto NAME = "PARAM_MAP_RC";


    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
    std::array<char, 16> param_id; /*<  Onboard parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string */
    int16_t param_index; /*<  Parameter index. Send -1 to use the param ID field as identifier (else the param id will be ignored), send -2 to disable any existing map for this rc_channel_index. */
    uint8_t parameter_rc_channel_index; /*<  Index of parameter RC channel. Not equal to the RC channel id. Typically corresponds to a potentiometer-knob on the RC. */
    float param_value0; /*<  Initial parameter value */
    float scale; /*<  Scale, maps the RC range [-1, 1] to a parameter value */
    float param_value_min; /*<  Minimum param value. The protocol does not define if this overwrites an onboard minimum value. (Depends on implementation) */
    float param_value_max; /*<  Maximum param value. The protocol does not define if this overwrites an onboard maximum value. (Depends on implementation) */


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
        ss << "  parameter_rc_channel_index: " << +parameter_rc_channel_index << std::endl;
        ss << "  param_value0: " << param_value0 << std::endl;
        ss << "  scale: " << scale << std::endl;
        ss << "  param_value_min: " << param_value_min << std::endl;
        ss << "  param_value_max: " << param_value_max << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << param_value0;                  // offset: 0
        map << scale;                         // offset: 4
        map << param_value_min;               // offset: 8
        map << param_value_max;               // offset: 12
        map << param_index;                   // offset: 16
        map << target_system;                 // offset: 18
        map << target_component;              // offset: 19
        map << param_id;                      // offset: 20
        map << parameter_rc_channel_index;    // offset: 36
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> param_value0;                  // offset: 0
        map >> scale;                         // offset: 4
        map >> param_value_min;               // offset: 8
        map >> param_value_max;               // offset: 12
        map >> param_index;                   // offset: 16
        map >> target_system;                 // offset: 18
        map >> target_component;              // offset: 19
        map >> param_id;                      // offset: 20
        map >> parameter_rc_channel_index;    // offset: 36
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
