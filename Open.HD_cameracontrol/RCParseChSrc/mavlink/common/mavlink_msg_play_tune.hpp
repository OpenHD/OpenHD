// MESSAGE PLAY_TUNE support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief PLAY_TUNE message
 *
 * Control vehicle tone generation (buzzer)
 */
struct PLAY_TUNE : mavlink::Message {
    static constexpr msgid_t MSG_ID = 258;
    static constexpr size_t LENGTH = 232;
    static constexpr size_t MIN_LENGTH = 32;
    static constexpr uint8_t CRC_EXTRA = 187;
    static constexpr auto NAME = "PLAY_TUNE";


    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
    std::array<char, 30> tune; /*<  tune in board specific format */
    std::array<char, 200> tune2; /*<  tune extension (appended to tune) */


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
        ss << "  tune: \"" << to_string(tune) << "\"" << std::endl;
        ss << "  tune2: \"" << to_string(tune2) << "\"" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << target_system;                 // offset: 0
        map << target_component;              // offset: 1
        map << tune;                          // offset: 2
        map << tune2;                         // offset: 32
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> target_system;                 // offset: 0
        map >> target_component;              // offset: 1
        map >> tune;                          // offset: 2
        map >> tune2;                         // offset: 32
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
