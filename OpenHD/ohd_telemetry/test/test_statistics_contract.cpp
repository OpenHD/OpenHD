// SPDX-License-Identifier: GPL-3.0-only
#include "internal/OHDLinkStatisticsHelper.h"
#include <cassert>
#include <iostream>

// Common logging links against this application hook. Keep this protocol test
// independent of the production recording directory, like the registry tests.
const char* getVideoPath() { return "/tmp/"; }

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
  openhd::LinkActionHandler::AnalyzeChannelsResult survey{};
  survey.devourer_survey = true;
  survey.rf_batch_id = 123;
  survey.rf_page_offset = 30;
  survey.rf_total_channels = 47;
  survey.progress = 100;
  survey.channels_mhz[0] = 5745;
  survey.foreign_packets[0] = 0; // undecodable interference still has energy
  survey.survey_metrics[0] = 0xc000 | (85 << 7) | 73;
  const auto survey_msg = openhd::LinkStatisticsHelper::generate_msg_analyze_channels_progress(100, 191, survey);
  mavlink_openhd_wifbroadcast_analyze_channels_progress_t survey_decoded{};
  mavlink_msg_openhd_wifbroadcast_analyze_channels_progress_decode(&survey_msg.m, &survey_decoded);
  assert(survey_decoded.dummy0 == 0x4456 && survey_decoded.dummy1 == 123);
  assert(static_cast<uint32_t>(survey_decoded.dummy2) == ((47u << 16) | 30u));
  assert(survey_decoded.channels_mhz[0] == 5745 && survey_decoded.foreign_packets[0] == 0);
  assert(survey_decoded.dummy[0] == (0xc000 | (85 << 7) | 73));
  assert(survey_decoded.dummy[1] == 0 && survey_decoded.progress_perc == 100);
  assert(survey_msg.m.len == MAVLINK_MSG_ID_OPENHD_WIFBROADCAST_ANALYZE_CHANNELS_PROGRESS_LEN);
  survey.devourer_survey = false;
  survey.rf_sampled_busy = true;
  survey.rf_busy_centipercent[0] = 8200;
  const auto esp_msg = openhd::LinkStatisticsHelper::generate_msg_analyze_channels_progress(100, 191, survey);
  mavlink_msg_openhd_wifbroadcast_analyze_channels_progress_decode(&esp_msg.m, &survey_decoded);
  assert(survey_decoded.dummy0 == 0x4553 && survey_decoded.dummy[0] == 8200);
  std::cout << "Generated statistics, local state and legacy metadata passed\n";
}
