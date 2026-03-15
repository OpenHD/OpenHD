#include <openhd/mavlink.h>
#include <mav_include.h>
#include <vector>


static constexpr uint16_t WALKSNAIL_PAYLOAD_TYPE = 0x5A00; // pick a custom ID

static std::vector<MavlinkMessage> pack_uart_data_to_mavlink(
    const uint8_t* data,
    size_t length,
    uint8_t src_sys_id,
    uint8_t target_sys_id
)
{
    std::vector<MavlinkMessage> ret;
    size_t offset = 0;

    while (offset < length) {
        const size_t chunk = std::min(length - offset,
                                      (size_t)MAVLINK_MSG_TUNNEL_FIELD_PAYLOAD_LEN);

        mavlink_message_t msg;
        mavlink_msg_tunnel_pack(
            src_sys_id,
            MAV_COMP_ID_ONBOARD_COMPUTER,
            &msg,
            target_sys_id,
            MAV_COMP_ID_ONBOARD_COMPUTER,
            WALKSNAIL_PAYLOAD_TYPE,
            static_cast<uint8_t>(chunk),
            data + offset
        );

        MavlinkMessage wrapped;
        wrapped.m = msg;
        wrapped.recommended_n_injections = 2; // lossy link, send twice
        ret.push_back(wrapped);

        offset += chunk;
    }
    return ret;
}
