#ifndef STREAMS_H
#define STREAMS_H

#include <array>
#include <chrono>
#include <utility>
#include <vector>
#include <utility>

#include "openhd-wifi.hpp"
#include "openhd-profile.hpp"
#include "openhd-link-statistics.hpp"

#include "../../lib/wifibroadcast/src/UDPWfibroadcastWrapper.hpp"

/**
 * This class takes a list of discovered wifi cards (and their settings) and
 * is responsible for configuring the given cards and then setting up all the Wifi-broadcast streams needed for OpenHD.
 * This class assumes a corresponding instance on the air or ground unit, respective.
 */
class WBStreams {
 public:
  explicit WBStreams(OHDProfile profile,std::vector<std::shared_ptr<WifiCardHolder>> broadcast_cards);
  // register callback that is called in regular intervals with link statistics
  void set_callback(openhd::link_statistics::STATS_CALLBACK stats_callback){
	_stats_callback=std::move(stats_callback);
  }
  // Verbose string about the current state.
  [[nodiscard]] std::string createDebug() const;
  // see interface
  void addExternalDeviceIpForwarding(const std::string& ip);
  void removeExternalDeviceIpForwarding(const std::string& ip);
  // Returns true if this WBStream has ever received any data. If no data has been ever received after X seconds,
  // there most likely was an unsuccessful frequency change.
  [[nodiscard]] bool ever_received_any_data()const;
  /*openhd::link_statistics::StatsTotalRxStreams get_stats_all_rx_streams();
  openhd::link_statistics::StatsAllCards get_stats_all_cards();*/
 private:
  const OHDProfile _profile;
  const int DEFAULT_MCS_INDEX = 5;
  std::vector<std::shared_ptr<WifiCardHolder>> _broadcast_cards;
 private:
  void configure();
  void configure_telemetry();
  void configure_video();
  // For telemetry, bidirectional in opposite directions
  std::unique_ptr<UDPWBTransmitter> udpTelemetryTx;
  std::unique_ptr<UDPWBReceiver> udpTelemetryRx;
  // For video, on air there are only tx instances, on ground there are only rx instances.
  std::vector<std::unique_ptr<UDPWBTransmitter>> udpVideoTxList;
  std::vector<std::unique_ptr<UDPWBReceiver>> udpVideoRxList;
  // TODO make more configurable
  [[nodiscard]] std::unique_ptr<UDPWBTransmitter> createUdpWbTx(uint8_t radio_port, int udp_port,bool enableFec)const;
  [[nodiscard]] std::unique_ptr<UDPWBReceiver> createUdpWbRx(uint8_t radio_port, int udp_port);
  [[nodiscard]] std::vector<std::string> get_rx_card_names()const;
  // called from the wifibroadcast instance(s), which have their own threads.
  std::mutex _statisticsDataLock;
  void onNewStatisticsData(const OpenHDStatisticsWriter::Data& data);
  // hacky, we accumulate the stats for all RX streams, which are 1 on the air (telemetry rx) and
  // 3 on the ground (telemetry and 2x video rx)
  // first is always telemetry, second and third are video if on ground
  std::array<OpenHDStatisticsWriter::Data,3> _last_stats_per_rx_stream{};
  // OpenHD
  // dBm / rssi for all connected cards that are doing wifibroadcast
  openhd::link_statistics::StatsAllCards _stats_all_cards{};
  openhd::link_statistics::StatsTotalAllStreams _stats_total_all_streams{};
  openhd::link_statistics::STATS_CALLBACK _stats_callback=nullptr;
};

#endif
