// MESSAGE CHANGE_OPERATOR_CONTROL_ACK support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief CHANGE_OPERATOR_CONTROL_ACK message
 *
 * Accept / deny control of this MAV
 */
struct CHANGE_OPERATOR_CONTROL_ACK : mavlink::Message {
    static constexpr msgid_t MSG_ID = 6;
    static constexpr size_t LENGTH = 3;
    static constexpr size_t MIN_LENGTH = 3;
    static constexpr uint8_t CRC_EXTRA = 104;
    static constexpr auto NAME = "CHANGE_OPERATOR_CONTROL_ACK";


    uint8_t gcs_system_id; /*<  ID of the GCS this message  */
    uint8_t control_request; /*<  0: request control of this MAV, 1: Release control of this MAV */
    uint8_t ack; /*<  0: ACK, 1: NACK: Wrong passkey, 2: NACK: Unsupported passkey encryption method, 3: NACK: Already under control */


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
        ss << "  gcs_system_id: " << +gcs_system_id << std::endl;
        ss << "  control_request: " << +control_request << std::endl;
        ss << "  ack: " << +ack << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << gcs_system_id;                 // offset: 0
        map << control_request;               // offset: 1
        map << ack;                           // offset: 2
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> gcs_system_id;                 // offset: 0
        map >> control_request;               // offset: 1
        map >> ack;                           // offset: 2
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
