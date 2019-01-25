// MESSAGE GPS_RTK support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief GPS_RTK message
 *
 * RTK GPS data. Gives information on the relative baseline calculation the GPS is reporting
 */
struct GPS_RTK : mavlink::Message {
    static constexpr msgid_t MSG_ID = 127;
    static constexpr size_t LENGTH = 35;
    static constexpr size_t MIN_LENGTH = 35;
    static constexpr uint8_t CRC_EXTRA = 25;
    static constexpr auto NAME = "GPS_RTK";


    uint32_t time_last_baseline_ms; /*< [ms] Time since boot of last baseline message received. */
    uint8_t rtk_receiver_id; /*<  Identification of connected RTK receiver. */
    uint16_t wn; /*<  GPS Week Number of last baseline */
    uint32_t tow; /*< [ms] GPS Time of Week of last baseline */
    uint8_t rtk_health; /*<  GPS-specific health report for RTK data. */
    uint8_t rtk_rate; /*< [Hz] Rate of baseline messages being received by GPS */
    uint8_t nsats; /*<  Current number of sats used for RTK calculation. */
    uint8_t baseline_coords_type; /*<  Coordinate system of baseline */
    int32_t baseline_a_mm; /*< [mm] Current baseline in ECEF x or NED north component. */
    int32_t baseline_b_mm; /*< [mm] Current baseline in ECEF y or NED east component. */
    int32_t baseline_c_mm; /*< [mm] Current baseline in ECEF z or NED down component. */
    uint32_t accuracy; /*<  Current estimate of baseline accuracy. */
    int32_t iar_num_hypotheses; /*<  Current number of integer ambiguity hypotheses. */


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
        ss << "  time_last_baseline_ms: " << time_last_baseline_ms << std::endl;
        ss << "  rtk_receiver_id: " << +rtk_receiver_id << std::endl;
        ss << "  wn: " << wn << std::endl;
        ss << "  tow: " << tow << std::endl;
        ss << "  rtk_health: " << +rtk_health << std::endl;
        ss << "  rtk_rate: " << +rtk_rate << std::endl;
        ss << "  nsats: " << +nsats << std::endl;
        ss << "  baseline_coords_type: " << +baseline_coords_type << std::endl;
        ss << "  baseline_a_mm: " << baseline_a_mm << std::endl;
        ss << "  baseline_b_mm: " << baseline_b_mm << std::endl;
        ss << "  baseline_c_mm: " << baseline_c_mm << std::endl;
        ss << "  accuracy: " << accuracy << std::endl;
        ss << "  iar_num_hypotheses: " << iar_num_hypotheses << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_last_baseline_ms;         // offset: 0
        map << tow;                           // offset: 4
        map << baseline_a_mm;                 // offset: 8
        map << baseline_b_mm;                 // offset: 12
        map << baseline_c_mm;                 // offset: 16
        map << accuracy;                      // offset: 20
        map << iar_num_hypotheses;            // offset: 24
        map << wn;                            // offset: 28
        map << rtk_receiver_id;               // offset: 30
        map << rtk_health;                    // offset: 31
        map << rtk_rate;                      // offset: 32
        map << nsats;                         // offset: 33
        map << baseline_coords_type;          // offset: 34
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_last_baseline_ms;         // offset: 0
        map >> tow;                           // offset: 4
        map >> baseline_a_mm;                 // offset: 8
        map >> baseline_b_mm;                 // offset: 12
        map >> baseline_c_mm;                 // offset: 16
        map >> accuracy;                      // offset: 20
        map >> iar_num_hypotheses;            // offset: 24
        map >> wn;                            // offset: 28
        map >> rtk_receiver_id;               // offset: 30
        map >> rtk_health;                    // offset: 31
        map >> rtk_rate;                      // offset: 32
        map >> nsats;                         // offset: 33
        map >> baseline_coords_type;          // offset: 34
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
