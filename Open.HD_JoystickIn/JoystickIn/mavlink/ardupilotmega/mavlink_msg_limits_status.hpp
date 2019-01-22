// MESSAGE LIMITS_STATUS support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief LIMITS_STATUS message
 *
 * Status of AP_Limits. Sent in extended status stream when AP_Limits is enabled.
 */
struct LIMITS_STATUS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 167;
    static constexpr size_t LENGTH = 22;
    static constexpr size_t MIN_LENGTH = 22;
    static constexpr uint8_t CRC_EXTRA = 144;
    static constexpr auto NAME = "LIMITS_STATUS";


    uint8_t limits_state; /*<  State of AP_Limits. */
    uint32_t last_trigger; /*< [ms] Time (since boot) of last breach. */
    uint32_t last_action; /*< [ms] Time (since boot) of last recovery action. */
    uint32_t last_recovery; /*< [ms] Time (since boot) of last successful recovery. */
    uint32_t last_clear; /*< [ms] Time (since boot) of last all-clear. */
    uint16_t breach_count; /*<  Number of fence breaches. */
    uint8_t mods_enabled; /*<  AP_Limit_Module bitfield of enabled modules. */
    uint8_t mods_required; /*<  AP_Limit_Module bitfield of required modules. */
    uint8_t mods_triggered; /*<  AP_Limit_Module bitfield of triggered modules. */


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
        ss << "  limits_state: " << +limits_state << std::endl;
        ss << "  last_trigger: " << last_trigger << std::endl;
        ss << "  last_action: " << last_action << std::endl;
        ss << "  last_recovery: " << last_recovery << std::endl;
        ss << "  last_clear: " << last_clear << std::endl;
        ss << "  breach_count: " << breach_count << std::endl;
        ss << "  mods_enabled: " << +mods_enabled << std::endl;
        ss << "  mods_required: " << +mods_required << std::endl;
        ss << "  mods_triggered: " << +mods_triggered << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << last_trigger;                  // offset: 0
        map << last_action;                   // offset: 4
        map << last_recovery;                 // offset: 8
        map << last_clear;                    // offset: 12
        map << breach_count;                  // offset: 16
        map << limits_state;                  // offset: 18
        map << mods_enabled;                  // offset: 19
        map << mods_required;                 // offset: 20
        map << mods_triggered;                // offset: 21
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> last_trigger;                  // offset: 0
        map >> last_action;                   // offset: 4
        map >> last_recovery;                 // offset: 8
        map >> last_clear;                    // offset: 12
        map >> breach_count;                  // offset: 16
        map >> limits_state;                  // offset: 18
        map >> mods_enabled;                  // offset: 19
        map >> mods_required;                 // offset: 20
        map >> mods_triggered;                // offset: 21
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
