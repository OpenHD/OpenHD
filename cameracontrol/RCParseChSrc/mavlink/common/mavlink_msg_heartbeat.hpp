// MESSAGE HEARTBEAT support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief HEARTBEAT message
 *
 * The heartbeat message shows that a system is present and responding. The type of the MAV and Autopilot hardware allow the receiving system to treat further messages from this system appropriate (e.g. by laying out the user interface based on the autopilot).
 */
struct HEARTBEAT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 0;
    static constexpr size_t LENGTH = 9;
    static constexpr size_t MIN_LENGTH = 9;
    static constexpr uint8_t CRC_EXTRA = 50;
    static constexpr auto NAME = "HEARTBEAT";


    uint8_t type; /*<  Type of the MAV (quadrotor, helicopter, etc.) */
    uint8_t autopilot; /*<  Autopilot type / class. */
    uint8_t base_mode; /*<  System mode bitmap. */
    uint32_t custom_mode; /*<  A bitfield for use for autopilot-specific flags */
    uint8_t system_status; /*<  System status flag. */
    uint8_t mavlink_version; /*<  MAVLink version, not writable by user, gets added by protocol because of magic data type: uint8_t_mavlink_version */


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
        ss << "  type: " << +type << std::endl;
        ss << "  autopilot: " << +autopilot << std::endl;
        ss << "  base_mode: " << +base_mode << std::endl;
        ss << "  custom_mode: " << custom_mode << std::endl;
        ss << "  system_status: " << +system_status << std::endl;
        ss << "  mavlink_version: " << +mavlink_version << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << custom_mode;                   // offset: 0
        map << type;                          // offset: 4
        map << autopilot;                     // offset: 5
        map << base_mode;                     // offset: 6
        map << system_status;                 // offset: 7
        map << uint8_t(3);               // offset: 8
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> custom_mode;                   // offset: 0
        map >> type;                          // offset: 4
        map >> autopilot;                     // offset: 5
        map >> base_mode;                     // offset: 6
        map >> system_status;                 // offset: 7
        map >> mavlink_version;               // offset: 8
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
