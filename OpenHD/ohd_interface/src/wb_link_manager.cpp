//
// Created by consti10 on 13.09.23.
//

#include <cstring>
#include <sstream>
#include "wb_link_manager.h"
#include "openhd_spdlog.h"

namespace openhd::wb {

std::optional<ManagementFrameData> parse_data_management(const uint8_t *data, const int data_len) {
    if(data_len==sizeof(ManagementFrameData)+1 && data[0]==MNGMNT_PACKET_ID_CHANNEL_WIDTH){
        ManagementFrameData ret{};
        std::memcpy(&ret,&data[1],data_len);
        return ret;
    }
    return std::nullopt;
}

std::vector<uint8_t> pack_management_frame(const ManagementFrameData& data) {
    std::vector<uint8_t> ret;
    ret.resize(1+sizeof(ManagementFrameData));
    ret[0]=MNGMNT_PACKET_ID_CHANNEL_WIDTH;
    std::memcpy(&ret[1],(void*)&data,sizeof(ManagementFrameData));
    return ret;
}

std::string management_frame_to_string(const ManagementFrameData &data) {
    return fmt::format("Center: {}Mhz BW:{}Mhz",(int)data.center_frequency_mhz,(int)data.bandwidth_mhz);
}

}
