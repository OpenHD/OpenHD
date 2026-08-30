/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#ifndef STREAMS_H
#define STREAMS_H

#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <unordered_map>
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
 * mode & are injection capable. Needs to be at least one card. An Air unit can
 * additionally use a second Devourer card as an adaptive-channel scout. The
 * given cards need to support monitor mode and
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
  void set_fatal_error_callback(std::function<void()> callback);
  // Reopen the radio interfaces without replacing this WBLink. This preserves
  // all settings callbacks registered by the telemetry component.
  bool restart_after_card_replug(std::vector<WiFiCard> broadcast_cards);
  [[nodiscard]] bool is_radio_available() const {
    return m_radio_available.load();
  }
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
  bool request_set_tx_power_mw(int card_idx, int new_tx_power_mw, bool armed);
  bool request_set_tx_power_rtl8812au(int card_idx, int tx_power_index_override,
                                      bool armed);
  bool request_set_tx_power_level(int level);
  // MCS index can be changed on air (video downlink) and on ground (uplink).
  bool request_set_air_mcs_index(int mcs_index);
  bool request_set_ground_mcs_index(int mcs_index);

 private:
  // These do not "break" the bidirectional connectivity and therefore
  // can be changed easily on the fly
  bool set_air_video_fec_percentage(int fec_percentage);
  bool set_air_enable_wb_video_variable_bitrate(int value);
  bool set_air_max_fec_block_size_for_platform(int value);
  bool set_air_wb_video_rate_for_mcs_adjustment_percent(int value);
  bool apply_radio_settings(
      openhd::LinkActionHandler::RadioSettingsParam radio_settings);
  bool set_dev_air_set_high_retransmit_count(int value);
  bool request_set_ground_rx_channel_width(int channel_width);
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
  void wt_air_perform_frequency_retry();
  void wt_air_perform_adaptive_channel_selection();
  void wt_air_check_adaptive_channel_switch();
  void wt_manage_devourer_fhss();
  [[nodiscard]] std::vector<int> devourer_fhss_channels() const;
  [[nodiscard]] bool adaptive_channel_supported() const;
  void reset_adaptive_channel_selection();
  void wt_gnd_perform_channel_switch_rollback_check();
  void gnd_note_channel_switch_attempt(int previous_frequency,
                                       int previous_channel_width,
                                       int new_frequency,
                                       int new_channel_width,
                                       uint32_t transaction_id = 0);
  // this is special, mcs index can not only be changed via mavlink param, but
  // also via RC channel (if enabled)
  void wt_perform_mcs_via_rc_channel_if_enabled();
  void wt_perform_bw_via_rc_channel_if_enabled();
  void wt_perform_tx_mode_via_rc_channel_if_enabled();
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
  size_t calculate_history_size_from_ms(int window_ms,
                                        int packets_per_second) const;
  void apply_retransmission_history_window(int window_ms_video,
                                           int window_ms_telemetry,
                                           int window_ms_rc);
  [[nodiscard]] int get_configured_tx_mcs_index() const;
  // set passive mode to disabled (do not drop packets) unless we are ground
  // and passive mode is enabled by the user
  void re_enable_injection_unless_user_passive_mode_enabled();
  int get_max_fec_block_size();
  // Called when the wifi card (really really likely) disconneccted
  void on_wifi_card_fatal_error();

 private:
  const OHDProfile m_profile;
  // Direct-USB Devourer identities contain the transient USB bus/address and
  // must be refreshed after a device re-enumerates.
  std::vector<WiFiCard> m_broadcast_cards;
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
  std::atomic_bool m_work_thread_run = false;
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
  std::atomic<int> m_last_announced_bitrate_kbits = -1;
  std::atomic<int> m_curr_n_rate_adjustments = 0;
  // Set to true when armed, disarmed by default
  // Used to differentiate between different tx power levels when armed /
  // disarmed
  bool m_is_armed = false;
  std::atomic_bool m_request_apply_tx_power = false;
  std::atomic_bool m_request_apply_tx_mcs_index = false;
  std::atomic<int> m_pending_rc_channel_width = 0;
  std::atomic<int> m_rc_tx_mode_override = -1;
  std::chrono::steady_clock::time_point m_last_log_key_mismatch =
      std::chrono::steady_clock::now();
  // We store tx power for easy access in stats
  std::atomic<int> m_curr_tx_power_idx = 0;
  std::atomic<int> m_curr_tx_power_mw = 0;
  std::atomic<int> m_last_received_packet_ts_ms =
      openhd::util::steady_clock_time_epoch_ms();
  std::atomic<bool> m_logged_missing_video_crypto = false;
  bool m_video_crypto_available = false;
  std::chrono::steady_clock::time_point m_reset_frequency_time_point =
      std::chrono::steady_clock::now();
  // 40Mhz / 20Mhz link management
  std::unique_ptr<ManagementAir> m_management_air = nullptr;
  std::unique_ptr<ManagementGround> m_management_gnd = nullptr;
  // We start on 20Mhz, and only go up to 40Mhz if requested/available
  std::atomic<int> m_gnd_curr_rx_channel_width =
      openhd::DEFAULT_GND_RX_CHANNEL_WIDTH;
  std::atomic<int> m_gnd_curr_rx_frequency = -1;
  // QOpenHD arms this target on ground after sending the request to air.  It
  // lets ground follow when all five air RECEIVED packets are lost and the old
  // link consequently disappears.
  std::atomic<int> m_gnd_pending_frequency = -1;
  std::atomic<int> m_gnd_pending_channel_width =
      openhd::DEFAULT_GND_RX_CHANNEL_WIDTH;
  std::atomic<int> m_gnd_pending_frequency_since_ms = 0;
  std::atomic<int> m_gnd_last_no_received_warning_ms = 0;
  uint32_t m_air_last_frequency_retry_transaction = 0;
  struct AdaptiveChannelEvidence {
    uint64_t false_alarm_average = 0;
    uint32_t samples = 0;
  };
  std::mutex m_adaptive_channel_mutex;
  std::unordered_map<int, AdaptiveChannelEvidence> m_adaptive_channel_evidence;
  std::vector<int> m_adaptive_channel_candidates;
  size_t m_adaptive_channel_candidate_index = 0;
  int m_adaptive_recommended_frequency = -1;
  uint8_t m_adaptive_recommendation_streak = 0;
  std::chrono::steady_clock::time_point m_adaptive_last_sample_tp =
      std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point m_adaptive_last_switch_tp =
      std::chrono::steady_clock::now() - std::chrono::minutes(3);
  struct AdaptiveAirSwitchState {
    bool active = false;
    int previous_frequency = -1;
    int attempted_frequency = -1;
    int64_t baseline_count_p_valid = 0;
    std::chrono::steady_clock::time_point switch_tp =
        std::chrono::steady_clock::now();
  };
  AdaptiveAirSwitchState m_adaptive_air_switch_state{};
  std::atomic<int> m_adaptive_pending_target = -1;
  std::atomic<int> m_adaptive_pending_previous = -1;
  std::array<uint8_t, 16> m_devourer_fhss_key{};
  bool m_devourer_fhss_running = false;
  bool m_devourer_fhss_wait_logged = false;
  struct GroundSwitchRollbackState {
    bool active = false;
    int previous_frequency = -1;
    int previous_channel_width = openhd::DEFAULT_GND_RX_CHANNEL_WIDTH;
    int attempted_frequency = -1;
    int attempted_channel_width = -1;
    uint32_t transaction_id = 0;
    int64_t baseline_count_p_valid = 0;
    std::chrono::steady_clock::time_point switch_tp =
        std::chrono::steady_clock::now();
  };
  GroundSwitchRollbackState m_gnd_switch_rollback_state{};
  uint32_t m_gnd_last_prepared_frequency_transaction = 0;
  uint32_t m_gnd_last_committed_frequency_transaction = 0;
  static constexpr auto FREQUENCY_RECEIVED_BURST_TIMEOUT =
      std::chrono::milliseconds(1000);
  static constexpr auto GND_SWITCH_ROLLBACK_TIMEOUT =
      std::chrono::milliseconds(7000);
  static constexpr auto ADAPTIVE_SAMPLE_INTERVAL =
      std::chrono::milliseconds(3000);
  static constexpr auto ADAPTIVE_SAMPLE_DWELL =
      std::chrono::milliseconds(300);
  static constexpr auto ADAPTIVE_SWITCH_COOLDOWN =
      std::chrono::minutes(3);
  // Allows temporarily closing the video input
  std::atomic_bool m_air_close_video_in = false;
  const int m_recommended_max_fec_blk_size_for_this_platform;
  std::atomic_bool m_wifi_card_error_has_been_handled = false;
  std::atomic_bool m_radio_available = true;
  std::mutex m_radio_restart_mutex;
  std::mutex m_fatal_error_callback_mutex;
  std::function<void()> m_fatal_error_callback;
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
