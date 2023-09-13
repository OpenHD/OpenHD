//
// Created by consti10 on 13.09.23.
//

#ifndef OPENHD_WBLINKMANAGER_H
#define OPENHD_WBLINKMANAGER_H

#include <variant>
#include <optional>
#include <cstdint>
#include <vector>
#include <string>

namespace openhd::wb{

static constexpr uint8_t MNGMNT_PACKET_ID_CHANNEL_WIDTH=0;

struct ManagementFrameData{
    uint32_t center_frequency_mhz;
    uint8_t bandwidth_mhz;
}__attribute__ ((packed));
std::vector<uint8_t> pack_management_frame(const ManagementFrameData& data);
std::optional<ManagementFrameData> parse_data_management(const uint8_t *data, int data_len);
std::string management_frame_to_string(const ManagementFrameData& data);


}


#endif //OPENHD_WBLINKMANAGER_H
