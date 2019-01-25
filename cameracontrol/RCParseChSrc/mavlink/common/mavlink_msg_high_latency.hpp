// MESSAGE HIGH_LATENCY support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief HIGH_LATENCY message
 *
 * Message appropriate for high latency connections like Iridium
 */
struct HIGH_LATENCY : mavlink::Message {
    static constexpr msgid_t MSG_ID = 234;
    static constexpr size_t LENGTH = 40;
    static constexpr size_t MIN_LENGTH = 40;
    static constexpr uint8_t CRC_EXTRA = 150;
    static constexpr auto NAME = "HIGH_LATENCY";


    uint8_t base_mode; /*<  Bitmap of enabled system modes. */
    uint32_t custom_mode; /*<  A bitfield for use for autopilot-specific flags. */
    uint8_t landed_state; /*<  The landed state. Is set to MAV_LANDED_STATE_UNDEFINED if landed state is unknown. */
    int16_t roll; /*< [cdeg] roll */
    int16_t pitch; /*< [cdeg] pitch */
    uint16_t heading; /*< [cdeg] heading */
    int8_t throttle; /*< [%] throttle (percentage) */
    int16_t heading_sp; /*< [cdeg] heading setpoint */
    int32_t latitude; /*< [degE7] Latitude */
    int32_t longitude; /*< [degE7] Longitude */
    int16_t altitude_amsl; /*< [m] Altitude above mean sea level */
    int16_t altitude_sp; /*< [m] Altitude setpoint relative to the home position */
    uint8_t airspeed; /*< [m/s] airspeed */
    uint8_t airspeed_sp; /*< [m/s] airspeed setpoint */
    uint8_t groundspeed; /*< [m/s] groundspeed */
    int8_t climb_rate; /*< [m/s] climb rate */
    uint8_t gps_nsat; /*<  Number of satellites visible. If unknown, set to 255 */
    uint8_t gps_fix_type; /*<  GPS Fix type. */
    uint8_t battery_remaining; /*< [%] Remaining battery (percentage) */
    int8_t temperature; /*< [degC] Autopilot temperature (degrees C) */
    int8_t temperature_air; /*< [degC] Air temperature (degrees C) from airspeed sensor */
    uint8_t failsafe; /*<  failsafe (each bit represents a failsafe where 0=ok, 1=failsafe active (bit0:RC, bit1:batt, bit2:GPS, bit3:GCS, bit4:fence) */
    uint8_t wp_num; /*<  current waypoint number */
    uint16_t wp_distance; /*< [m] distance to target */


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
        ss << "  base_mode: " << +base_mode << std::endl;
        ss << "  custom_mode: " << custom_mode << std::endl;
        ss << "  landed_state: " << +landed_state << std::endl;
        ss << "  roll: " << roll << std::endl;
        ss << "  pitch: " << pitch << std::endl;
        ss << "  heading: " << heading << std::endl;
        ss << "  throttle: " << +throttle << std::endl;
        ss << "  heading_sp: " << heading_sp << std::endl;
        ss << "  latitude: " << latitude << std::endl;
        ss << "  longitude: " << longitude << std::endl;
        ss << "  altitude_amsl: " << altitude_amsl << std::endl;
        ss << "  altitude_sp: " << altitude_sp << std::endl;
        ss << "  airspeed: " << +airspeed << std::endl;
        ss << "  airspeed_sp: " << +airspeed_sp << std::endl;
        ss << "  groundspeed: " << +groundspeed << std::endl;
        ss << "  climb_rate: " << +climb_rate << std::endl;
        ss << "  gps_nsat: " << +gps_nsat << std::endl;
        ss << "  gps_fix_type: " << +gps_fix_type << std::endl;
        ss << "  battery_remaining: " << +battery_remaining << std::endl;
        ss << "  temperature: " << +temperature << std::endl;
        ss << "  temperature_air: " << +temperature_air << std::endl;
        ss << "  failsafe: " << +failsafe << std::endl;
        ss << "  wp_num: " << +wp_num << std::endl;
        ss << "  wp_distance: " << wp_distance << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << custom_mode;                   // offset: 0
        map << latitude;                      // offset: 4
        map << longitude;                     // offset: 8
        map << roll;                          // offset: 12
        map << pitch;                         // offset: 14
        map << heading;                       // offset: 16
        map << heading_sp;                    // offset: 18
        map << altitude_amsl;                 // offset: 20
        map << altitude_sp;                   // offset: 22
        map << wp_distance;                   // offset: 24
        map << base_mode;                     // offset: 26
        map << landed_state;                  // offset: 27
        map << throttle;                      // offset: 28
        map << airspeed;                      // offset: 29
        map << airspeed_sp;                   // offset: 30
        map << groundspeed;                   // offset: 31
        map << climb_rate;                    // offset: 32
        map << gps_nsat;                      // offset: 33
        map << gps_fix_type;                  // offset: 34
        map << battery_remaining;             // offset: 35
        map << temperature;                   // offset: 36
        map << temperature_air;               // offset: 37
        map << failsafe;                      // offset: 38
        map << wp_num;                        // offset: 39
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> custom_mode;                   // offset: 0
        map >> latitude;                      // offset: 4
        map >> longitude;                     // offset: 8
        map >> roll;                          // offset: 12
        map >> pitch;                         // offset: 14
        map >> heading;                       // offset: 16
        map >> heading_sp;                    // offset: 18
        map >> altitude_amsl;                 // offset: 20
        map >> altitude_sp;                   // offset: 22
        map >> wp_distance;                   // offset: 24
        map >> base_mode;                     // offset: 26
        map >> landed_state;                  // offset: 27
        map >> throttle;                      // offset: 28
        map >> airspeed;                      // offset: 29
        map >> airspeed_sp;                   // offset: 30
        map >> groundspeed;                   // offset: 31
        map >> climb_rate;                    // offset: 32
        map >> gps_nsat;                      // offset: 33
        map >> gps_fix_type;                  // offset: 34
        map >> battery_remaining;             // offset: 35
        map >> temperature;                   // offset: 36
        map >> temperature_air;               // offset: 37
        map >> failsafe;                      // offset: 38
        map >> wp_num;                        // offset: 39
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
