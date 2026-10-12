// SPDX-License-Identifier: GPL-3.0-only
#include "internal/OHDLinkStatisticsHelper.h"
#include <cassert>
#include <iostream>

int main() {
  using namespace openhd::link_statistics;
  Xmavlink_openhd_stats_monitor_mode_wifi_card_t card{};
  assert(card.count_p_received == 0 && card.rx_rssi == 0);
  card.NON_MAVLINK_CARD_ACTIVE = true;
  card.card_sub_type = 7; // application-only, never serialized as a new field
  card.rx_rssi = -61;
  card.dummy0 = -128; // unavailable EVM
  card.dummy1 = -321; // native thermal metadata
  card.dummy2 = 0x123456;
  auto packet = openhd::LinkStatisticsHelper::pack_card(100, 191, 2, card);
  mavlink_openhd_stats_monitor_mode_wifi_card_t decoded{};
  mavlink_msg_openhd_stats_monitor_mode_wifi_card_decode(&packet.m, &decoded);
  assert(decoded.card_index == 2 && decoded.rx_rssi == -61);
  assert(decoded.dummy0 == -128 && decoded.dummy1 == -321 && decoded.dummy2 == 0x123456);
  assert(packet.m.len == MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_CARD_LEN);
  Xmavlink_openhd_stats_wb_video_ground_t ground{};
  ground.curr_incoming_bitrate = 11000000;
  ground.dummy0 = 3; ground.dummy1 = 17; ground.dummy2 = 89;
  auto video = openhd::LinkStatisticsHelper::pack_vid_gnd(100, 191, ground);
  mavlink_openhd_stats_wb_video_ground_t video_decoded{};
  mavlink_msg_openhd_stats_wb_video_ground_decode(&video.m, &video_decoded);
  assert(video_decoded.curr_incoming_bitrate == 11000000);
  assert(video_decoded.dummy0 == 3 && video_decoded.dummy1 == 17 && video_decoded.dummy2 == 89);
  Xmavlink_openhd_stats_telemetry_t telemetry{};
  assert(telemetry.to_string().find("curr_tx_pps:0") != std::string::npos);
  std::cout << "Generated statistics, local state and legacy metadata passed\n";
}
