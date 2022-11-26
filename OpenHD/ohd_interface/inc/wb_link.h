#ifndef STREAMS_H
#define STREAMS_H

#include <array>
#include <chrono>
#include <optional>
#include <utility>
#include <vector>

#include "../../lib/wifibroadcast/src/UdpWBReceiver.hpp"
#include "../../lib/wifibroadcast/src/UdpWBTransmitter.hpp"
#include "mavlink_settings/ISettingsComponent.hpp"
#include "openhd-action-handler.hpp"
#include "openhd-link-statistics.hpp"
#include "openhd-platform.hpp"
#include "openhd-profile.hpp"
#include "openhd-spdlog.hpp"
#include "wb_link_settings.hpp"
#include "wifi_card.hpp"

/**
 * This class takes a list of discovered wifi cards (and their settings) and
 * is responsible for configuring the given cards and then setting up all the Wifi-broadcast streams needed for OpenHD.
 * In the end, we have a link that has some broadcast characteristics for video (video is always broadcast from air to ground)
 * but also a bidirectional link (without re-transmission(s)) for telemetry.
 * This class assumes a corresponding instance on the air or ground unit, respective.
 */
class WBLink {
 public:
  /**
   * @param broadcast_cards list of discovered wifi card(s) that support monitor mode & are injection capable. Needs to be at least
   * one card, and only one card on an air unit. The given cards need to support monitor mode and either 2.4G or 5G wifi.
   * @param opt_action_handler global openhd action handler, optional (can be nullptr during testing of specific modules instead
   * of testing a complete running openhd instance)
   */
  explicit WBLink(OHDProfile profile,OHDPlatform platform,std::vector<std::shared_ptr<WifiCardHolder>> broadcast_cards,
                     std::shared_ptr<openhd::ActionHandler> opt_action_handler);
  WBLink(const WBLink&)=delete;
  WBLink(const WBLink&&)=delete;
  ~WBLink();
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
  // returns all mavlink settings, values might change depending on the used hardware
  std::vector<openhd::Setting> get_all_settings();
  // schedule an asynchronous restart. if there is already a restart scheduled, return immediately
  void restart_async(std::chrono::milliseconds delay=std::chrono::milliseconds(0));
  // needs to be set for FEC auto to work
  void set_video_codec(int codec);
 private:
  // Some settings need a full restart of the tx / rx instances to apply
  void restart();
  // set the frequency (wifi channel) of all wifibroadcast cards
  bool set_frequency(int frequency);
  //
  void apply_frequency_and_channel_width();
  // set the tx power of all wifibroadcast cards
  bool set_txpower(int tx_power);
  // set the mcs index for all wifibroadcast cards
  bool set_mcs_index(int mcs_index);
  // called by the async pattern
  void apply_mcs_index();
  bool set_video_fec_block_length(int block_length);
  bool set_video_fec_percentage(int fec_percentage);
  bool set_enable_wb_video_variable_bitrate(int value);
  // set the channel width
  bool set_channel_width(int channel_width);
  // Check if all cards support changing the mcs index
  bool validate_cards_support_setting_mcs_index();
  // Check if all cards support changing the channel width
  bool validate_cards_support_setting_channel_width();
  // Make sure no processes interfering with monitor mode run on the given cards,
  // then sets them to monitor mode
  void takeover_cards_monitor_mode();
  // set the right frequency, channel width and tx power. Cards need to be in monitor mode already !
  void configure_cards();
  // start telemetry and video rx/tx stream(s)
  void configure_streams();
  void configure_telemetry();
  void configure_video();
  //openhd::WBStreamsSettings _last_settings;
  // Protects all the tx / rx instances, since we have the restart() from the settings.
  std::mutex m_wbRxTxInstancesLock;
  std::unique_ptr<openhd::WBStreamsSettingsHolder> m_settings;
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
 private:
  const OHDProfile m_profile;
  const OHDPlatform m_platform;
  std::vector<std::shared_ptr<WifiCardHolder>> m_broadcast_cards;
  std::shared_ptr<openhd::ActionHandler> m_opt_action_handler=nullptr;
  std::mutex m_restart_async_lock;
  std::unique_ptr<std::thread> m_restart_async_thread =nullptr;
  std::shared_ptr<spdlog::logger> m_console;
  // disable all openhd frequency checking - note that openhd just uses the proper iw command to set a frequency - if setting
  // the frequency actually had an effect, it doesn't know (cannot really know) and therefore QOpenHD can then report a different wifi freq,
  // even though the frequency actually hasn't changed
  static constexpr auto FIlE_DISABLE_ALL_FREQUENCY_CHECKS="/boot/openhd/disable_all_frequency_checks.txt";
  const bool m_disable_all_frequency_checks;
  int m_curr_video_codec=0;
 private:
  bool m_recalculate_stats_thread_run;
  std::unique_ptr<std::thread> m_recalculate_stats_thread;
  void loop_recalculate_stats();
  int64_t last_tx_error_count=-1;
  int64_t last_recommended_bitrate=-1;
  //
  class WorkItem{
   public:
    explicit WorkItem(std::function<void()> work,std::chrono::steady_clock::time_point earliest_execution_time):
    m_earliest_execution_time(earliest_execution_time),m_work(std::move(work)){
      //
    }
    void execute(){
      m_work();
    }
   private:
    const std::chrono::steady_clock::time_point m_earliest_execution_time;
    const std::function<void()> m_work;
  };
  void schedule_work_item(std::shared_ptr<WorkItem> work_item);
  std::shared_ptr<WorkItem> get_scheduled_work_item();
  std::mutex m_work_item_queue_mutex;
  std::queue<std::shared_ptr<WorkItem>> m_work_item_queue;
};

#endif
