// MESSAGE COMMAND_LONG support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief COMMAND_LONG message
 *
 * Send a command with up to seven parameters to the MAV
 */
struct COMMAND_LONG : mavlink::Message {
    static constexpr msgid_t MSG_ID = 76;
    static constexpr size_t LENGTH = 33;
    static constexpr size_t MIN_LENGTH = 33;
    static constexpr uint8_t CRC_EXTRA = 152;
    static constexpr auto NAME = "COMMAND_LONG";


    uint8_t target_system; /*<  System which should execute the command */
    uint8_t target_component; /*<  Component which should execute the command, 0 for all components */
    uint16_t command; /*<  Command ID (of command to send). */
    uint8_t confirmation; /*<  0: First transmission of this command. 1-255: Confirmation transmissions (e.g. for kill command) */
    float param1; /*<  Parameter 1 (for the specific command). */
    float param2; /*<  Parameter 2 (for the specific command). */
    float param3; /*<  Parameter 3 (for the specific command). */
    float param4; /*<  Parameter 4 (for the specific command). */
    float param5; /*<  Parameter 5 (for the specific command). */
    float param6; /*<  Parameter 6 (for the specific command). */
    float param7; /*<  Parameter 7 (for the specific command). */


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
        ss << "  command: " << command << std::endl;
        ss << "  confirmation: " << +confirmation << std::endl;
        ss << "  param1: " << param1 << std::endl;
        ss << "  param2: " << param2 << std::endl;
        ss << "  param3: " << param3 << std::endl;
        ss << "  param4: " << param4 << std::endl;
        ss << "  param5: " << param5 << std::endl;
        ss << "  param6: " << param6 << std::endl;
        ss << "  param7: " << param7 << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << param1;                        // offset: 0
        map << param2;                        // offset: 4
        map << param3;                        // offset: 8
        map << param4;                        // offset: 12
        map << param5;                        // offset: 16
        map << param6;                        // offset: 20
        map << param7;                        // offset: 24
        map << command;                       // offset: 28
        map << target_system;                 // offset: 30
        map << target_component;              // offset: 31
        map << confirmation;                  // offset: 32
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> param1;                        // offset: 0
        map >> param2;                        // offset: 4
        map >> param3;                        // offset: 8
        map >> param4;                        // offset: 12
        map >> param5;                        // offset: 16
        map >> param6;                        // offset: 20
        map >> param7;                        // offset: 24
        map >> command;                       // offset: 28
        map >> target_system;                 // offset: 30
        map >> target_component;              // offset: 31
        map >> confirmation;                  // offset: 32
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
