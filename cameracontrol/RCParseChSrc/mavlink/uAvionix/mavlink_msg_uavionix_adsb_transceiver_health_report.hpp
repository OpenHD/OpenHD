// MESSAGE UAVIONIX_ADSB_TRANSCEIVER_HEALTH_REPORT support class

#pragma once

namespace mavlink {
namespace uAvionix {
namespace msg {

/**
 * @brief UAVIONIX_ADSB_TRANSCEIVER_HEALTH_REPORT message
 *
 * Transceiver heartbeat with health report (updated every 10s)
 */
struct UAVIONIX_ADSB_TRANSCEIVER_HEALTH_REPORT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 10003;
    static constexpr size_t LENGTH = 1;
    static constexpr size_t MIN_LENGTH = 1;
    static constexpr uint8_t CRC_EXTRA = 4;
    static constexpr auto NAME = "UAVIONIX_ADSB_TRANSCEIVER_HEALTH_REPORT";


    uint8_t rfHealth; /*<  ADS-B transponder messages */


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
        ss << "  rfHealth: " << +rfHealth << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << rfHealth;                      // offset: 0
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> rfHealth;                      // offset: 0
    }
};

} // namespace msg
} // namespace uAvionix
} // namespace mavlink
