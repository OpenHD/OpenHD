#ifndef STREAMS_H
#define STREAMS_H

#include <array>
#include <chrono>
#include <utility>
#include <vector>
#include <utility>
#include <optional>

#include "OHDWifiCard.hpp"
#include "openhd-profile.hpp"
#include "openhd-platform.hpp"
#include "openhd-link-statistics.hpp"
#include "openhd-spdlog.hpp"
#include "WBStreamsSettings.hpp"
#include "mavlink_settings/ISettingsComponent.hpp"

#include "../../lib/wifibroadcast/src/UDPWfibroadcastWrapper.hpp"

/**
 * This class takes a list of discovered wifi cards (and their settings) and
 * is responsible for configuring the given cards and then setting up all the Wifi-broadcast streams needed for OpenHD.
 * This class assumes a corresponding instance on the air or ground unit, respective.
 */
class WBStreams {
 public:
  /**
   * @param broadcast_cards list of discovered wifi card(s) that support monitor mode & are injection capable. Needs to be at least
   * one card, and only one card on an air unit.
   */
  explicit WBStreams(OHDProfile profile,OHDPlatform platform,std::vector<std::shared_ptr<WifiCardHolder>> broadcast_cards);
  WBStreams(const WBStreams&)=delete;
  WBStreams(const WBStreams&&)=delete;
  // register callback that is called in regular intervals with link statistics
  void set_callback(openhd::link_statistics::STATS_CALLBACK stats_callback);
  // Verbose string about the current state.
  // could be const if there wasn't the mutex
  [[nodiscard]] std::string createDebug();
  // start or stop video data forwarding to another external device
  // NOTE: Only for the ground unit, and only for video (see OHDInterface for more info)
  void addExternalDeviceIpForwardingVideoOnly(const std::string& ip);
  void removeExternalDeviceIpForwardingVideoOnly(const std::string& ip);
  // Returns true if this WBStream has ever received any data. If no data has been ever received after X seconds,
  // there most likely was an unsuccessful frequency change.
  [[nodiscard]] bool ever_received_any_data();
  // Some settings need a full restart of the tx / rx instances to apply
  void restart();
  // schedule an asynchronous restart. if there is already a restart scheduled, return immediately
  void restart_async(std::chrono::milliseconds delay=std::chrono::milliseconds(0));
  // set the frequency (wifi channel) of all wifibroadcast cards
  bool set_frequency(int frequency);
  // set the tx power of all wifibroadcast cards
  bool set_txpower(int tx_power);
  // set the mcs index for all wifibroadcast cards
  bool set_mcs_index(int mcs_index);
  bool set_fec_block_length(int block_length);
  bool set_fec_percentage(int fec_percentage);
  // set the channel width
  // TODO doesn't work yet, aparently we need more than only the pcap header.
  bool set_channel_width(int channel_width);
  // settings hacky begin
  std::vector<openhd::Setting> get_all_settings();
  //void process_new_setting(openhd::Setting changed_setting);
  // settings hacky end
  // Check if all cards support changing the mcs index
  bool validate_cards_support_setting_mcs_index();
  // Check if all cards support changing the channel width
  bool validate_cards_support_setting_channel_width();
 private:
  const OHDProfile _profile;
  const OHDPlatform _platform;
  std::vector<std::shared_ptr<WifiCardHolder>> _broadcast_cards;
 private:
  // This needs some proper investigation !
  // In short: Make sure no OS service(s) that could interfere with monitor mode run on the cards used for wifibroadcast
  void takeover_cards();
  // set cards to monitor mode and set the right frequency, tx power
  void configure_cards();
  // start telemetry and video rx/tx stream(s)
  void configure_streams();
  void configure_telemetry();
  void configure_video();
  //openhd::WBStreamsSettings _last_settings;
  // Protects all the tx / rx instances, since we have the restart() from the settings.
  std::mutex _wbRxTxInstancesLock;
  std::unique_ptr<openhd::WBStreamsSettingsHolder> _settings;
  // For telemetry, bidirectional in opposite directions
  std::unique_ptr<UDPWBTransmitter> udpTelemetryTx;
  std::unique_ptr<UDPWBReceiver> udpTelemetryRx;
  // For video, on air there are only tx instances, on ground there are only rx instances.
  std::vector<std::unique_ptr<UDPWBTransmitter>> udpVideoTxList;
  std::vector<std::unique_ptr<UDPWBReceiver>> udpVideoRxList;
  // TODO make more configurable
  [[nodiscard]] std::unique_ptr<UDPWBTransmitter> createUdpWbTx(uint8_t radio_port, int udp_port,bool enableFec,std::optional<int> udp_recv_buff_size=std::nullopt)const;
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
  openhd::link_statistics::STATS_CALLBACK _stats_callback=nullptr;
  //
  std::mutex _restart_async_lock;
  std::unique_ptr<std::thread> _restart_async_thread=nullptr;
  // last calculated "All stats".
  openhd::link_statistics::AllStats _last_all_stats;
  std::shared_ptr<spdlog::logger> m_console;
};

#endif
