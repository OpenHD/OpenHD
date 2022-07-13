//
// Created by consti10 on 13.07.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_H_

namespace openhd::link_statistics{

// Data from all RX instances
struct StatsTotalRxStreams{
  uint64_t count_p_all=0; // accumulate all packets from all streams
  uint64_t count_p_bad_all=0; // bad p
};

struct StatsPerCard{
  uint32_t count_p_received=0;
  uint32_t count_p_injected=0;
  int8_t rssi=INT8_MAX; // dBm / rssi, mavlink also defaults to INT8_MAX - makes sense if in dbm
};
// Stats per connected card
using StatsAllCards=std::array<StatsPerCard,4>;

}
#endif //OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_H_
