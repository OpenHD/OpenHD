// MESSAGE UAVCAN_NODE_STATUS support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief UAVCAN_NODE_STATUS message
 *
 * General status information of an UAVCAN node. Please refer to the definition of the UAVCAN message "uavcan.protocol.NodeStatus" for the background information. The UAVCAN specification is available at http://uavcan.org.
 */
struct UAVCAN_NODE_STATUS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 310;
    static constexpr size_t LENGTH = 17;
    static constexpr size_t MIN_LENGTH = 17;
    static constexpr uint8_t CRC_EXTRA = 28;
    static constexpr auto NAME = "UAVCAN_NODE_STATUS";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    uint32_t uptime_sec; /*< [s] Time since the start-up of the node. */
    uint8_t health; /*<  Generalized node health status. */
    uint8_t mode; /*<  Generalized operating mode. */
    uint8_t sub_mode; /*<  Not used currently. */
    uint16_t vendor_specific_status_code; /*<  Vendor-specific status information. */


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
        ss << "  uptime_sec: " << uptime_sec << std::endl;
        ss << "  health: " << +health << std::endl;
        ss << "  mode: " << +mode << std::endl;
        ss << "  sub_mode: " << +sub_mode << std::endl;
        ss << "  vendor_specific_status_code: " << vendor_specific_status_code << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << uptime_sec;                    // offset: 8
        map << vendor_specific_status_code;   // offset: 12
        map << health;                        // offset: 14
        map << mode;                          // offset: 15
        map << sub_mode;                      // offset: 16
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> uptime_sec;                    // offset: 8
        map >> vendor_specific_status_code;   // offset: 12
        map >> health;                        // offset: 14
        map >> mode;                          // offset: 15
        map >> sub_mode;                      // offset: 16
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
