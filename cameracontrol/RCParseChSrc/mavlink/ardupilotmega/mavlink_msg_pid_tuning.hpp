// MESSAGE PID_TUNING support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief PID_TUNING message
 *
 * PID tuning information.
 */
struct PID_TUNING : mavlink::Message {
    static constexpr msgid_t MSG_ID = 194;
    static constexpr size_t LENGTH = 25;
    static constexpr size_t MIN_LENGTH = 25;
    static constexpr uint8_t CRC_EXTRA = 98;
    static constexpr auto NAME = "PID_TUNING";


    uint8_t axis; /*<  Axis. */
    float desired; /*< [deg/s] Desired rate. */
    float achieved; /*< [deg/s] Achieved rate. */
    float FF; /*<  FF component. */
    float P; /*<  P component. */
    float I; /*<  I component. */
    float D; /*<  D component. */


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
        ss << "  axis: " << +axis << std::endl;
        ss << "  desired: " << desired << std::endl;
        ss << "  achieved: " << achieved << std::endl;
        ss << "  FF: " << FF << std::endl;
        ss << "  P: " << P << std::endl;
        ss << "  I: " << I << std::endl;
        ss << "  D: " << D << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << desired;                       // offset: 0
        map << achieved;                      // offset: 4
        map << FF;                            // offset: 8
        map << P;                             // offset: 12
        map << I;                             // offset: 16
        map << D;                             // offset: 20
        map << axis;                          // offset: 24
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> desired;                       // offset: 0
        map >> achieved;                      // offset: 4
        map >> FF;                            // offset: 8
        map >> P;                             // offset: 12
        map >> I;                             // offset: 16
        map >> D;                             // offset: 20
        map >> axis;                          // offset: 24
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
