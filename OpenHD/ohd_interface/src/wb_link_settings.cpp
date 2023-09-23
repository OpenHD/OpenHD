//
// Created by consti10 on 19.09.23.
//
#include "wb_link_settings.h"

#include "include_json.hpp"

namespace openhd{

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WBLinkSettings, wb_frequency, wb_air_tx_channel_width, wb_air_mcs_index,
                                   wb_enable_stbc, wb_enable_ldpc, wb_enable_short_guard,
                                   wb_tx_power_milli_watt, wb_tx_power_milli_watt_armed,
                                   wb_rtl8812au_tx_pwr_idx_override, wb_rtl8812au_tx_pwr_idx_override_armed,
                                   wb_video_fec_percentage,
                                   wb_video_rate_for_mcs_adjustment_percent,
                                   wb_max_fec_block_size_for_platform,
                                   wb_mcs_index_via_rc_channel,
                                   enable_wb_video_variable_bitrate,
                                   wb_enable_listen_only_mode);

std::optional<WBLinkSettings> openhd::WBLinkSettingsHolder::impl_deserialize(const std::string &file_as_string) const {
    return openhd_json_parse<WBLinkSettings>(file_as_string);
}

std::string WBLinkSettingsHolder::imp_serialize(const openhd::WBLinkSettings &data) const {
    const nlohmann::json tmp=data;
    return tmp.dump(4);
}
}

