// MESSAGE AIRSPEED_AUTOCAL support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief AIRSPEED_AUTOCAL message
 *
 * Airspeed auto-calibration.
 */
struct AIRSPEED_AUTOCAL : mavlink::Message {
    static constexpr msgid_t MSG_ID = 174;
    static constexpr size_t LENGTH = 48;
    static constexpr size_t MIN_LENGTH = 48;
    static constexpr uint8_t CRC_EXTRA = 167;
    static constexpr auto NAME = "AIRSPEED_AUTOCAL";


    float vx; /*< [m/s] GPS velocity north. */
    float vy; /*< [m/s] GPS velocity east. */
    float vz; /*< [m/s] GPS velocity down. */
    float diff_pressure; /*< [Pa] Differential pressure. */
    float EAS2TAS; /*<  Estimated to true airspeed ratio. */
    float ratio; /*<  Airspeed ratio. */
    float state_x; /*<  EKF state x. */
    float state_y; /*<  EKF state y. */
    float state_z; /*<  EKF state z. */
    float Pax; /*<  EKF Pax. */
    float Pby; /*<  EKF Pby. */
    float Pcz; /*<  EKF Pcz. */


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
        ss << "  vx: " << vx << std::endl;
        ss << "  vy: " << vy << std::endl;
        ss << "  vz: " << vz << std::endl;
        ss << "  diff_pressure: " << diff_pressure << std::endl;
        ss << "  EAS2TAS: " << EAS2TAS << std::endl;
        ss << "  ratio: " << ratio << std::endl;
        ss << "  state_x: " << state_x << std::endl;
        ss << "  state_y: " << state_y << std::endl;
        ss << "  state_z: " << state_z << std::endl;
        ss << "  Pax: " << Pax << std::endl;
        ss << "  Pby: " << Pby << std::endl;
        ss << "  Pcz: " << Pcz << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << vx;                            // offset: 0
        map << vy;                            // offset: 4
        map << vz;                            // offset: 8
        map << diff_pressure;                 // offset: 12
        map << EAS2TAS;                       // offset: 16
        map << ratio;                         // offset: 20
        map << state_x;                       // offset: 24
        map << state_y;                       // offset: 28
        map << state_z;                       // offset: 32
        map << Pax;                           // offset: 36
        map << Pby;                           // offset: 40
        map << Pcz;                           // offset: 44
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> vx;                            // offset: 0
        map >> vy;                            // offset: 4
        map >> vz;                            // offset: 8
        map >> diff_pressure;                 // offset: 12
        map >> EAS2TAS;                       // offset: 16
        map >> ratio;                         // offset: 20
        map >> state_x;                       // offset: 24
        map >> state_y;                       // offset: 28
        map >> state_z;                       // offset: 32
        map >> Pax;                           // offset: 36
        map >> Pby;                           // offset: 40
        map >> Pcz;                           // offset: 44
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
