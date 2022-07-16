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

// Data from all RX and all TX instances on either ground or air, accumulated.
struct StatsTotalAllStreams{
  uint64_t count_wifi_packets_received=0; // current count of all received Wi-Fi packets
  uint64_t count_bytes_received=0; // current count of all received bytes, does not include IEE802 header or similar, but does include FEC overhead
  uint64_t count_wifi_packets_injected=0; // current count of all injected Wi-Fi packets
  uint64_t count_bytes_injected=0;  // current count of all outgoing bytes, does not include IEE802 header or similar, but does include FEC overhead
  uint64_t curr_video0_bps=0; // current video bps, when on air this is the bitrate of the video encoder (what's injected), when on ground
  uint64_t curr_video1_bps=0;// this is the bitrate received. For both primary and secondary video stream.
  // telemetry is both rx and tx on both air and ground
  uint64_t curr_telemetry_rx_bps=0; // curr ingoing telemetry, in bps
  uint64_t curr_telemetry_tx_bps=0; // curr outgoing telemetry in bps
  [[nodiscard]] std::string to_string()const{
	std::stringstream ss;
	ss << "StatsTotalAllStreams"<<"{count_wifi_packets_received:" << count_wifi_packets_received << ", count_bytes_received:" << (int)count_bytes_received <<
	   ", count_wifi_packets_injected:" << count_wifi_packets_injected << ", count_bytes_injected:" << count_bytes_injected<<"\n"
	   <<",video0:"<<bitrate_to_string(curr_video0_bps)<<",video1:"<<bitrate_to_string(curr_video1_bps)
	   <<",tele_rx:"<<bitrate_to_string(curr_telemetry_rx_bps)<<",tele_tx:"<<bitrate_to_string(curr_telemetry_tx_bps)<< "}";
	return ss.str();
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

// stats for the video stream, only generated on the video rx, and therefore only produced on the ground station
struct StatsFECVideoStreamRx{
  // total block count
  uint64_t count_blocks_total = 0;
  // a block counts as "lost" if it was removed before being fully received or recovered
  uint64_t count_blocks_lost = 0;
  // a block counts as "recovered" if it was recovered using FEC packets
  uint64_t count_blocks_recovered = 0;
  // n of primary fragments that were reconstructed during the recovery process of a block
  uint64_t count_fragments_recovered = 0;
  // n of forwarded bytes
  uint64_t count_bytes_forwarded=0;
};
static std::ostream& operator<<(std::ostream& strm, const StatsFECVideoStreamRx& obj){
  std::stringstream ss;
  ss<<"StatsFECVideoStreamRx{blocks_total:"<<obj.count_blocks_lost<<",blocks_lost:"<<obj.count_blocks_lost<<",blocks_recovered:"<<obj.count_blocks_recovered
	<<",fragments_recovered:"<<obj.count_fragments_recovered<<"bytes_forwarded:"<<obj.count_bytes_forwarded<<"}";
  strm<<ss.str();
  return strm;
}

struct AllStats{
  //openhd::link_statistics::StatsTotalRxStreams stats_total_rx_streams{};
  openhd::link_statistics::StatsTotalAllStreams stats_total_all_streams{};
  openhd::link_statistics::StatsAllCards stats_all_cards{};
  // optional, since this is only generated on the ground pi (where the video rx-es are)
  std::optional<StatsFECVideoStreamRx> stats_video_stream0_rx;
  std::optional<StatsFECVideoStreamRx> stats_video_stream1_rx;
};
static std::ostream& operator<<(std::ostream& strm, const AllStats& obj){
  std::stringstream ss;
  ss<<obj.stats_total_all_streams.to_string()<<"\n";
  int idx=0;
  for(const auto& card:obj.stats_all_cards){
	if(card.exists_in_openhd){
	  ss<<card.to_string(idx)<<"\n";
	  idx++;
	}
  }
  if(obj.stats_video_stream0_rx.has_value()){
	ss<<obj.stats_video_stream0_rx.value()<<"\n";
  }
  if(obj.stats_video_stream1_rx.has_value()){
	ss<<obj.stats_video_stream1_rx.value()<<"\n";
  }
  strm<<ss.str();
  return strm;
}

typedef std::function<void(AllStats all_stats)> STATS_CALLBACK;

}
#endif //OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_HPP_
