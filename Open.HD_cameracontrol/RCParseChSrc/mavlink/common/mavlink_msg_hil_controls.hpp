// MESSAGE HIL_CONTROLS support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief HIL_CONTROLS message
 *
 * Sent from autopilot to simulation. Hardware in the loop control outputs
 */
struct HIL_CONTROLS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 91;
    static constexpr size_t LENGTH = 42;
    static constexpr size_t MIN_LENGTH = 42;
    static constexpr uint8_t CRC_EXTRA = 63;
    static constexpr auto NAME = "HIL_CONTROLS";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    float roll_ailerons; /*<  Control output -1 .. 1 */
    float pitch_elevator; /*<  Control output -1 .. 1 */
    float yaw_rudder; /*<  Control output -1 .. 1 */
    float throttle; /*<  Throttle 0 .. 1 */
    float aux1; /*<  Aux 1, -1 .. 1 */
    float aux2; /*<  Aux 2, -1 .. 1 */
    float aux3; /*<  Aux 3, -1 .. 1 */
    float aux4; /*<  Aux 4, -1 .. 1 */
    uint8_t mode; /*<  System mode. */
    uint8_t nav_mode; /*<  Navigation mode (MAV_NAV_MODE) */


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
        ss << "  time_usec: " << time_usec << std::endl;
        ss << "  roll_ailerons: " << roll_ailerons << std::endl;
        ss << "  pitch_elevator: " << pitch_elevator << std::endl;
        ss << "  yaw_rudder: " << yaw_rudder << std::endl;
        ss << "  throttle: " << throttle << std::endl;
        ss << "  aux1: " << aux1 << std::endl;
        ss << "  aux2: " << aux2 << std::endl;
        ss << "  aux3: " << aux3 << std::endl;
        ss << "  aux4: " << aux4 << std::endl;
        ss << "  mode: " << +mode << std::endl;
        ss << "  nav_mode: " << +nav_mode << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << roll_ailerons;                 // offset: 8
        map << pitch_elevator;                // offset: 12
        map << yaw_rudder;                    // offset: 16
        map << throttle;                      // offset: 20
        map << aux1;                          // offset: 24
        map << aux2;                          // offset: 28
        map << aux3;                          // offset: 32
        map << aux4;                          // offset: 36
        map << mode;                          // offset: 40
        map << nav_mode;                      // offset: 41
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> roll_ailerons;                 // offset: 8
        map >> pitch_elevator;                // offset: 12
        map >> yaw_rudder;                    // offset: 16
        map >> throttle;                      // offset: 20
        map >> aux1;                          // offset: 24
        map >> aux2;                          // offset: 28
        map >> aux3;                          // offset: 32
        map >> aux4;                          // offset: 36
        map >> mode;                          // offset: 40
        map >> nav_mode;                      // offset: 41
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
