//
// Created by consti10 on 13.07.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_HPP_

#include <string>
#include <sstream>
#include <optional>

// NOTE: CURRENTLY MESSED UP / HACKY, NEEDS CARE
namespace openhd::link_statistics{

// for debugging
static std::string bitrate_to_string(uint64_t bits_per_second){
  const double mBits_per_second=static_cast<double>(bits_per_second)/(1000*1000);
  if(mBits_per_second>1){
	return std::to_string(mBits_per_second)+"mBit/s";
  }
  const double kBits_per_second=static_cast<double>(bits_per_second)/1000;
  return std::to_string(kBits_per_second)+"kBit/s";
}


// These structs match the custom openhd mavlink messages, kinda annoying but
// we do not have a mavlink dependency in ohd_interface so we need to duplicate that code

struct StatsMonitorModeLink{
  uint64_t curr_tx_pps;
  uint64_t curr_rx_pps;
  uint64_t curr_tx_bps;
  uint64_t curr_rx_bps;
  uint64_t curr_rx_packet_loss;
  int32_t unused0;
  int32_t unused1;
  int32_t unused2;
  int32_t unused3;
  [[nodiscard]] std::string to_string()const{
    return "TODO";
  }
};

struct StatsTelemetry{
  uint64_t curr_tx_pps;
  uint64_t curr_rx_pps;
  uint64_t curr_tx_bps;
  uint64_t curr_rx_bps;
  uint64_t curr_rx_packet_loss_perc;
  uint64_t unused_0;
  uint64_t unused_1;
  [[nodiscard]] std::string to_string()const{
    return "TODO";
  }
};

struct StatsWBVideoAir{
  uint8_t link_index;
  uint8_t curr_video_codec;
  int32_t curr_recommended_bitrate;
  int32_t curr_measured_encoder_bitrate;
  int32_t curr_injected_bitrate;
  int32_t curr_injected_pps;
  int32_t curr_dropped_packets;
  int32_t curr_fec_encode_time_avg_ms;
  int32_t curr_fec_encode_time_min_ms;
  int32_t curr_fec_encode_time_max_ms;
  int16_t curr_fec_block_size_avg;
  int16_t curr_fec_block_size_min;
  int16_t curr_fec_block_size_max;
  int32_t unused0;
  int32_t unused1;
  [[nodiscard]] std::string to_string()const{
    return "TODO";
  }
};

struct StatsWBVideoGround{
  uint8_t link_index;
  int32_t curr_incoming_bitrate;
  uint64_t count_blocks_total;
  uint64_t count_blocks_lost;
  uint64_t count_blocks_recovered;
  uint64_t count_fragments_recovered;
  int32_t curr_fec_decode_time_avg_ms;
  int32_t curr_fec_decode_time_min_ms;
  int32_t curr_fec_decode_time_max_ms;
  int32_t unused0;
  int32_t unused1;
  [[nodiscard]] std::string to_string()const{
    return "TODO";
  }
};

struct StatsPerCard{
  bool exists_in_openhd=false; // We have place for up to X wifi cards, but they might be unused - don't waste any telemetry bandwidth on these cards
  int8_t rx_rssi=INT8_MAX; // dBm / rssi, mavlink also defaults to INT8_MAX - makes sense if in dbm
  uint64_t count_p_received=0; //TODO
  uint64_t count_p_injected=0; //TODO
  [[nodiscard]] std::string to_string(const int index)const{
	std::stringstream ss;
	ss << "StatsPerCard"<<index<<"{exists:" << (exists_in_openhd ? "Y":"N") << ", rssi:" << (int)rx_rssi <<
	   ", count_p_received:" << count_p_received << ", count_p_injected:" << count_p_injected << "}";
	return ss.str();
  }
};
// Stats per connected card
using StatsAllCards=std::array<StatsPerCard,4>;

struct StatsAirGround{
  bool is_air=false;
  // air and ground
  StatsMonitorModeLink monitor_mode_link;
  StatsTelemetry telemetry;
  StatsAllCards cards;
  // for air
  StatsWBVideoAir air_video0;
  StatsWBVideoAir air_video1;
  // for ground
  StatsWBVideoGround ground_video0;
  StatsWBVideoGround ground_video1;
};

static std::ostream& operator<<(std::ostream& strm, const StatsAirGround& obj){
  std::stringstream ss;
  ss<<"StatsAirGround{\n";
  ss<<obj.cards.at(0).to_string(0)<<"\n";
  ss<<"}";
  strm<<ss.str();
  return strm;
}

typedef std::function<void(StatsAirGround all_stats)> STATS_CALLBACK;

}
#endif //OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_HPP_
