// MESSAGE COLLISION support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief COLLISION message
 *
 * Information about a potential collision
 */
struct COLLISION : mavlink::Message {
    static constexpr msgid_t MSG_ID = 247;
    static constexpr size_t LENGTH = 19;
    static constexpr size_t MIN_LENGTH = 19;
    static constexpr uint8_t CRC_EXTRA = 81;
    static constexpr auto NAME = "COLLISION";


    uint8_t src; /*<  Collision data source */
    uint32_t id; /*<  Unique identifier, domain based on src field */
    uint8_t action; /*<  Action that is being taken to avoid this collision */
    uint8_t threat_level; /*<  How concerned the aircraft is about this collision */
    float time_to_minimum_delta; /*< [s] Estimated time until collision occurs */
    float altitude_minimum_delta; /*< [m] Closest vertical distance between vehicle and object */
    float horizontal_minimum_delta; /*< [m] Closest horizontal distance between vehicle and object */


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
        ss << "  src: " << +src << std::endl;
        ss << "  id: " << id << std::endl;
        ss << "  action: " << +action << std::endl;
        ss << "  threat_level: " << +threat_level << std::endl;
        ss << "  time_to_minimum_delta: " << time_to_minimum_delta << std::endl;
        ss << "  altitude_minimum_delta: " << altitude_minimum_delta << std::endl;
        ss << "  horizontal_minimum_delta: " << horizontal_minimum_delta << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << id;                            // offset: 0
        map << time_to_minimum_delta;         // offset: 4
        map << altitude_minimum_delta;        // offset: 8
        map << horizontal_minimum_delta;      // offset: 12
        map << src;                           // offset: 16
        map << action;                        // offset: 17
        map << threat_level;                  // offset: 18
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> id;                            // offset: 0
        map >> time_to_minimum_delta;         // offset: 4
        map >> altitude_minimum_delta;        // offset: 8
        map >> horizontal_minimum_delta;      // offset: 12
        map >> src;                           // offset: 16
        map >> action;                        // offset: 17
        map >> threat_level;                  // offset: 18
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
