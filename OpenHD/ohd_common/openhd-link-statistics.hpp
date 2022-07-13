//
// Created by consti10 on 13.07.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_HPP_

#include <string>
#include <sstream>

// NOTE: CURRENTLY MESSED UP / HACKY, NEEDS CARE
namespace openhd::link_statistics{

// Data from all RX instances
struct StatsTotalRxStreams{
  uint64_t count_p_all=0; // accumulate all packets from all streams
  uint64_t count_p_bad_all=0; // bad packets
  [[nodiscard]] std::string to_string()const{
	std::stringstream ss;
	ss<<"StatsTotalRxStreams"<<"{"<<count_p_all<<", "<<count_p_bad_all<<"}";
	return ss.str();
  }
};

struct StatsPerCard{
  bool exists_in_openhd=false; // We have place for up to X wifi cards, but they might be unused - don't waste any telemetry bandwidth on these cards
  int8_t rx_rssi=INT8_MAX; // dBm / rssi, mavlink also defaults to INT8_MAX - makes sense if in dbm
  uint32_t count_p_received=0;
  uint32_t count_p_injected=0;
  [[nodiscard]] std::string to_string(const int index)const{
	std::stringstream ss;
	ss << "StatsPerCard"<<index<<"{exists:" << (exists_in_openhd ? "Y":"N") << ", rssi:" << (int)rx_rssi <<
	   ", count_p_received:" << count_p_received << ", count_p_injected:" << count_p_injected << "}";
	return ss.str();
  }
};
// Stats per connected card
using StatsAllCards=std::array<StatsPerCard,4>;

struct AllStats{
  openhd::link_statistics::StatsTotalRxStreams stats_total_rx_streams{};
  openhd::link_statistics::StatsAllCards stats_all_cards{};
};

typedef std::function<void(AllStats all_stats)> STATS_CALLBACK;

}
#endif //OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_HPP_
