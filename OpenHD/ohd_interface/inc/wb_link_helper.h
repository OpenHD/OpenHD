//
// Created by consti10 on 10.12.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_HELPER_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_HELPER_H_

#include <mutex>
#include <optional>
#include <utility>

#include "TimeHelper.hpp"
#include "openhd_spdlog.h"
#include "wb_link_settings.h"

/**
 * The wb_link class is becoming a bit big and therefore hard to read.
 * Here we have some common helper methods used in wb_link
 */
namespace openhd::wb {

/**
 * @return true if the "disable all frequency checks" file exists
 */
bool disable_all_frequency_checks();

/**
 * returns true if all the given cards supports the given frequency
 */
bool all_cards_support_frequency(
    uint32_t frequency, const std::vector<WiFiCard>& m_broadcast_cards,
    const std::shared_ptr<spdlog::logger>& m_console);

bool all_cards_support_frequency_and_channel_width(
    uint32_t frequency, uint32_t channel_width,
    const std::vector<WiFiCard>& m_broadcast_cards,
    const std::shared_ptr<spdlog::logger>& m_console);

bool validate_frequency_change(
    int new_frequency, int current_channel_width,
    const std::vector<WiFiCard>& m_broadcast_cards,
    const std::shared_ptr<spdlog::logger>& m_console);
bool validate_air_channel_width_change(
    int new_channel_width, const WiFiCard& card,
    const std::shared_ptr<spdlog::logger>& m_console);

bool validate_air_mcs_index_change(
    int new_mcs_index, const WiFiCard& card,
    const std::shared_ptr<spdlog::logger>& m_console);

bool any_card_support_frequency(
    uint32_t frequency, const std::vector<WiFiCard>& m_broadcast_cards,
    const std::shared_ptr<spdlog::logger>& m_console);

bool set_frequency_and_channel_width_for_all_cards(
    uint32_t frequency, uint32_t channel_width,
    const std::vector<WiFiCard>& m_broadcast_cards);

void set_tx_power_for_all_cards(int tx_power_mw,
                                int rtl8812au_tx_power_index_override,
                                const std::vector<WiFiCard>& m_broadcast_cards);

// WB takes a list of card device names
std::vector<std::string> get_card_names(const std::vector<WiFiCard>& cards);

// Returns true if any of the given cards is of type rtl8812au
bool has_any_rtl8812au(const std::vector<WiFiCard>& cards);
// Returns true if any of the given cards is not of type rtl8812au
bool has_any_non_rtl8812au(const std::vector<WiFiCard>& cards);

bool any_card_supports_stbc_ldpc_sgi(const std::vector<WiFiCard>& cards);

std::vector<WifiChannel> get_scan_channels_frequencies(const WiFiCard& card,
                                                       int channels_to_scan);
std::vector<WifiChannel> get_analyze_channels_frequencies(const WiFiCard& card,
                                                          int channels_to_scan);

/*
 * Removes network manager from the given cards (if it is running)
 * and in general tries to make sure no linux stuff that would interfer with
 * monitor mode is running on the card(s), and then sets them into monitor mode.
 */
void takeover_cards_monitor_mode(const std::vector<WiFiCard>& cards,
                                 std::shared_ptr<spdlog::logger> console);
/**
 * Gives the card(s) back to network manager;
 */
void giveback_cards_monitor_mode(const std::vector<WiFiCard>& cards,
                                 std::shared_ptr<spdlog::logger> console);

int calculate_bitrate_for_wifi_config_kbits(
    const WiFiCard& card, int frequency_mhz, int channel_width_mhz,
    int mcs_index, int dev_adjustment_percent, bool debug_log);

class ForeignPacketsHelper {
 public:
  void update(uint64_t count_p_any, uint64_t count_p_valid) {
    const uint64_t n_foreign_packets =
        count_p_any > count_p_valid ? count_p_any - count_p_valid : 0;
    if (m_foreign_packets_last_time > n_foreign_packets) {
      m_foreign_packets_last_time = n_foreign_packets;
      return;
    }
    const int delta =
        static_cast<int>(n_foreign_packets - m_foreign_packets_last_time);
    m_foreign_packets_last_time = n_foreign_packets;
    update_n_foreign_packets(delta);
  }
  int get_foreign_packets_per_second() const { return m_pps_current; }
  void update_n_foreign_packets(int n_foreign_packets) {
    assert(n_foreign_packets >= 0);
    // openhd::log::get_default()->debug("N foreign
    // packets:{}",n_foreign_packets);
    m_pps_foreign_packets_count += n_foreign_packets;
    const auto elapsed =
        std::chrono::steady_clock::now() - m_pps_last_recalculation;
    if (elapsed > std::chrono::seconds(1)) {
      m_pps_last_recalculation = std::chrono::steady_clock::now();
      if (m_pps_foreign_packets_count <= 0) {
        m_pps_current = 0;
        return;
      }
      const int elapsed_us = static_cast<int>(
          std::chrono::duration_cast<std::chrono::microseconds>(elapsed)
              .count());
      m_pps_current = m_pps_foreign_packets_count * 1000 * 1000 / elapsed_us;
      m_pps_foreign_packets_count = 0;
    }
  }

 private:
  uint64_t m_foreign_packets_last_time = 0;
  int m_pps_foreign_packets_count = 0;
  std::chrono::steady_clock::time_point m_pps_last_recalculation =
      std::chrono::steady_clock::now();
  int m_pps_current = -1;
};

/**
 * This class basically only offers atomic read / write operations on the "RC
 * CHANNELS" as reported by the FC. This is needed for the "MCS VIA RC CHANNEL
 * CHANGE" feature.
 */
class RCChannelHelper {
 public:
  // Atomic / thread-safe setter / getter
  void set_rc_channels(const std::array<int, 18>& rc_channels);
  std::optional<std::array<int, 18>> get_fc_reported_rc_channels();
  /**
   * Get the mcs index mapping pwm channel (channel_index) to mcs indices.
   * If no rc data has been supplied by the FC yet and / or the channel index is
   * invalid or the pwm value is not valid, return std::nullopt.
   */
  std::optional<int> get_mcs_from_rc_channel(
      int channel_index, std::shared_ptr<spdlog::logger>& m_console);
  // returns either a valid channel width (20 /40) or std::nullopt
  std::optional<uint8_t> get_bw_from_rc_channel(int channel_index);

 private:
  std::optional<std::array<int, 18>> m_rc_channels;
  std::mutex m_rc_channels_mutex;
};

class FrameDropsHelper {
 public:
  // Thread-safe, aka can be called from the thread injecting frame(s) in
  // reference to the wb_link worker thread
  void notify_dropped_frame(int n_dropped = 1) {
    m_frame_drop_counter += n_dropped;
  }
  // Thread-safe as long as it is called from the thread performing management
  bool needs_bitrate_reduction() {
    if (m_opt_no_error_delay.has_value()) {
      if (std::chrono::steady_clock::now() >= m_opt_no_error_delay) {
        const auto elapsed = std::chrono::steady_clock::now() - m_last_check;
        m_last_check = std::chrono::steady_clock::now();
        const int dropped_since_last_check = m_frame_drop_counter.exchange(0);
        m_console->debug(
            "Dropped {} frames in {} during adjust period (no bitrate "
            "reduction)",
            dropped_since_last_check, MyTimeHelper::R(elapsed));
        m_opt_no_error_delay = std::nullopt;
      }
      return false;
    }
    const auto elapsed = std::chrono::steady_clock::now() - m_last_check;
    if (elapsed >= std::chrono::seconds(3)) {
      m_last_check = std::chrono::steady_clock::now();
      const int dropped_since_last_check = m_frame_drop_counter.exchange(0);
      static constexpr int MAX_DROPPED_FRAMES_ALLOWED = 3;
      if (dropped_since_last_check > MAX_DROPPED_FRAMES_ALLOWED) {
        m_console->debug("Dropped {} frames during {} delta period",
                         dropped_since_last_check, MyTimeHelper::R(elapsed));
        return true;
      }
    }
    return false;
  }
  void set_console(std::shared_ptr<spdlog::logger> console) {
    m_console = std::move(console);
  }
  // Every time we change the bitrate, it might take some time until the camera
  // reacts (TODO: Define a minumum allowed variance for openhd supported
  // cameras)
  // - this results in dropped frame(s) during this period not being reported as
  // an error (Such that we don't do any rate reduction while the encoder is
  // still reacting to the newly set bitrate)
  void delay_for(std::chrono::milliseconds delay) {
    m_opt_no_error_delay = std::chrono::steady_clock::now() + delay;
  }

 private:
  std::shared_ptr<spdlog::logger> m_console;
  std::chrono::steady_clock::time_point m_last_check =
      std::chrono::steady_clock::now();
  std::atomic_int m_frame_drop_counter = 0;
  std::optional<std::chrono::steady_clock::time_point> m_opt_no_error_delay =
      std::nullopt;
};

class PollutionHelper {
 public:
 private:
};

}  // namespace openhd::wb

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_HELPER_H_
