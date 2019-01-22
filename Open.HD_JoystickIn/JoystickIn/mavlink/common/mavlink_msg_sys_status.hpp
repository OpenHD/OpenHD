// MESSAGE SYS_STATUS support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief SYS_STATUS message
 *
 * The general system state. If the system is following the MAVLink standard, the system state is mainly defined by three orthogonal states/modes: The system mode, which is either LOCKED (motors shut down and locked), MANUAL (system under RC control), GUIDED (system with autonomous position control, position setpoint controlled manually) or AUTO (system guided by path/waypoint planner). The NAV_MODE defined the current flight state: LIFTOFF (often an open-loop maneuver), LANDING, WAYPOINTS or VECTOR. This represents the internal navigation state machine. The system status shows whether the system is currently active or not and if an emergency occurred. During the CRITICAL and EMERGENCY states the MAV is still considered to be active, but should start emergency procedures autonomously. After a failure occurred it should first move from active to critical to allow manual intervention and then move to emergency after a certain timeout.
 */
struct SYS_STATUS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 1;
    static constexpr size_t LENGTH = 31;
    static constexpr size_t MIN_LENGTH = 31;
    static constexpr uint8_t CRC_EXTRA = 124;
    static constexpr auto NAME = "SYS_STATUS";


    uint32_t onboard_control_sensors_present; /*<  Bitmap showing which onboard controllers and sensors are present. Value of 0: not present. Value of 1: present. */
    uint32_t onboard_control_sensors_enabled; /*<  Bitmap showing which onboard controllers and sensors are enabled:  Value of 0: not enabled. Value of 1: enabled. */
    uint32_t onboard_control_sensors_health; /*<  Bitmap showing which onboard controllers and sensors are operational or have an error:  Value of 0: not enabled. Value of 1: enabled. */
    uint16_t load; /*< [d%] Maximum usage in percent of the mainloop time. Values: [0-1000] - should always be below 1000 */
    uint16_t voltage_battery; /*< [mV] Battery voltage */
    int16_t current_battery; /*< [cA] Battery current, -1: autopilot does not measure the current */
    int8_t battery_remaining; /*< [%] Remaining battery energy, -1: autopilot estimate the remaining battery */
    uint16_t drop_rate_comm; /*< [c%] Communication drop rate, (UART, I2C, SPI, CAN), dropped packets on all links (packets that were corrupted on reception on the MAV) */
    uint16_t errors_comm; /*<  Communication errors (UART, I2C, SPI, CAN), dropped packets on all links (packets that were corrupted on reception on the MAV) */
    uint16_t errors_count1; /*<  Autopilot-specific errors */
    uint16_t errors_count2; /*<  Autopilot-specific errors */
    uint16_t errors_count3; /*<  Autopilot-specific errors */
    uint16_t errors_count4; /*<  Autopilot-specific errors */


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
        ss << "  onboard_control_sensors_present: " << onboard_control_sensors_present << std::endl;
        ss << "  onboard_control_sensors_enabled: " << onboard_control_sensors_enabled << std::endl;
        ss << "  onboard_control_sensors_health: " << onboard_control_sensors_health << std::endl;
        ss << "  load: " << load << std::endl;
        ss << "  voltage_battery: " << voltage_battery << std::endl;
        ss << "  current_battery: " << current_battery << std::endl;
        ss << "  battery_remaining: " << +battery_remaining << std::endl;
        ss << "  drop_rate_comm: " << drop_rate_comm << std::endl;
        ss << "  errors_comm: " << errors_comm << std::endl;
        ss << "  errors_count1: " << errors_count1 << std::endl;
        ss << "  errors_count2: " << errors_count2 << std::endl;
        ss << "  errors_count3: " << errors_count3 << std::endl;
        ss << "  errors_count4: " << errors_count4 << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << onboard_control_sensors_present; // offset: 0
        map << onboard_control_sensors_enabled; // offset: 4
        map << onboard_control_sensors_health; // offset: 8
        map << load;                          // offset: 12
        map << voltage_battery;               // offset: 14
        map << current_battery;               // offset: 16
        map << drop_rate_comm;                // offset: 18
        map << errors_comm;                   // offset: 20
        map << errors_count1;                 // offset: 22
        map << errors_count2;                 // offset: 24
        map << errors_count3;                 // offset: 26
        map << errors_count4;                 // offset: 28
        map << battery_remaining;             // offset: 30
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> onboard_control_sensors_present; // offset: 0
        map >> onboard_control_sensors_enabled; // offset: 4
        map >> onboard_control_sensors_health; // offset: 8
        map >> load;                          // offset: 12
        map >> voltage_battery;               // offset: 14
        map >> current_battery;               // offset: 16
        map >> drop_rate_comm;                // offset: 18
        map >> errors_comm;                   // offset: 20
        map >> errors_count1;                 // offset: 22
        map >> errors_count2;                 // offset: 24
        map >> errors_count3;                 // offset: 26
        map >> errors_count4;                 // offset: 28
        map >> battery_remaining;             // offset: 30
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
