#ifndef STREAMS_H
#define STREAMS_H

#include <array>
#include <chrono>
#include <optional>
#include <utility>
#include <vector>

//#include "../../lib/wifibroadcast/src/ForeignPacketsReceiver.h"
//#include "../../lib/wifibroadcast/src/UdpWBReceiver.hpp"
//#include "../../lib/wifibroadcast/src/UdpWBTransmitter.hpp"
#include "../../lib/wifibroadcast/src/WBTxRx.h"
#include "../../lib/wifibroadcast/src/WBStreamTx.h"
#include "../../lib/wifibroadcast/src/WBStreamRx.h"
#include "openhd_action_handler.hpp"
#include "openhd_link.hpp"
#include "openhd_link_statistics.hpp"
#include "openhd_platform.h"
#include "openhd_profile.h"
#include "openhd_settings_imp.hpp"
#include "openhd_spdlog.h"
#include "wb_link_settings.hpp"
#include "wifi_card.h"
#include "wb_link_work_item.hpp"

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
   * In the case where there are multiple card(s), the first given card is used for transmission & receive, the other card(s) are not used
   * for transmission, only for receiving.
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
  [[nodiscard]] openhd::WifiSpace get_current_frequency_channel_space()const;
 private:
  // validate param, then schedule change
  bool request_set_frequency(int frequency);
  // validate param, then schedule change
  bool request_set_channel_width(int channel_width);
  // apply the frequency (wifi channel) and channel with for all wifibroadcast cards
  // r.n uses both iw and modifies the radiotap header
  bool apply_frequency_and_channel_width(uint32_t frequency, uint32_t channel_width);
  bool apply_frequency_and_channel_width_from_settings();
  // ------------- tx power is a bit confusing due to the difference(s) between HW
  bool set_tx_power_mw(int tx_power_mw);
  bool set_tx_power_rtl8812au(int tx_power_index_override);
  // set the tx power of all wifibroadcast cards. For rtl8812au, uses the tx power index
  // for other cards, uses the mW value
  void apply_txpower();
  // change the MCS index (only supported by rtl8812au)
  // guaranteed to return immediately (Doesn't need iw or something similar)
  // If the hw supports changing the mcs index, and the mcs index is valid, apply it and return true
  // Leave untouched and return false otherwise.
  bool set_mcs_index(int mcs_index);
  // this is special, mcs index can not only be changed via mavlink param, but also via RC channel (if enabled)
  void set_mcs_index_from_rc_channel(const std::array<int,18>& rc_channels);
  // special, change tx power depending on if the FC is armed / disarmed
  void update_arming_state(bool armed);
  // These do not "break" the bidirectional connectivity and therefore
  // can be changed easily on the fly
  bool set_video_fec_percentage(int fec_percentage);
  bool set_enable_wb_video_variable_bitrate(int value);
  bool set_max_fec_block_size_for_platform(int value);
  bool set_wb_video_rate_for_mcs_adjustment_percent(int value);
  // Make sure no processes interfering with monitor mode run on the given cards,
  // then sets them to monitor mode
  void takeover_cards_monitor_mode();
  // set the right frequency, channel width and tx power. Cards need to be in monitor mode already !
  void set_freq_width_power();
  // Reads the current settings and creates the appropriate Radiotap Header params
  [[nodiscard]] RadiotapHeader::UserSelectableParams create_radiotap_params()const;
  std::unique_ptr<WBStreamTx> create_wb_tx(uint8_t radio_port,bool is_video);
  std::unique_ptr<WBStreamRx> create_wb_rx(uint8_t radio_port,bool is_video,WBStreamRx::OUTPUT_DATA_CALLBACK cb);
 private:
  // Recalculate stats, apply settings asynchronously and more
  void loop_do_work();
  // update statistics, done in regular intervals, updated data is given to the ohd_telemetry module via the action handler
  void update_statistics();
  static constexpr auto RECALCULATE_STATISTICS_INTERVAL=std::chrono::milliseconds(100);
  std::chrono::steady_clock::time_point m_last_stats_recalculation=std::chrono::steady_clock::now();
  // Do rate adjustments, does nothing if variable bitrate is disabled
  void perform_rate_adjustment();
  void schedule_work_item(const std::shared_ptr<WorkItem>& work_item);
  // We limit changing specific params to one after another
  bool check_work_queue_empty();
  static constexpr auto DELAY_FOR_TRANSMIT_ACK =std::chrono::seconds(2);
 private:
  // Called by telemetry on both air and ground (send to opposite, respective)
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
    uint32_t channel_width=0;
  };
  // How often per second we broadcast the session key -
  // we send the session key ~2 times per second
  static constexpr std::chrono::milliseconds SESSION_KEY_PACKETS_INTERVAL=std::chrono::milliseconds(500);
  // Testing shows we have to listen for a while to reliably get data - since
  // 1) the card might take some time switching and
  // 2) we might lose session key packet(s)
  static constexpr std::chrono::seconds DEFAULT_SCAN_TIME_PER_CHANNEL{3};
  // This is a long-running operation during which changing things like frequency and more are disabled.
  // Loop through all possible frequencies + optionally channel widths until we can say with a high certainty
  // we have found a running air unit on this channel. (-> only supported on ground).
  // On success, apply this frequency.
  // On failure, restore previous state.
  ScanResult scan_channels(const openhd::ActionHandler::ScanChannelsParam& scan_channels_params);
  // queue it up on the work queue
  void async_scan_channels(openhd::ActionHandler::ScanChannelsParam scan_channels_params);
 private:
  void reset_all_rx_stats();
  int get_last_rx_packet_chan_width();
  int64_t get_total_dropped_packets();
 private:
  // We return false on all the change settings request(s) if there is already a change operation queued
  // up, or we currently perform a channel scan
  // Not completely "thread safe" so to say but good enough.
  bool check_in_state_support_changing_settings();
 private:
  const OHDProfile m_profile;
  const OHDPlatform m_platform;
  const std::vector<WiFiCard> m_broadcast_cards;
  // disable all openhd frequency checking - note that I am quite sure about the correctness of openhd internal checking in regards to wifi channels ;)
  const bool m_disable_all_frequency_checks;
  std::shared_ptr<openhd::ActionHandler> m_opt_action_handler=nullptr;
  std::shared_ptr<spdlog::logger> m_console;
  std::unique_ptr<openhd::WBStreamsSettingsHolder> m_settings;
  std::shared_ptr<WBTxRx> m_wb_txrx;
  // For telemetry, bidirectional in opposite directions
  std::unique_ptr<WBStreamTx> m_wb_tele_tx;
  std::unique_ptr<WBStreamRx> m_wb_tele_rx;
  // For video, on air there are only tx instances, on ground there are only rx instances.
  std::vector<std::unique_ptr<WBStreamTx>> m_wb_video_tx_list;
  std::vector<std::unique_ptr<WBStreamRx>> m_wb_video_rx_list;
  //std::unique_ptr<ForeignPacketsReceiver> m_foreign_packets_receiver;
  std::atomic<bool> is_scanning=false;
  // We have one worker thread for asynchronously performing operation(s) like changing the frequency
  // but also recalculating statistics that are then forwarded to openhd_telemetry for broadcast
  bool m_work_thread_run;
  std::unique_ptr<std::thread> m_work_thread;
  std::mutex m_work_item_queue_mutex;
  std::queue<std::shared_ptr<WorkItem>> m_work_item_queue;
  // These are for variable bitrate / tx error reduces bitrate
  static constexpr auto RATE_ADJUSTMENT_INTERVAL=std::chrono::seconds(1);
  std::chrono::steady_clock::time_point m_last_rate_adjustment=std::chrono::steady_clock::now();
  int64_t m_last_total_tx_error_count=0;
  int m_n_detected_and_reset_tx_errors=0;
  uint32_t m_max_video_rate_for_current_wifi_config =0;
  uint32_t m_recommended_video_bitrate=0;
  // Set to true when armed, disarmed by default
  // Used to differentiate between different tx power levels when armed / disarmed
  bool m_is_armed= false;
  // if a card does not support injection, we log a error message any injecting method is called
  std::chrono::steady_clock::time_point m_last_log_card_does_might_not_inject=std::chrono::steady_clock::now();
  static constexpr auto WARN_CARD_DOES_NOT_INJECT_INTERVAL=std::chrono::seconds(5);
};

#endif
