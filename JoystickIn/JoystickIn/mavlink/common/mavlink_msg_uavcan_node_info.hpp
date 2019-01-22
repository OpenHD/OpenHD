// MESSAGE UAVCAN_NODE_INFO support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief UAVCAN_NODE_INFO message
 *
 * General information describing a particular UAVCAN node. Please refer to the definition of the UAVCAN service "uavcan.protocol.GetNodeInfo" for the background information. This message should be emitted by the system whenever a new node appears online, or an existing node reboots. Additionally, it can be emitted upon request from the other end of the MAVLink channel (see MAV_CMD_UAVCAN_GET_NODE_INFO). It is also not prohibited to emit this message unconditionally at a low frequency. The UAVCAN specification is available at http://uavcan.org.
 */
struct UAVCAN_NODE_INFO : mavlink::Message {
    static constexpr msgid_t MSG_ID = 311;
    static constexpr size_t LENGTH = 116;
    static constexpr size_t MIN_LENGTH = 116;
    static constexpr uint8_t CRC_EXTRA = 95;
    static constexpr auto NAME = "UAVCAN_NODE_INFO";


    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    uint32_t uptime_sec; /*< [s] Time since the start-up of the node. */
    std::array<char, 80> name; /*<  Node name string. For example, "sapog.px4.io". */
    uint8_t hw_version_major; /*<  Hardware major version number. */
    uint8_t hw_version_minor; /*<  Hardware minor version number. */
    std::array<uint8_t, 16> hw_unique_id; /*<  Hardware unique 128-bit ID. */
    uint8_t sw_version_major; /*<  Software major version number. */
    uint8_t sw_version_minor; /*<  Software minor version number. */
    uint32_t sw_vcs_commit; /*<  Version control system (VCS) revision identifier (e.g. git short commit hash). Zero if unknown. */


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
        ss << "  name: \"" << to_string(name) << "\"" << std::endl;
        ss << "  hw_version_major: " << +hw_version_major << std::endl;
        ss << "  hw_version_minor: " << +hw_version_minor << std::endl;
        ss << "  hw_unique_id: [" << to_string(hw_unique_id) << "]" << std::endl;
        ss << "  sw_version_major: " << +sw_version_major << std::endl;
        ss << "  sw_version_minor: " << +sw_version_minor << std::endl;
        ss << "  sw_vcs_commit: " << sw_vcs_commit << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << uptime_sec;                    // offset: 8
        map << sw_vcs_commit;                 // offset: 12
        map << name;                          // offset: 16
        map << hw_version_major;              // offset: 96
        map << hw_version_minor;              // offset: 97
        map << hw_unique_id;                  // offset: 98
        map << sw_version_major;              // offset: 114
        map << sw_version_minor;              // offset: 115
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> uptime_sec;                    // offset: 8
        map >> sw_vcs_commit;                 // offset: 12
        map >> name;                          // offset: 16
        map >> hw_version_major;              // offset: 96
        map >> hw_version_minor;              // offset: 97
        map >> hw_unique_id;                  // offset: 98
        map >> sw_version_major;              // offset: 114
        map >> sw_version_minor;              // offset: 115
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
