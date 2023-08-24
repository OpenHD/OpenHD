//
// Created by consti10 on 13.07.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_HPP_

#include <string>
#include <sstream>
#include <optional>
#include <cstring>

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
  uint64_t count_tx_inj_error_hint; /*<  count_tx_inj_error_hint*/
  uint64_t count_tx_dropped_packets; /*<  count_tx_dropped_packets*/
  uint64_t unused2; /*<  unused2*/
  uint64_t unused3; /*<  unused3*/
  int16_t curr_tx_pps; /*<  tx packets per second*/
  int16_t curr_rx_pps; /*<  rx packets per second*/
  int32_t curr_tx_bps; /*<  tx bits per second*/
  int32_t curr_rx_bps; /*<  rx bits per second*/
  int32_t curr_tx_card_idx; /*< curr tx card (for multi rx-es on ground) ,unused0 */
  int32_t curr_tx_mcs_index; /* curr tx mcs index used when injecting packets, unused1*/
  uint16_t curr_tx_channel_mhz;
  uint8_t curr_tx_channel_w_mhz;
  int16_t curr_rx_packet_loss_perc; /*<  curr_rx_packet_loss*/
  int16_t curr_rx_big_gaps_counter;
  int8_t tx_operating_mode = 0;
  uint16_t curr_rate_kbits=0;
  uint8_t curr_n_rate_adjustments=0;
  uint8_t curr_tx_stbc_lpdc_shortguard_bitfield;
  uint8_t curr_pollution_perc=0;
};

struct StatsTelemetry{
  uint64_t unused_0; /*<  unused_0*/
  uint64_t unused_1; /*<  unused_1*/
  int16_t curr_tx_pps; /*<  tx packets per second*/
  int16_t curr_rx_pps; /*<  rx packets per second*/
  int32_t curr_tx_bps; /*<  tx bits per second*/
  int32_t curr_rx_bps; /*<  rx bits per second*/
  int16_t curr_rx_packet_loss_perc; /*<  curr_rx_packet_loss_perc*/
  [[nodiscard]] std::string to_string()const{
    std::stringstream ss;
    ss<<"StatsTelemetry{";
    ss<<"curr_tx_pps:"<<curr_tx_pps<<",curr_rx_pps:"<<curr_rx_pps<<",curr_tx_bps:"<<curr_tx_bps<<", curr_rx_bps:"<< curr_rx_bps;
    ss<<", curr_rx_packet_loss_perc"<<curr_rx_packet_loss_perc;
    ss<<"}";
    return ss.str();
  }
};

struct StatsWBVideoAir{
  uint8_t link_index;
  int32_t curr_recommended_bitrate;
  int32_t curr_measured_encoder_bitrate;
  int32_t curr_injected_bitrate;
  int32_t curr_injected_pps;
  int32_t curr_dropped_frames;
  uint32_t curr_fec_encode_time_avg_us;
  uint32_t curr_fec_encode_time_min_us;
  uint32_t curr_fec_encode_time_max_us;
  uint16_t curr_fec_block_size_avg;
  uint16_t curr_fec_block_size_min;
  uint16_t curr_fec_block_size_max;
  uint32_t curr_time_until_tx_avg_us;
  uint32_t curr_time_until_tx_min_us;
  uint32_t curr_time_until_tx_max_us;
  int32_t curr_fec_percentage;
};

struct StatsWBVideoGround{
  uint8_t link_index;
  int32_t curr_incoming_bitrate;
  uint64_t count_blocks_total;
  uint64_t count_blocks_lost;
  uint64_t count_blocks_recovered;
  uint64_t count_fragments_recovered;
  uint32_t curr_fec_decode_time_avg_us;
  uint32_t curr_fec_decode_time_min_us;
  uint32_t curr_fec_decode_time_max_us;
  int32_t unused0;
  int32_t unused1;
};

struct StatsPerCard{
  bool exists_in_openhd=false; // We have place for up to X wifi cards, but they might be unused - don't waste any telemetry bandwidth on these cards
  uint8_t card_type=0;
  int8_t rx_rssi_card=INT8_MIN;
  int8_t rx_rssi_1=INT8_MIN; // dBm / rssi
  int8_t rx_rssi_2=INT8_MIN; // dBm / rssi
  int8_t signal_quality=-1;
  uint64_t count_p_received=0; //TODO
  uint64_t count_p_injected=0; //TODO
  int16_t tx_power_current=0;
  int16_t tx_power_armed=0;
  int16_t tx_power_disarmed=0;
  int8_t curr_rx_packet_loss_perc=0;
  uint8_t curr_status=0;
};
// Stats per connected card
using StatsAllCards=std::array<StatsPerCard,4>;

struct StatsAirGround{
  bool is_air=false;
  bool ready=false;
  // air and ground
  StatsMonitorModeLink monitor_mode_link;
  StatsTelemetry telemetry;
  StatsAllCards cards;
  // for air
  std::vector<StatsWBVideoAir> stats_wb_video_air;
  // for ground
  std::vector<StatsWBVideoGround> stats_wb_video_ground;
};

typedef std::function<void(StatsAirGround all_stats)> STATS_CALLBACK;

// We pack those 3 into a single uint8_t in the mavlink msg
struct StbcLpdcShortGuardBitfield {
  unsigned int stbc:1;
  unsigned int lpdc:1;
  unsigned int short_guard:1;
  unsigned int unused:5;
}__attribute__ ((packed));
static_assert(sizeof(StbcLpdcShortGuardBitfield)==1);
static uint8_t write_stbc_lpdc_shortguard_bitfield(bool stbc, bool lpdc,bool short_guard){
  StbcLpdcShortGuardBitfield bitfield{stbc,lpdc,short_guard,0};
  uint8_t ret;
  std::memcpy(&ret,(uint8_t*)&bitfield,1);
  return ret;
}
static StbcLpdcShortGuardBitfield get_stbc_lpdc_shortguard_bitfield(uint8_t bitfield){
  StbcLpdcShortGuardBitfield ret{};
  std::memcpy((uint8_t*)&ret,&bitfield,1);
  return ret;
}
}
#endif //OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_HPP_
