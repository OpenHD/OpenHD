#ifndef STREAMS_H
#define STREAMS_H

#include <array>
#include <chrono>
#include <optional>
#include <utility>
#include <vector>

#include "../lib/wifibroadcast/wifibroadcast/src/WBStreamRx.h"
#include "../lib/wifibroadcast/wifibroadcast/src/WBStreamTx.h"
#include "../lib/wifibroadcast/wifibroadcast/src/WBTxRx.h"
#include "../lib/wifibroadcast/wifibroadcast/src/encryption/EncryptionFsUtils.h"
#include "openhd_action_handler.h"
#include "openhd_link.hpp"
#include "openhd_link_statistics.hpp"
#include "openhd_platform.h"
#include "openhd_profile.h"
#include "openhd_settings_imp.h"
#include "openhd_spdlog.h"
#include "openhd_util_time.h"
#include "wb_link_helper.h"
#include "wb_link_manager.h"
#include "wb_link_settings.h"
#include "wb_link_work_item.hpp"
#include "wifi_card.h"

/**
 * This class takes a list of cards supporting monitor mode (only 1 card on air)
 * and is responsible for configuring the given cards and then setting up all
 * the Wifi-broadcast streams needed for OpenHD. In the end, we have a link that
 * has some broadcast characteristics for video (video is always broadcast from
 * air to ground) but also a bidirectional link (without re-transmission(s)) for
 * telemetry. This class assumes a corresponding instance on the air or ground
 * unit, respective.
 */
class WBLink : public OHDLink {
 public:
  /**
   * @param broadcast_cards list of discovered wifi card(s) that support monitor
   * mode & are injection capable. Needs to be at least one card, and only one
   * card on an air unit. The given cards need to support monitor mode and
   * either 2.4G or 5G wifi. In the case where there are multiple card(s), the
   * first given card is used for transmission & receive, the other card(s) are
   * not used for transmission, only for receiving.
   * @param opt_action_handler global openhd action handler, optional (can be
   * nullptr during testing of specific modules instead of testing a complete
   * running openhd instance)
   */
  explicit WBLink(OHDProfile profile, std::vector<WiFiCard> broadcast_cards);
  WBLink(const WBLink&) = delete;
  WBLink(const WBLink&&) = delete;
  ~WBLink();
  /**
   * @return all mavlink settings, values might change depending on air/ground
   * and/or the used hardware
   */
  std::vector<openhd::Setting> get_all_settings();
  /**
   * Used by wifi hotspot feature (opposite wifi space if possible)
   * @return the current wb channel space
   */
  [[nodiscard]] openhd::WifiSpace get_current_frequency_channel_space() const;

 private:
  // NOTE:
  // For everything prefixed with 'request_', we validate the param (since it
  // comes from mavlink and might be unsafe to apply) And return false if it is
  // an invalid param (e.g. an unsupported frequency by the card). We then
  // return true if we can enqueue this change operation to be applied on the
  // worker thread (false otherwise). This way we have the nice feature that we
  // 1) reject settings while the worker thread is busy (e.g. during a channel
  // scan) or if a previous change (like tx power) is still being performed. In
  // this case, the user can just try again later (and should not be able to
  // change the frequency for example during a channel scan anyway). 2) can send
  // the mavlink ack immediately, instead of needing to wait for the action to
  // be performed (Changing the tx power for example can take some time, while
  // the OS is busy talking to the wifi driver). Only disadvantage: We need to
  // be able to reason about weather the given change will be successfully or
  // not beforehand.
  bool request_set_frequency(int frequency);
  // Channel width / bandwidth is local to the air, and can be changed without
  // synchronization due to 20Mhz management packets
  bool request_set_air_tx_channel_width(int channel_width);
  // TX power can be set for both air / ground independently.
  bool request_set_tx_power_mw(int new_tx_power_mw, bool armed);
  bool request_set_tx_power_rtl8812au(int tx_power_index_override, bool armed);
  // MCS index can be changed on air (user can control the rate with it).
  bool request_set_air_mcs_index(int mcs_index);
  // These do not "break" the bidirectional connectivity and therefore
  // can be changed easily on the fly
  bool set_air_video_fec_percentage(int fec_percentage);
  bool set_air_enable_wb_video_variable_bitrate(int value);
  bool set_air_max_fec_block_size_for_platform(int value);
  bool set_air_wb_video_rate_for_mcs_adjustment_percent(int value);
  bool set_dev_air_set_high_retransmit_count(int value);
  // Initiate channel scan / channel analyze.
  // Those operations run asynchronous until completed, and during this time
  // all other "request_" setting changes are rejected (since the work thread
  // does the long-running async operation)
  bool request_start_scan_channels(
      openhd::LinkActionHandler::ScanChannelsParam scan_channels_params);
  bool request_start_analyze_channels(int channels_to_scan);

  // apply the frequency (wifi channel) and channel with for all wifibroadcast
  // cards r.n uses both iw and modifies the radiotap header
  bool apply_frequency_and_channel_width(int frequency, int channel_width_rx,
                                         int channel_width_tx);
  bool apply_frequency_and_channel_width_from_settings();
  // set the tx power of all wb cards. For rtl8812au, uses the tx power index
  // for other cards, uses the mW value
  void apply_txpower();
  /**
   * Every time the arming state is updated, we just set a flag here such that
   * the main thread updates the tx power
   */
  void update_arming_state(bool armed);
  // Recalculate stats, apply settings asynchronously and more
  void loop_do_work();
  // update statistics, done in regular intervals, updated data is given to the
  // ohd_telemetry module via the action handler
  void wt_update_statistics();
  // Do rate adjustments, does nothing if variable bitrate is disabled
  void wt_perform_rate_adjustment();
  void wt_gnd_perform_channel_management();
  // this is special, mcs index can not only be changed via mavlink param, but
  // also via RC channel (if enabled)
  void wt_perform_mcs_via_rc_channel_if_enabled();
  void wt_perform_bw_via_rc_channel_if_enabled();
  // Time out to go from wifibroadcast mode to wifi hotspot mode
  void wt_perform_air_hotspot_after_timeout();
  // X20 only, thermal protection
  void wt_perform_update_thermal_protection();
  // Returns true if the work item queue is currently empty and the item has
  // been added false otherwise. In general, we only suport one item on the work
  // queue - otherwise we reject the param, since the user can just try again
  // later (and in case the work queue is currently busy with a frequency scan
  // for example, we do not support changing the frequency or similar.
  bool try_schedule_work_item(const std::shared_ptr<WorkItem>& work_item);
  // Called by telemetry on both air and ground (send to opposite, respective)
  void transmit_telemetry_data(TelemetryTxPacket packet) override;
  // Called by the camera stream on the air unit only
  // transmit video data via wifibradcast
  void transmit_video_data(
      int stream_index,
      const openhd::FragmentedVideoFrame& fragmented_video_frame) override;
  void transmit_audio_data(const openhd::AudioPacket& audio_packet) override;
  // How often per second we broadcast the session key -
  // we send the session key ~2 times per second
  static constexpr std::chrono::milliseconds SESSION_KEY_PACKETS_INTERVAL =
      std::chrono::milliseconds(500);
  // This is a long-running operation during which changing things like
  // frequency and more are disabled. Tries to find a running air unit and goes
  // to this frequency if found. continuously broadcasts progress via mavlink.
  void perform_channel_scan(
      const openhd::LinkActionHandler::ScanChannelsParam& scan_channels_params);
  // similar to channel scan, analyze channel(s) for interference
  void perform_channel_analyze(int channels_to_scan);
  void reset_all_rx_stats();
  void recommend_bitrate_to_encoder(int recommended_video_bitrate_kbits);
  // set passive mode to disabled (do not drop packets) unless we are ground
  // and passive mode is enabled by the user
  void re_enable_injection_unless_user_passive_mode_enabled();
  int get_max_fec_block_size();
  // Called when the wifi card (really really likely) disconneccted
  void on_wifi_card_fatal_error();

 private:
  const OHDProfile m_profile;
  const std::vector<WiFiCard> m_broadcast_cards;
  std::shared_ptr<spdlog::logger> m_console;
  std::unique_ptr<openhd::WBLinkSettingsHolder> m_settings;
  std::shared_ptr<RadiotapHeaderTxHolder> m_tx_header_1;
  // On air, we use different radiotap data header(s) for different streams
  // (20Mhz vs 40Mhz)
  std::shared_ptr<RadiotapHeaderTxHolder> m_tx_header_2;
  std::shared_ptr<WBTxRx> m_wb_txrx;
  // For telemetry, bidirectional in opposite directions
  std::unique_ptr<WBStreamTx> m_wb_tele_tx;
  std::unique_ptr<WBStreamRx> m_wb_tele_rx;
  // For video, on air there are only tx instances, on ground there are only rx
  // instances.
  std::vector<std::unique_ptr<WBStreamTx>> m_wb_video_tx_list;
  std::vector<std::unique_ptr<WBStreamRx>> m_wb_video_rx_list;
  // For audio or custom data
  std::unique_ptr<WBStreamTx> m_wb_audio_tx;
  std::unique_ptr<WBStreamRx> m_wb_audio_rx;
  // We have one worker thread for asynchronously performing operation(s) like
  // changing the frequency but also recalculating statistics that are then
  // forwarded to openhd_telemetry for broadcast
  bool m_work_thread_run;
  std::unique_ptr<std::thread> m_work_thread;
  std::mutex m_work_item_queue_mutex;
  // NOTE: We only support one active work item at a time,
  // otherwise, we reject any changes requested by the user.
  std::queue<std::shared_ptr<WorkItem>> m_work_item_queue;
  static constexpr auto RECALCULATE_STATISTICS_INTERVAL =
      std::chrono::milliseconds(500);
  std::chrono::steady_clock::time_point m_last_stats_recalculation =
      std::chrono::steady_clock::now();
  std::atomic<int> m_max_total_rate_for_current_wifi_config_kbits = 0;
  std::atomic<int> m_max_video_rate_for_current_wifi_fec_config = 0;
  // Whenever the frequency has been changed, we reset tx errors and start new
  bool m_rate_adjustment_frequency_changed = false;
  // bitrate we recommend to the encoder / camera(s)
  int m_recommended_video_bitrate_kbits = 0;
  std::atomic<int> m_curr_n_rate_adjustments = 0;
  // Set to true when armed, disarmed by default
  // Used to differentiate between different tx power levels when armed /
  // disarmed
  bool m_is_armed = false;
  std::atomic_bool m_request_apply_tx_power = false;
  std::atomic_bool m_request_apply_air_mcs_index = false;
  std::atomic_bool m_request_apply_air_bw = false;
  std::chrono::steady_clock::time_point m_last_log_bind_phrase_mismatch =
      std::chrono::steady_clock::now();
  // We store tx power for easy access in stats
  std::atomic<int> m_curr_tx_power_idx = 0;
  std::atomic<int> m_curr_tx_power_mw = 0;
  std::atomic<int> m_last_received_packet_ts_ms =
      openhd::util::steady_clock_time_epoch_ms();
  std::chrono::steady_clock::time_point m_reset_frequency_time_point =
      std::chrono::steady_clock::now();
  // 40Mhz / 20Mhz link management
  std::unique_ptr<ManagementAir> m_management_air = nullptr;
  std::unique_ptr<ManagementGround> m_management_gnd = nullptr;
  // We start on 40Mhz, and go down to 20Mhz if possible
  std::atomic<int> m_gnd_curr_rx_channel_width = 40;
  std::atomic<int> m_gnd_curr_rx_frequency = -1;
  // Allows temporarily closing the video input
  std::atomic_bool m_air_close_video_in = false;
  const int m_recommended_max_fec_blk_size_for_this_platform;
  bool m_wifi_card_error_has_been_handled = false;
  // We have 3 thermal protection levels - as of now, only on X20
  static constexpr uint8_t THERMAL_PROTECTION_NONE = 0;
  static constexpr uint8_t THERMAL_PROTECTION_RATE_REDUCED = 1;
  static constexpr uint8_t THERMAL_PROTECTION_VIDEO_DISABLED = 2;
  std::atomic_uint8_t m_thermal_protection_level = 0;
  std::chrono::steady_clock::time_point m_thermal_protection_enable_tp =
      std::chrono::steady_clock::now();

 private:
  openhd::wb::ForeignPacketsHelper m_foreign_p_helper;
  openhd::wb::RCChannelHelper m_rc_channel_helper;
  openhd::wb::FrameDropsHelper m_frame_drop_helper;
  std::atomic_int m_primary_total_dropped_frames = 0;
  std::atomic_int m_secondary_total_dropped_frames = 0;

 private:
  const bool DIRTY_forward_gapped_fragments = false;
  const bool DIRTY_add_aud_nal = false;
  const int DIRTY_emulate_drop_mode = 0;

 private:
  const std::chrono::steady_clock::time_point m_wb_link_start_ts =
      std::chrono::steady_clock::now();
  std::optional<std::chrono::steady_clock::time_point> m_hs_timeout =
      std::chrono::steady_clock::now();
};

#endif
