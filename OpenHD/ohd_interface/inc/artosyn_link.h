#ifndef OPENHD_ARTOSYN_LINK_H
#define OPENHD_ARTOSYN_LINK_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "openhd_link.hpp"
#include "openhd_settings_imp.h"
#include "openhd_spdlog.h"
#include "artosyn_link_settings.h"
#include "non_wb_video_bitrate_meter.h"

// SDK types used by helper methods.
extern "C" {
#include "bb_api.h"
}

/**
 * Artosyn 8030 link implementation using the vendor host SDK.
 *
 * This is a minimal, one-way integration:
 * - Air: transmit telemetry + video
 * - Ground: receive telemetry + video
 *
 * Two-way support can be added later by wiring the opposite directions.
 */
class ArtosynLink : public OHDLink {
 public:
  explicit ArtosynLink(OHDProfile profile);
  ~ArtosynLink() override;

  void transmit_telemetry_data(TelemetryTxPacket packet) override;
  void transmit_video_data(
      int stream_index,
      const openhd::FragmentedVideoFrame& fragmented_video_frame) override;
  void transmit_audio_data(const openhd::AudioPacket& audio_packet) override;

  // Lightweight detection to decide whether to instantiate this link.
  static bool probe();

  std::vector<openhd::Setting> get_all_settings();

 public:
  struct Config {
    std::string addr = "127.0.0.1";
    int port = 50000;
    int slot = 0;
    int video_port = 2;
    int telemetry_port = 1;
    bool use_datagram = true;
    int rx_buf_size = 64 * 1024;
    int tx_buf_size = 64 * 1024;
    int read_timeout_ms = 100;
  };

 private:
  bool init_device();
  void shutdown_device();
  void try_open_sockets_if_ready();

  int open_socket(int port, bool want_tx, bool want_rx,
                  bool allow_legacy_fallback = true);
  void open_configured_sockets(bool allow_legacy_fallback,
                               const char* reason);
  void start_rx_threads();
  void stop_rx_threads();

  void rx_loop_video();
  void rx_loop_telemetry();
  void rx_loop_shared();
  int write_stream_packet(int fd, uint8_t stream_id, const uint8_t* data,
                          uint32_t size);
  void start_video_tx_thread();
  void stop_video_tx_thread();
  void video_tx_loop();
  void log_tx_error_throttled(const char* stream, int ret);

  void apply_link_settings();
  bool read_metrics(int* link_state, int* rx_mcs, int* tx_mcs, int* bw,
                    int* phy_tp_kbps, int* real_tp_kbps, int* tx_freq_khz,
                    int* rx_freq_khz, int* rx_bw, int* rx_phy_tp_kbps,
                    int* rx_real_tp_kbps);
  bool read_quality_metrics(int* snr, int* ldpc_err, int* ldpc_num,
                            int* gain_a, int* gain_b);
  bool read_power_metrics(int* power_auto, int* power_dbm);
  bool read_chan_metrics(int* chan_auto, int* work_chan,
                         int* work_freq_khz);
  bool read_band_metrics(int* band_auto, int* work_band);
  bool read_rf_metrics(int* a_tx, int* a_rx, int* b_tx, int* b_rx);
  bool read_sys_info(uint64_t* uptime_ms, std::string* soft_ver,
                     std::string* hw_ver, std::string* fw_ver,
                     std::string* compile_time);
  bool read_runsys(int* runsys_id);
  bool read_mcs_throughput(int* tx_tp_kbps, int* rx_tp_kbps);
  bool read_peer_quality(int* snr, int* ldpc_err, int* ldpc_num,
                         int* gain_a, int* gain_b);
  bool read_ap_time(int* ap_time_ms);
  bool read_1v1_info(bb_info_t* self, bb_info_t* peer);
  bool read_sock_info(int port, bb_sock_info_t* out_info);
  bool read_status_extra(int* role, int* mode, int* sync_mode,
                         int* sync_master, int* cfg_sbmp, int* rt_sbmp,
                         std::string* local_mac, int* pair_state,
                         std::string* peer_mac, int* tx_rf_mode,
                         int* rx_rf_mode, int* tx_tintlv_en,
                         int* rx_tintlv_en, int* tx_tintlv_num,
                         int* rx_tintlv_num, int* tx_tintlv_len,
                         int* rx_tintlv_len);

  void start_stats_thread();
  void stop_stats_thread();
  void stats_loop();
  void update_link_stats();
  void update_video_bitrate_recommendation(int capacity_kbits,
                                           int64_t now_ms);
  void start_connect_worker();
  void stop_connect_worker();
  void connect_loop();

 private:
  const OHDProfile m_profile;
  std::shared_ptr<spdlog::logger> m_console;
  Config m_cfg;

  std::atomic<bool> m_running{false};
  std::atomic<bool> m_stats_running{false};

  std::unique_ptr<openhd::ArtosynLinkSettingsHolder> m_settings;

  bb_host_t* m_host = nullptr;
  bb_dev_handle_t* m_dev = nullptr;

  int m_video_fd = -1;
  int m_telemetry_fd = -1;
  bool m_shared_socket = false;

  std::thread m_rx_video_thread;
  std::thread m_rx_telemetry_thread;
  std::thread m_video_tx_thread;
  std::thread m_stats_thread;
  std::thread m_connect_thread;
  std::mutex m_radio_write_mutex;
  std::mutex m_video_tx_mutex;
  std::condition_variable m_video_tx_cv;
  std::deque<std::pair<int, openhd::FragmentedVideoFrame>>
      m_pending_video_frames;
  bool m_stop_video_tx = false;
  bool m_video_tx_wait_for_idr = false;
  uint64_t m_video_tx_dropped_frames = 0;
  std::atomic<bool> m_stop_connect_worker{false};
  std::atomic<bool> m_legacy_init_retry_done{false};
  std::atomic<bool> m_bb_initialized_by_openhd{false};
  std::atomic<bool> m_bb_started_by_openhd{false};
  std::atomic<int> m_status_ioctl_fail_streak{0};
  std::atomic<int64_t> m_last_recover_request_ms{0};
  std::atomic<int64_t> m_next_socket_probe_ms{0};

  std::atomic<uint64_t> m_tx_total_bytes{0};
  std::atomic<uint64_t> m_tx_total_packets{0};
  std::atomic<uint64_t> m_rx_total_bytes{0};
  std::atomic<uint64_t> m_rx_total_packets{0};
  std::atomic<uint64_t> m_tx_tele_bytes{0};
  std::atomic<uint64_t> m_tx_tele_packets{0};
  std::atomic<uint64_t> m_rx_tele_bytes{0};
  std::atomic<uint64_t> m_rx_tele_packets{0};
  std::atomic<int64_t> m_last_rx_packet_ts_ms{0};
  std::atomic<int64_t> m_last_tx_error_log_ms{0};

  int64_t m_last_stats_ts_ms = 0;
  int m_bitrate_capacity_ema_kbits = 0;
  int m_last_recommended_bitrate_kbits = -1;
  int64_t m_last_bitrate_calculation_ms = 0;
  int64_t m_last_bitrate_announcement_ms = 0;
  uint64_t m_last_stats_tx_bytes = 0;
  uint64_t m_last_stats_tx_packets = 0;
  uint64_t m_last_stats_rx_bytes = 0;
  uint64_t m_last_stats_rx_packets = 0;
  uint64_t m_last_stats_tx_tele_bytes = 0;
  uint64_t m_last_stats_tx_tele_packets = 0;
  uint64_t m_last_stats_rx_tele_bytes = 0;
  uint64_t m_last_stats_rx_tele_packets = 0;
  openhd::non_wb::VideoBitrateMeter m_video_bitrate_meter;
};

#endif  // OPENHD_ARTOSYN_LINK_H
