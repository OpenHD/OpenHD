// MESSAGE COMPASSMOT_STATUS support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief COMPASSMOT_STATUS message
 *
 * Status of compassmot calibration.
 */
struct COMPASSMOT_STATUS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 177;
    static constexpr size_t LENGTH = 20;
    static constexpr size_t MIN_LENGTH = 20;
    static constexpr uint8_t CRC_EXTRA = 240;
    static constexpr auto NAME = "COMPASSMOT_STATUS";


    uint16_t throttle; /*< [d%] Throttle. */
    float current; /*< [A] Current. */
    uint16_t interference; /*< [%] Interference. */
    float CompensationX; /*<  Motor Compensation X. */
    float CompensationY; /*<  Motor Compensation Y. */
    float CompensationZ; /*<  Motor Compensation Z. */


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
        ss << "  throttle: " << throttle << std::endl;
        ss << "  current: " << current << std::endl;
        ss << "  interference: " << interference << std::endl;
        ss << "  CompensationX: " << CompensationX << std::endl;
        ss << "  CompensationY: " << CompensationY << std::endl;
        ss << "  CompensationZ: " << CompensationZ << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << current;                       // offset: 0
        map << CompensationX;                 // offset: 4
        map << CompensationY;                 // offset: 8
        map << CompensationZ;                 // offset: 12
        map << throttle;                      // offset: 16
        map << interference;                  // offset: 18
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> current;                       // offset: 0
        map >> CompensationX;                 // offset: 4
        map >> CompensationY;                 // offset: 8
        map >> CompensationZ;                 // offset: 12
        map >> throttle;                      // offset: 16
        map >> interference;                  // offset: 18
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
