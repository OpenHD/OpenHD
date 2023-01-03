#ifndef STREAMS_H
#define STREAMS_H

#include <array>
#include <chrono>
#include <optional>
#include <utility>
#include <vector>

#include "../../lib/wifibroadcast/src/UdpWBReceiver.hpp"
#include "../../lib/wifibroadcast/src/UdpWBTransmitter.hpp"
#include "../../lib/wifibroadcast/src/ForeignPacketsReceiver.h"
#include "mavlink_settings/ISettingsComponent.hpp"
#include "openhd-action-handler.hpp"
#include "openhd-link-statistics.hpp"
#include "openhd-platform.hpp"
#include "openhd-profile.hpp"
#include "openhd-spdlog.hpp"
#include "openhd-telemetry-tx-rx.h"
#include "wb_link_settings.hpp"
#include "wifi_card.hpp"

#include "ohd_link.hpp"

/**
 * This class takes a list of cards supporting monitor mode (only 1 card on air) and
 * is responsible for configuring the given cards and then setting up all the Wifi-broadcast streams needed for OpenHD.
 * In the end, we have a link that has some broadcast characteristics for video (video is always broadcast from air to ground)
 * but also a bidirectional link (without re-transmission(s)) for telemetry.
 * This class assumes a corresponding instance on the air or ground unit, respective.
 */
class WBLink :public OHDLink{
 public:
  /**
   * @param broadcast_cards list of discovered wifi card(s) that support monitor mode & are injection capable. Needs to be at least
   * one card, and only one card on an air unit. The given cards need to support monitor mode and either 2.4G or 5G wifi.
   * @param opt_action_handler global openhd action handler, optional (can be nullptr during testing of specific modules instead
   * of testing a complete running openhd instance)
   */
  explicit WBLink(OHDProfile profile,OHDPlatform platform,std::vector<WiFiCard> broadcast_cards,
                     std::shared_ptr<openhd::ActionHandler> opt_action_handler);
  WBLink(const WBLink&)=delete;
  WBLink(const WBLink&&)=delete;
  ~WBLink();
  // Verbose string about the current state.
  [[nodiscard]] std::string createDebug()const;
  // returns all mavlink settings, values might change depending on the used hardware
  std::vector<openhd::Setting> get_all_settings();
  openhd::Space get_current_frequency_channel_space()const;
 private:
  // validate param, then schedule change
  bool request_set_frequency(int frequency);
  // validate param, then schedule change
  bool request_set_channel_width(int channel_width);
  // apply the frequency (wifi channel) of all wifibroadcast cards
  bool apply_frequency_and_channel_width();
  // validate param, then schedule change
  bool request_set_txpower(int tx_power);
  // set the tx power of all wifibroadcast cards
  void apply_txpower();
  // validate param, then schedule change
  bool request_set_mcs_index(int mcs_index);
  // set the mcs index for all tx instances
  void apply_mcs_index();
  // These do not "break" the bidirectional connectivity and therefore
  // can be changed easily on the fly
  bool set_video_fec_block_length(int block_length);
  bool set_video_fec_percentage(int fec_percentage);
  bool set_enable_wb_video_variable_bitrate(int value);
  bool set_max_fec_block_size_for_platform(int value);
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
  void configure_telemetry();
  void configure_video();
  std::unique_ptr<openhd::WBStreamsSettingsHolder> m_settings;
  // For telemetry, bidirectional in opposite directions
  std::unique_ptr<WBTransmitter> m_wb_tele_tx;
  std::unique_ptr<AsyncWBReceiver> m_wb_tele_rx;
  // For video, on air there are only tx instances, on ground there are only rx instances.
  std::vector<std::unique_ptr<WBTransmitter>> m_wb_video_tx_list;
  std::vector<std::unique_ptr<AsyncWBReceiver>> m_wb_video_rx_list;
  std::unique_ptr<ForeignPacketsReceiver> m_foreign_packets_receiver;
  // Reads the current settings and creates the appropriate Radiotap Header params
  [[nodiscard]] RadiotapHeader::UserSelectableParams create_radiotap_params()const;
  [[nodiscard]] TOptions create_tx_options(uint8_t radio_port,bool is_video)const;
  [[nodiscard]] ROptions create_rx_options(uint8_t radio_port)const;
  std::unique_ptr<WBTransmitter> create_wb_tx(uint8_t radio_port,bool is_video);
  std::unique_ptr<AsyncWBReceiver> create_wb_rx(uint8_t radio_port,WBReceiver::OUTPUT_DATA_CALLBACK cb);
  [[nodiscard]] std::vector<std::string> get_rx_card_names()const;
 private:
  const OHDProfile m_profile;
  const OHDPlatform m_platform;
  const std::vector<WiFiCard> m_broadcast_cards;
  std::shared_ptr<openhd::ActionHandler> m_opt_action_handler=nullptr;
  std::shared_ptr<spdlog::logger> m_console;
  // disable all openhd frequency checking - note that I am quite sure about the correctness of openhd internal checking in regards to wifi channels ;)
  const bool m_disable_all_frequency_checks;
 private:
  // We have one worker thread for asynchronously performing operation(s) like changing the frequency
  // but also recalculating statistics that are then forwarded to openhd_telemetry for broadcast
  bool m_work_thread_run;
  std::unique_ptr<std::thread> m_work_thread;
  // Recalculate stats, apply settings asynchronously and more
  void loop_do_work();
  // update statistics, done in regular intervals, update data is given to the ohd_telemetry module via the action handler
  void update_statistics();
  static constexpr auto RECALCULATE_STATISTICS_INTERVAL=std::chrono::seconds(1);
  std::chrono::steady_clock::time_point m_last_stats_recalculation=std::chrono::steady_clock::now();
  // Do rate adjustments, does nothing if variable bitrate is disabled
  void perform_rate_adjustment();
  static constexpr auto RATE_ADJUSTMENT_INTERVAL=std::chrono::seconds(1);
  std::chrono::steady_clock::time_point m_last_rate_adjustment=std::chrono::steady_clock::now();
  int64_t last_tx_error_count=-1;
  int64_t last_recommended_bitrate=-1;
  // A bit dirty, some settings need to be applied asynchronous
  class WorkItem{
   public:
    explicit WorkItem(std::function<void()> work,std::chrono::steady_clock::time_point earliest_execution_time):
    m_earliest_execution_time(earliest_execution_time),m_work(std::move(work)){
    }
    void execute(){
      m_work();
    }
    bool ready_to_be_executed(){
      return std::chrono::steady_clock::now()>=m_earliest_execution_time;
    }
   private:
    const std::chrono::steady_clock::time_point m_earliest_execution_time;
    const std::function<void()> m_work;
  };
  void schedule_work_item(const std::shared_ptr<WorkItem>& work_item);
  // We limit changing specific params to one after another
  bool check_work_queue_empty();
  std::mutex m_work_item_queue_mutex;
  std::queue<std::shared_ptr<WorkItem>> m_work_item_queue;
  static constexpr auto DELAY_FOR_TRANSMIT_ACK =std::chrono::seconds(2);
 private:
  bool set_wb_rtl8812au_tx_pwr_idx_override(int value);
  bool has_rtl8812au();
 private:
  void transmit_telemetry_data(std::shared_ptr<std::vector<uint8_t>> data)override;
  // Called by the camera stream on the air unit only
  // transmit video data via wifibradcast
  void transmit_video_data(int stream_index,const openhd::FragmentedVideoFrame& fragmented_video_frame) override;
 public:
  // Warning: This operation will block the calling thread for up to X ms.
  // During scan, you cannot change any wb settings
  struct ScanResult{
    bool success=false;
    uint32_t frequency =0;
  };
  // Testing shows we have to listen for up to 1 second to reliable get data (the wifi card might take some time switching)
  static constexpr std::chrono::seconds DEFAULT_SCAN_TIME_PER_CHANNEL{1};
  // checking both 2G and 5G channels takes really long, but in rare cases might be wanted by the user
  struct ScanChannelsParams{
    std::chrono::nanoseconds duration_per_channel=DEFAULT_SCAN_TIME_PER_CHANNEL;
    bool check_2g_channels_if_card_support=false;
    bool check_5g_channels_if_card_supports=false;
  };
  ScanResult scan_channels(const ScanChannelsParams& scan_channels_params);
  // queue it up on the work queue
  void async_scan_channels(ScanChannelsParams scan_channels_params);
 private:
  std::atomic<bool> is_scanning=false;
  void reset_all_count_p_stats();
  int get_count_p_all();
  int get_count_p_decryption_ok();
 private:
  // We return false on all the change settings request(s) if there is already a change operation queued
  // up or we currently perform a channel scan
  // Not completely "thread safe" so to say but good enough.
  bool check_in_state_support_changing_settings();
};

#endif
