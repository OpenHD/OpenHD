#include "artosyn_link.h"

#include <Poco/Exception.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Timespan.h>

#include <algorithm>
#include <chrono>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <sstream>
#include <utility>

extern "C" {
#include "ar8030.h"
#include "bb_api.h"
}

#include "openhd_action_handler.h"
#include "openhd_sock.h"
#include "openhd_settings_imp.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"
#include "openhd_util_time.h"
#include "wifi_card.h"

namespace {
ArtosynLink::Config config_from_settings(
    const openhd::ArtosynLinkSettings& s) {
  ArtosynLink::Config cfg;
  cfg.addr = s.addr;
  cfg.port = s.port;
  cfg.slot = s.slot;
  cfg.video_port = s.video_port;
  cfg.telemetry_port = s.telemetry_port;
  cfg.use_datagram = (s.use_datagram != 0);
  cfg.rx_buf_size = s.rx_buf_size;
  cfg.tx_buf_size = s.tx_buf_size;
  cfg.read_timeout_ms = s.read_timeout_ms;
  return cfg;
}

std::string mac_to_string(const bb_mac_t& mac) {
  std::ostringstream oss;
  oss << std::uppercase << std::hex << std::setfill('0')
      << std::setw(2) << static_cast<int>(mac.addr[0]) << ":"
      << std::setw(2) << static_cast<int>(mac.addr[1]) << ":"
      << std::setw(2) << static_cast<int>(mac.addr[2]) << ":"
      << std::setw(2) << static_cast<int>(mac.addr[3]);
  return oss.str();
}

int16_t clamp_int16(int value) {
  if (value > std::numeric_limits<int16_t>::max())
    return std::numeric_limits<int16_t>::max();
  if (value < std::numeric_limits<int16_t>::min())
    return std::numeric_limits<int16_t>::min();
  return static_cast<int16_t>(value);
}

int8_t clamp_int8(int value) {
  if (value > std::numeric_limits<int8_t>::max())
    return std::numeric_limits<int8_t>::max();
  if (value < std::numeric_limits<int8_t>::min())
    return std::numeric_limits<int8_t>::min();
  return static_cast<int8_t>(value);
}

int32_t clamp_int32(int64_t value) {
  if (value > std::numeric_limits<int32_t>::max())
    return std::numeric_limits<int32_t>::max();
  if (value < std::numeric_limits<int32_t>::min())
    return std::numeric_limits<int32_t>::min();
  return static_cast<int32_t>(value);
}

uint16_t clamp_uint16(int value) {
  if (value < 0) return 0;
  if (value > std::numeric_limits<uint16_t>::max())
    return std::numeric_limits<uint16_t>::max();
  return static_cast<uint16_t>(value);
}

uint8_t clamp_uint8(int value) {
  if (value < 0) return 0;
  if (value > std::numeric_limits<uint8_t>::max())
    return std::numeric_limits<uint8_t>::max();
  return static_cast<uint8_t>(value);
}

int bandwidth_enum_to_mhz(int bw) {
  switch (bw) {
    case BB_BW_10M:
      return 10;
    case BB_BW_20M:
      return 20;
    case BB_BW_40M:
      return 40;
    default:
      return 0;
  }
}

struct ArtosynUsbInfo {
  bool present = false;
  bool hs_mode = false;
};

static constexpr auto kArtosynUsbIdPrimary = "4152:8030";
// Keep old ID for compatibility with setups that still report this variant.
static constexpr auto kArtosynUsbIdLegacy = "1D6B:8030";

static bool contains_artosyn_usb_id(const std::string& lsusb_upper) {
  return OHDUtil::contains(lsusb_upper, kArtosynUsbIdPrimary) ||
         OHDUtil::contains(lsusb_upper, kArtosynUsbIdLegacy);
}

static bool has_artosyn_device_nodes() {
  for (int i = 0; i < 16; ++i) {
    if (OHDFilesystemUtil::exists("/dev/ar_mdev" + std::to_string(i))) {
      return true;
    }
  }
  return OHDFilesystemUtil::exists("/dev/artosyn_sdio");
}

static bool has_sysutils_artosyn_hint() {
  (void)openhd::wait_for_sysutils(std::chrono::seconds(3),
                                  std::chrono::milliseconds(200));
  auto cards_opt = openhd::request_sysutil_wifi_cards();
  if ((!cards_opt.has_value() || cards_opt->empty()) &&
      openhd::request_sysutil_wifi_refresh(std::chrono::seconds(2))) {
    cards_opt = openhd::request_sysutil_wifi_cards();
  }
  if (!cards_opt.has_value()) {
    return false;
  }
  for (const auto& card : cards_opt.value()) {
    if (card.disabled) {
      continue;
    }
    if (OHDUtil::equal_after_uppercase(card.type, "ARTOSYN")) {
      return true;
    }
    if (OHDUtil::contains_after_uppercase(card.interface_name, "AR_MDEV")) {
      return true;
    }
  }
  return false;
}

static ArtosynUsbInfo detect_artosyn_usb_info() {
  ArtosynUsbInfo info{};
  info.present = has_artosyn_device_nodes();
  const auto lsusb_out = OHDUtil::run_command_out("lsusb", false);
  if (!lsusb_out.has_value()) {
    return info;
  }
  const auto lsusb_upper = OHDUtil::to_uppercase(lsusb_out.value());
  if (!contains_artosyn_usb_id(lsusb_upper)) {
    return info;
  }
  info.present = true;
  info.hs_mode =
      OHDUtil::contains(lsusb_upper, "IN HS MODE") ||
      OHDUtil::contains(lsusb_upper, "HS MODE");
  return info;
}

// Cache expensive lsusb probing since link stats are emitted at 2Hz.
static ArtosynUsbInfo detect_artosyn_usb_info_cached(int64_t now_ms) {
  static int64_t last_probe_ms = 0;
  static ArtosynUsbInfo cached{};
  if (last_probe_ms == 0 || (now_ms - last_probe_ms) > 5000) {
    cached = detect_artosyn_usb_info();
    last_probe_ms = now_ms;
  }
  return cached;
}

static bool is_openhd_debug_mode_enabled() {
  return OHDFilesystemUtil::exists("/usr/local/share/openhd/debug.txt") ||
         OHDFilesystemUtil::exists("/usr/share/openhd/debug.txt");
}

static bool force_artosyn_bb_init_start() {
  const char* env = std::getenv("OHD_ARTOSYN_FORCE_BB_INIT");
  if (!env) {
    return false;
  }
  return std::string(env) == "1";
}

static bool allow_artosyn_legacy_recovery() {
  const char* env = std::getenv("OHD_ARTOSYN_LEGACY_RECOVERY");
  return force_artosyn_bb_init_start() || (env && std::string(env) == "1");
}

static bool apply_artosyn_link_settings_on_init() {
  const char* env = std::getenv("OHD_ARTOSYN_APPLY_SETTINGS");
  if (!env) {
    return false;
  }
  return std::string(env) == "1";
}

static bool expose_artosyn_diagnostic_settings() {
  const char* env = std::getenv("OHD_ARTOSYN_DIAG_PARAMS");
  if (!env) {
    return false;
  }
  return std::string(env) == "1";
}

static bool is_openhd_debug_mode_enabled_cached(int64_t now_ms) {
  static int64_t last_probe_ms = 0;
  static bool cached = false;
  if (last_probe_ms == 0 || (now_ms - last_probe_ms) > 5000) {
    cached = is_openhd_debug_mode_enabled();
    last_probe_ms = now_ms;
  }
  return cached;
}

// The Artosyn SDK writes connection failures directly to stderr. Check the
// daemon endpoint quietly first so normal startup does not emit SDK errors
// while the daemon is absent or still starting.
static bool is_tcp_endpoint_reachable_quietly(
    const std::string& address, int port,
    std::chrono::milliseconds timeout = std::chrono::milliseconds(100)) {
  try {
    Poco::Net::SocketAddress endpoint(address, port);
    Poco::Net::StreamSocket socket;
    const Poco::Timespan connect_timeout(
        0, static_cast<long>(timeout.count()) * 1000);
    socket.connect(endpoint, connect_timeout);
    socket.close();
    return true;
  } catch (const Poco::Exception&) {
    return false;
  }
}

static bool probe_artosyn_daemon_once(const ArtosynLink::Config& cfg) {
  if (!is_tcp_endpoint_reachable_quietly(cfg.addr, cfg.port)) {
    return false;
  }
  if (bb_host_connect_test(cfg.addr.c_str(), cfg.port) != 0) {
    return false;
  }
  bb_host_t* host = nullptr;
  if (bb_host_connect(&host, cfg.addr.c_str(), cfg.port) != 0) {
    return false;
  }
  bb_dev_list_t* list = nullptr;
  int n = bb_dev_getlist(host, &list);
  if (list) {
    bb_dev_freelist(list);
  }
  bb_host_disconnect(host);
  return n > 0;
}

static std::string describe_artosyn_runtime_state(const ArtosynLink::Config& cfg) {
  const auto usb = detect_artosyn_usb_info();
  std::ostringstream ss;
  ss << "daemon=" << cfg.addr << ":" << cfg.port
     << " sysutils_hint=" << (has_sysutils_artosyn_hint() ? "yes" : "no")
     << " dev_nodes=" << (has_artosyn_device_nodes() ? "yes" : "no")
     << " usb_present=" << (usb.present ? "yes" : "no")
     << " usb_hs_mode=" << (usb.hs_mode ? "yes" : "no");
  return ss.str();
}

static bool should_restart_artosyn_daemon(const ArtosynLink::Config& cfg) {
  (void)cfg;
  const auto usb = detect_artosyn_usb_info();
  if (!usb.present || !usb.hs_mode) {
    return false;
  }
  if (!has_sysutils_artosyn_hint()) {
    return false;
  }
  static int64_t last_restart_request_ms = 0;
  const int64_t now_ms = openhd::util::steady_clock_time_epoch_ms();
  if (last_restart_request_ms != 0 &&
      (now_ms - last_restart_request_ms) < 15000) {
    return false;
  }
  last_restart_request_ms = now_ms;
  return true;
}
}  // namespace

ArtosynLink::ArtosynLink(OHDProfile profile)
    : m_profile(profile), m_console(openhd::log::create_or_get("artosyn")) {
  m_settings = std::make_unique<openhd::ArtosynLinkSettingsHolder>();
  m_cfg = config_from_settings(m_settings->get_settings());
  m_settings->register_listener([this]() {
    m_console->warn("Artosyn settings changed, restarting link");
    stop_connect_worker();
    stop_stats_thread();
    stop_video_tx_thread();
    stop_rx_threads();
    shutdown_device();
    m_cfg = config_from_settings(m_settings->get_settings());
    if (init_device()) {
      start_rx_threads();
      start_video_tx_thread();
      start_stats_thread();
    } else {
      m_console->warn("Artosyn reconnect will continue in background.");
      start_connect_worker();
    }
  });
  if (!init_device()) {
    m_console->warn(
        "Artosyn init failed. Continuing with background reconnect attempts.");
    start_connect_worker();
  } else {
    start_rx_threads();
    start_video_tx_thread();
    start_stats_thread();
  }
}

ArtosynLink::~ArtosynLink() {
  stop_connect_worker();
  stop_stats_thread();
  stop_video_tx_thread();
  stop_rx_threads();
  shutdown_device();
}

bool ArtosynLink::probe() {
  openhd::ArtosynLinkSettingsHolder holder;
  Config cfg = config_from_settings(holder.get_settings());
  const bool artosyn_hw_hint = has_sysutils_artosyn_hint();
  const auto usb_info = detect_artosyn_usb_info();
  // Treat non-default daemon endpoint as explicit user intent to use Artosyn.
  const bool explicit_artosyn_config =
      cfg.addr != "127.0.0.1" || cfg.port != 50000 || cfg.slot != 0;
  const bool allow_runtime_reconnect =
      artosyn_hw_hint || usb_info.present || explicit_artosyn_config;
  openhd::log::get_default()->info(
      "Artosyn probe: daemon={}:{} slot={} sysutils_hint={} usb_present={} "
      "usb_hs_mode={} explicit_cfg={}",
      cfg.addr, cfg.port, cfg.slot, artosyn_hw_hint ? "yes" : "no",
      usb_info.present ? "yes" : "no", usb_info.hs_mode ? "yes" : "no",
      explicit_artosyn_config ? "yes" : "no");
  // Keep startup snappy: do a short daemon probe, then fall back to background
  // reconnect handling if Artosyn is likely present but daemon is still booting.
  const int max_attempts = 5;
  for (int attempt = 0; attempt < max_attempts; ++attempt) {
    if (probe_artosyn_daemon_once(cfg)) {
      openhd::log::get_default()->info(
          "Artosyn probe success: daemon reachable at {}:{}.", cfg.addr,
          cfg.port);
      return true;
    }
    if (attempt + 1 < max_attempts) {
      std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
  }
  if (allow_runtime_reconnect) {
    openhd::log::get_default()->warn(
        "Artosyn hint/config present (sysutils_hint={} usb_present={} "
        "explicit_cfg={}), but daemon is not reachable at {}:{} yet. "
        "Deferring connection to runtime reconnect.",
        artosyn_hw_hint ? "yes" : "no", usb_info.present ? "yes" : "no",
        explicit_artosyn_config ? "yes" : "no", cfg.addr, cfg.port);
    return true;
  }
  openhd::log::get_default()->warn(
      "Artosyn probe failed: no sysutils hint, no Artosyn USB presence, no "
      "explicit Artosyn config, and daemon {}:{} unreachable. "
      "Falling back to non-Artosyn link stack.",
      cfg.addr, cfg.port);
  return false;
}

bool ArtosynLink::init_device() {
  m_status_ioctl_fail_streak = 0;
  m_legacy_init_retry_done = false;
  m_bb_initialized_by_openhd = false;
  m_bb_started_by_openhd = false;
  m_console->info(
      "Artosyn init begin: role={} daemon={}:{} slot={} vport={} tport={} "
      "datagram={} rx_buf={} tx_buf={} rto_ms={}",
      m_profile.is_air ? "air" : "ground", m_cfg.addr, m_cfg.port, m_cfg.slot,
      m_cfg.video_port, m_cfg.telemetry_port, m_cfg.use_datagram ? 1 : 0,
      m_cfg.rx_buf_size, m_cfg.tx_buf_size, m_cfg.read_timeout_ms);

  m_console->info("Artosyn init step: bb_host_connect");
  if (bb_host_connect(&m_host, m_cfg.addr.c_str(), m_cfg.port) != 0) {
    m_console->warn("Cannot connect to artosyn daemon at {}:{}",
                    m_cfg.addr, m_cfg.port);
    return false;
  }
  m_console->info("Artosyn init step ok: bb_host_connect");

  bb_dev_list_t* list = nullptr;
  m_console->info("Artosyn init step: bb_dev_getlist");
  int n = bb_dev_getlist(m_host, &list);
  m_console->info("Artosyn init step result: bb_dev_getlist n={} list={}",
                  n, list ? "yes" : "no");
  if (n <= 0 || !list) {
    m_console->warn("No artosyn devices found ({})",
                    describe_artosyn_runtime_state(m_cfg));
    if (should_restart_artosyn_daemon(m_cfg)) {
      m_console->warn(
          "Requesting sysutils Artosyn daemon restart for HS-mode USB recovery.");
      if (!openhd::request_sysutil_artosyn_restart(std::chrono::seconds(4))) {
        m_console->warn("Sysutils Artosyn daemon restart request failed.");
      }
    }
    if (list) {
      bb_dev_freelist(list);
    }
    bb_host_disconnect(m_host);
    m_host = nullptr;
    return false;
  }

  m_console->info("Artosyn init step: bb_dev_open");
  bb_dev_t* dev = list[0];
  m_dev = bb_dev_open(dev);
  bb_dev_freelist(list);
  if (!m_dev) {
    m_console->warn("Failed to open artosyn device");
    bb_host_disconnect(m_host);
    m_host = nullptr;
    return false;
  }
  m_console->info("Artosyn init step ok: bb_dev_open");

  if (force_artosyn_bb_init_start()) {
    m_console->info("Artosyn init step: bb_init (forced by env)");
    if (bb_init(m_dev) != 0) {
      m_console->warn("bb_init failed");
    } else {
      m_bb_initialized_by_openhd = true;
      m_console->info("Artosyn init step ok: bb_init");
    }
    m_console->info("Artosyn init step: bb_start (forced by env)");
    if (bb_start(m_dev) != 0) {
      m_console->warn("bb_start failed");
    } else {
      m_bb_started_by_openhd = true;
      m_console->info("Artosyn init step ok: bb_start");
    }
  } else {
    m_console->info(
        "Artosyn init step: skip bb_init/bb_start (daemon-managed SDK flow). "
        "Set OHD_ARTOSYN_FORCE_BB_INIT=1 to force legacy behavior.");
  }
  if (apply_artosyn_link_settings_on_init()) {
    m_console->info("Artosyn init step: apply_link_settings");
    apply_link_settings();
    m_console->info("Artosyn init step ok: apply_link_settings");
  } else {
    m_console->info(
        "Artosyn init step: skip apply_link_settings on init "
        "(daemon-managed safe mode). Set OHD_ARTOSYN_APPLY_SETTINGS=1 "
        "to enable init-time BB_SET_* ioctls.");
  }

  m_running = true;
  update_link_stats();
  try_open_sockets_if_ready();
  if (m_video_fd < 0 || m_telemetry_fd < 0) {
    m_console->warn(
        "Artosyn sockets deferred until link is CONNECT (video_fd={} tele_fd={}).",
        m_video_fd, m_telemetry_fd);
  }
  m_console->info("Artosyn init complete: video_fd={} tele_fd={}", m_video_fd,
                  m_telemetry_fd);
  return true;
}

void ArtosynLink::shutdown_device() {
  if (m_video_fd >= 0) {
    bb_socket_close(m_video_fd);
    m_video_fd = -1;
  }
  if (m_telemetry_fd >= 0 && !m_shared_socket) {
    bb_socket_close(m_telemetry_fd);
  }
  m_telemetry_fd = -1;
  m_shared_socket = false;
  if (m_dev) {
    if (m_bb_started_by_openhd) {
      bb_stop(m_dev);
    }
    if (m_bb_initialized_by_openhd) {
      bb_deinit(m_dev);
    }
    bb_dev_close(m_dev);
    m_dev = nullptr;
  }
  if (m_host) {
    bb_host_disconnect(m_host);
    m_host = nullptr;
  }
  m_running = false;
}

int ArtosynLink::open_socket(int port, bool want_tx, bool want_rx,
                             bool allow_legacy_fallback) {
  if (!m_dev) return -1;

  auto make_flags = [want_tx, want_rx](bool use_datagram) {
    (void)want_tx;
    (void)want_rx;
    // The P401 USB-data firmware requires a bidirectional socket even for a
    // logically one-way OpenHD endpoint.
    uint32_t flags = BB_SOCK_FLAG_TX | BB_SOCK_FLAG_RX;
    if (use_datagram) flags |= BB_SOCK_FLAG_DATAGRAM;
    return flags;
  };

  bb_sock_opt_t opt{};
  opt.tx_buf_size = m_cfg.tx_buf_size;
  opt.rx_buf_size = m_cfg.rx_buf_size;

  uint32_t flags = make_flags(m_cfg.use_datagram);
  int fd = bb_socket_open(m_dev, static_cast<bb_slot_e>(m_cfg.slot),
                          static_cast<uint32_t>(port), flags, &opt);
  if (fd < 0 && m_cfg.use_datagram) {
    flags = make_flags(false);
    m_console->warn(
        "bb_socket_open failed for slot {} port {} flags=0x{:x}; retrying "
        "without datagram flags=0x{:x}",
        m_cfg.slot, port, make_flags(true), flags);
    fd = bb_socket_open(m_dev, static_cast<bb_slot_e>(m_cfg.slot),
                        static_cast<uint32_t>(port), flags, &opt);
    if (fd >= 0) {
      m_console->info("artosyn socket open stream fallback port {} fd {}",
                      port, fd);
    }
  }
  if (fd < 0) {
    m_console->warn(
        "bb_socket_open failed for slot {} port {} flags=0x{:x} tx={} rx={} "
        "datagram={}",
        m_cfg.slot, port, flags, want_tx ? 1 : 0, want_rx ? 1 : 0,
        (flags & BB_SOCK_FLAG_DATAGRAM) ? 1 : 0);
    if (allow_legacy_fallback && allow_artosyn_legacy_recovery() &&
        !m_legacy_init_retry_done.exchange(true)) {
      m_console->warn(
          "Artosyn socket open failed, trying legacy bb_init/bb_start "
          "fallback once.");
      const int ret_init = bb_init(m_dev);
      int ret_start = -1;
      if (ret_init == 0) {
        m_bb_initialized_by_openhd = true;
        ret_start = bb_start(m_dev);
        if (ret_start == 0) {
          m_bb_started_by_openhd = true;
        }
      }
      m_console->warn(
          "Artosyn legacy fallback result: bb_init={} bb_start={}",
          ret_init, ret_start);
      fd = bb_socket_open(m_dev, static_cast<bb_slot_e>(m_cfg.slot),
                          static_cast<uint32_t>(port), flags, &opt);
      if (fd < 0) {
        m_console->warn(
            "bb_socket_open retry failed for slot {} port {} flags=0x{:x}",
            m_cfg.slot, port, flags);
      } else {
        m_console->info("artosyn socket open retry port {} fd {}", port, fd);
      }
    }
  } else {
    m_console->info("artosyn socket open port {} fd {}", port, fd);
  }
  return fd;
}

void ArtosynLink::open_configured_sockets(bool allow_legacy_fallback,
                                          const char* reason) {
  const int64_t now_ms = openhd::util::steady_clock_time_epoch_ms();
  const int64_t next_probe_ms = m_next_socket_probe_ms.load();
  if (next_probe_ms != 0 && now_ms < next_probe_ms) {
    return;
  }
  bool any_failed = false;
  if (m_cfg.telemetry_port == m_cfg.video_port) {
    if (m_video_fd < 0 || m_telemetry_fd < 0) {
      m_console->info("Artosyn init step: open shared socket ({})", reason);
      const int fd = open_socket(m_cfg.video_port, true, true,
                                 allow_legacy_fallback);
      m_video_fd = fd;
      m_telemetry_fd = fd;
      m_shared_socket = fd >= 0;
      any_failed = fd < 0;
    }
    m_next_socket_probe_ms = any_failed ? now_ms + 3000 : 0;
    return;
  }
  if (m_telemetry_fd < 0) {
    m_console->info("Artosyn init step: open telemetry socket ({})", reason);
    m_telemetry_fd = open_socket(m_cfg.telemetry_port, m_profile.is_air,
                                 m_profile.is_ground(),
                                 allow_legacy_fallback);
    any_failed = any_failed || m_telemetry_fd < 0;
  }
  if (m_video_fd < 0) {
    m_console->info("Artosyn init step: open video socket ({})", reason);
    m_video_fd = open_socket(m_cfg.video_port, m_profile.is_air,
                             m_profile.is_ground(), allow_legacy_fallback);
    any_failed = any_failed || m_video_fd < 0;
  }
  if (any_failed) {
    m_next_socket_probe_ms = now_ms + 3000;
  } else {
    m_next_socket_probe_ms = 0;
  }
}

void ArtosynLink::try_open_sockets_if_ready() {
  if (!m_dev) {
    return;
  }
  if (m_video_fd >= 0 && m_telemetry_fd >= 0) {
    return;
  }
  static int64_t s_last_status_probe_ms = 0;
  const int64_t now_ms = openhd::util::steady_clock_time_epoch_ms();
  if (s_last_status_probe_ms != 0 && (now_ms - s_last_status_probe_ms) < 1000) {
    return;
  }
  s_last_status_probe_ms = now_ms;

  bb_get_status_in_t st_in{};
  bb_get_status_out_t st_out{};
  st_in.user_bmp = (1 << m_cfg.slot);
  if (bb_ioctl_ex(m_dev, BB_GET_STATUS, &st_in, &st_out, 100) != 0) {
    static int64_t s_last_status_err_log_ms = 0;
    const int fail_streak = ++m_status_ioctl_fail_streak;
    if (now_ms - s_last_status_err_log_ms >= 2000) {
      s_last_status_err_log_ms = now_ms;
      m_console->warn(
          "Artosyn defer socket open: BB_GET_STATUS timeout/fail (slot={} "
          "streak={})",
          m_cfg.slot, fail_streak);
    }
    if (allow_artosyn_legacy_recovery() && fail_streak >= 3 &&
        !m_legacy_init_retry_done.exchange(true)) {
      m_console->warn(
          "Artosyn BB_GET_STATUS keeps failing, trying legacy bb_init/bb_start "
          "fallback once.");
      const int ret_init = bb_init(m_dev);
      int ret_start = -1;
      if (ret_init == 0) {
        m_bb_initialized_by_openhd = true;
        ret_start = bb_start(m_dev);
        if (ret_start == 0) {
          m_bb_started_by_openhd = true;
        }
      }
      m_console->warn(
          "Artosyn legacy fallback result: bb_init={} bb_start={}",
          ret_init, ret_start);
    }
    if (fail_streak >= 2) {
      m_console->warn(
          "Artosyn BB_GET_STATUS failed, probing sockets anyway "
          "(streak={}).",
          fail_streak);
      open_configured_sockets(false, "status-fallback");
    }
    const int64_t last_recover_ms = m_last_recover_request_ms.load();
    if (m_video_fd < 0 && m_telemetry_fd < 0 && fail_streak >= 5 &&
        (now_ms - last_recover_ms) > 10000) {
      m_last_recover_request_ms = now_ms;
      m_console->warn(
          "Artosyn repeated BB_GET_STATUS failures, requesting daemon/link "
          "recovery (streak={}).",
          fail_streak);
      if (should_restart_artosyn_daemon(m_cfg)) {
        if (!openhd::request_sysutil_artosyn_restart(std::chrono::seconds(4))) {
          m_console->warn("Sysutils Artosyn daemon restart request failed.");
        }
      }
      stop_rx_threads();
      shutdown_device();
      start_connect_worker();
    }
    return;
  }
  m_status_ioctl_fail_streak = 0;

  const int slot = m_cfg.slot;
  const int link_state = st_out.link_status[slot].state;
  const int pair_state = static_cast<int>(st_out.link_status[slot].pair_state);
  const int mode = static_cast<int>(st_out.mode);
  const int sync_mode = static_cast<int>(st_out.sync_mode);
  const int role = static_cast<int>(st_out.role);
  const int sync_master = static_cast<int>(st_out.sync_master);
  const int cfg_sbmp = static_cast<int>(st_out.cfg_sbmp);
  const int rt_sbmp = static_cast<int>(st_out.rt_sbmp);
  if (link_state != BB_LINK_STATE_CONNECT) {
    static int64_t s_last_defer_log_ms = 0;
    const int64_t now_ms = openhd::util::steady_clock_time_epoch_ms();
    if (now_ms - s_last_defer_log_ms >= 2000) {
      s_last_defer_log_ms = now_ms;
      m_console->info(
          "Artosyn defer socket open: link_state={} pair_state={} role={} "
          "mode={} sync_mode={} sync_master={} cfg_sbmp=0x{:x} rt_sbmp=0x{:x}",
          link_state, pair_state, role, mode, sync_mode, sync_master, cfg_sbmp,
          rt_sbmp);
      m_console->info(
          "Artosyn probing sockets despite non-CONNECT status; socket open "
          "will decide readiness.");
    }
    open_configured_sockets(false, "not-connect");
    return;
  }

  open_configured_sockets(true, "connected");
}

void ArtosynLink::start_rx_threads() {
  if (m_profile.is_ground() && m_shared_socket && m_video_fd >= 0 &&
      !m_rx_video_thread.joinable()) {
    m_rx_video_thread = std::thread([this]() { rx_loop_shared(); });
    return;
  }
  if (m_profile.is_ground() && m_video_fd >= 0 &&
      !m_rx_video_thread.joinable()) {
    m_rx_video_thread = std::thread([this]() { rx_loop_video(); });
  }
  if (m_profile.is_ground() && m_telemetry_fd >= 0 &&
      !m_rx_telemetry_thread.joinable()) {
    m_rx_telemetry_thread = std::thread([this]() { rx_loop_telemetry(); });
  }
}

void ArtosynLink::stop_rx_threads() {
  m_running = false;
  if (m_rx_video_thread.joinable()) {
    m_rx_video_thread.join();
  }
  if (m_rx_telemetry_thread.joinable()) {
    m_rx_telemetry_thread.join();
  }
}

void ArtosynLink::rx_loop_video() {
  std::vector<uint8_t> buf(static_cast<size_t>(m_cfg.rx_buf_size));
  int64_t last_error_log_ms = 0;
  while (m_running) {
    int n = bb_socket_read(m_video_fd, buf.data(),
                           static_cast<uint32_t>(buf.size()),
                           m_cfg.read_timeout_ms);
    if (n > 0) {
      m_rx_total_bytes.fetch_add(static_cast<uint64_t>(n),
                                 std::memory_order_relaxed);
      m_rx_total_packets.fetch_add(1, std::memory_order_relaxed);
      m_last_rx_packet_ts_ms.store(
          openhd::util::steady_clock_time_epoch_ms(),
          std::memory_order_relaxed);
      on_receive_video_data(0, buf.data(), n);
    } else if (n < 0) {
      const int64_t now_ms = openhd::util::steady_clock_time_epoch_ms();
      if (now_ms - last_error_log_ms >= 2000) {
        last_error_log_ms = now_ms;
        m_console->warn("Artosyn video read error n={} errno={} ({})", n, errno,
                        std::strerror(errno));
      }
    }
  }
}

void ArtosynLink::rx_loop_telemetry() {
  std::vector<uint8_t> buf(static_cast<size_t>(m_cfg.rx_buf_size));
  int64_t last_error_log_ms = 0;
  while (m_running) {
    int n = bb_socket_read(m_telemetry_fd, buf.data(),
                           static_cast<uint32_t>(buf.size()),
                           m_cfg.read_timeout_ms);
    if (n > 0) {
      m_rx_total_bytes.fetch_add(static_cast<uint64_t>(n),
                                 std::memory_order_relaxed);
      m_rx_total_packets.fetch_add(1, std::memory_order_relaxed);
      m_rx_tele_bytes.fetch_add(static_cast<uint64_t>(n),
                                std::memory_order_relaxed);
      m_rx_tele_packets.fetch_add(1, std::memory_order_relaxed);
      m_last_rx_packet_ts_ms.store(
          openhd::util::steady_clock_time_epoch_ms(),
          std::memory_order_relaxed);
      auto shared = std::make_shared<std::vector<uint8_t>>(buf.begin(),
                                                           buf.begin() + n);
      on_receive_telemetry_data(shared);
    } else if (n < 0) {
      const int64_t now_ms = openhd::util::steady_clock_time_epoch_ms();
      if (now_ms - last_error_log_ms >= 2000) {
        last_error_log_ms = now_ms;
        m_console->warn("Artosyn telemetry read error n={} errno={} ({})", n,
                        errno, std::strerror(errno));
      }
    }
  }
}

void ArtosynLink::rx_loop_shared() {
  std::vector<uint8_t> read_buf(static_cast<size_t>(m_cfg.rx_buf_size));
  std::vector<uint8_t> stream_buf(256U * 1024U);
  size_t stream_used = 0;
  while (m_running) {
    const int n = bb_socket_read(m_video_fd, read_buf.data(),
                                 static_cast<uint32_t>(read_buf.size()),
                                 m_cfg.read_timeout_ms);
    if (n <= 0) {
      continue;
    }
    if (stream_used + static_cast<size_t>(n) > stream_buf.size()) {
      stream_used = 0;
    }
    std::memcpy(stream_buf.data() + stream_used, read_buf.data(),
                static_cast<size_t>(n));
    stream_used += static_cast<size_t>(n);
    while (stream_used >= 7U) {
      if (std::memcmp(stream_buf.data(), "P401", 4) != 0) {
        std::memmove(stream_buf.data(), stream_buf.data() + 1,
                     --stream_used);
        continue;
      }
      const uint8_t stream_id = stream_buf[4];
      const uint32_t payload_size =
          (static_cast<uint32_t>(stream_buf[5]) << 8U) | stream_buf[6];
      const size_t frame_size = static_cast<size_t>(payload_size) + 7U;
      if (stream_used < frame_size) {
        break;
      }
      m_rx_total_bytes.fetch_add(payload_size, std::memory_order_relaxed);
      m_rx_total_packets.fetch_add(1, std::memory_order_relaxed);
      m_last_rx_packet_ts_ms.store(
          openhd::util::steady_clock_time_epoch_ms(),
          std::memory_order_relaxed);
      if (stream_id == 2) {
        on_receive_video_data(0, stream_buf.data() + 7, payload_size);
      } else if (stream_id == 1) {
        m_rx_tele_bytes.fetch_add(payload_size, std::memory_order_relaxed);
        m_rx_tele_packets.fetch_add(1, std::memory_order_relaxed);
        auto shared = std::make_shared<std::vector<uint8_t>>(
            stream_buf.begin() + 7,
            stream_buf.begin() + static_cast<ptrdiff_t>(frame_size));
        on_receive_telemetry_data(shared);
      }
      std::memmove(stream_buf.data(), stream_buf.data() + frame_size,
                   stream_used - frame_size);
      stream_used -= frame_size;
    }
  }
}

int ArtosynLink::write_stream_packet(int fd, uint8_t stream_id,
                                     const uint8_t* data, uint32_t size) {
  // Keep the application queue bounded instead of using a near-zero write
  // timeout. Once a complete video frame is admitted, finish writing it so
  // the receiver never gets a deliberately truncated H.264 frame.
  const int write_timeout_ms = m_cfg.read_timeout_ms;
  std::lock_guard<std::mutex> write_lock(m_radio_write_mutex);
  if (!m_shared_socket) {
    const int written =
        bb_socket_write(fd, data, size, write_timeout_ms);
    return written == static_cast<int>(size) ? written : -1;
  }
  if (size > 65535U) {
    return -1;
  }
  std::vector<uint8_t> framed(size + 7U);
  std::memcpy(framed.data(), "P401", 4);
  framed[4] = stream_id;
  framed[5] = static_cast<uint8_t>((size >> 8U) & 0xffU);
  framed[6] = static_cast<uint8_t>(size & 0xffU);
  std::memcpy(framed.data() + 7, data, size);
  const int written = bb_socket_write(
      fd, framed.data(), static_cast<uint32_t>(framed.size()),
      write_timeout_ms);
  return written == static_cast<int>(framed.size()) ? written : -1;
}

void ArtosynLink::log_tx_error_throttled(const char* stream, int ret) {
  const int64_t now_ms = openhd::util::steady_clock_time_epoch_ms();
  const int64_t last_ms = m_last_tx_error_log_ms.load();
  if (now_ms - last_ms < 2000) {
    return;
  }
  m_last_tx_error_log_ms = now_ms;
  m_console->warn("Artosyn {} write failed ret={} errno={} ({})", stream, ret,
                  errno, std::strerror(errno));
}

void ArtosynLink::transmit_telemetry_data(TelemetryTxPacket packet) {
  if (!m_profile.is_air) {
    return;
  }
  if (m_telemetry_fd < 0) return;
  if (!packet.data || packet.data->empty()) return;
  int injections = packet.n_injections < 1 ? 1 : packet.n_injections;
  for (int i = 0; i < injections; ++i) {
    const int written =
        write_stream_packet(m_telemetry_fd, 1, packet.data->data(),
                            static_cast<uint32_t>(packet.data->size()));
    if (written < 0) {
      log_tx_error_throttled("telemetry", written);
      continue;
    }
    const auto accounted_bytes =
        static_cast<uint64_t>(written > 0 ? written : packet.data->size());
    m_tx_total_bytes.fetch_add(accounted_bytes,
                               std::memory_order_relaxed);
    m_tx_total_packets.fetch_add(1, std::memory_order_relaxed);
    m_tx_tele_bytes.fetch_add(accounted_bytes, std::memory_order_relaxed);
    m_tx_tele_packets.fetch_add(1, std::memory_order_relaxed);
  }
}

void ArtosynLink::transmit_video_data(
    int stream_index,
    const openhd::FragmentedVideoFrame& fragmented_video_frame) {
  if (!m_profile.is_air) {
    return;
  }
  if (m_video_fd < 0) return;
  {
    std::lock_guard<std::mutex> lock(m_video_tx_mutex);
    const bool is_recovery_frame = fragmented_video_frame.is_intra_stream ||
                                   fragmented_video_frame.is_idr_frame;
    if (m_video_tx_wait_for_idr && !is_recovery_frame) {
      ++m_video_tx_dropped_frames;
      return;
    }
    constexpr size_t kMaxPendingFrames = 8;
    if (m_pending_video_frames.size() < kMaxPendingFrames) {
      // WBStreamTx only evicts queued frames for an IDR when the queue is
      // actually full. Clearing healthy pending P-frames on every periodic IDR
      // creates an artificial RTP sequence gap once per GOP.
      m_pending_video_frames.emplace_back(stream_index, fragmented_video_frame);
    } else if (is_recovery_frame) {
      // Congested queue: prioritize decoder recovery over stale dependencies.
      m_video_tx_dropped_frames += m_pending_video_frames.size();
      m_pending_video_frames.clear();
      m_pending_video_frames.emplace_back(stream_index, fragmented_video_frame);
    } else {
      // Preserve the queued dependency chain instead of replacing an older
      // P-frame with a newer frame which depends on the dropped one.
      ++m_video_tx_dropped_frames;
      return;
    }
  }
  m_video_tx_cv.notify_one();
}

void ArtosynLink::start_video_tx_thread() {
  if (!m_profile.is_air || m_video_tx_thread.joinable()) {
    return;
  }
  {
    std::lock_guard<std::mutex> lock(m_video_tx_mutex);
    m_stop_video_tx = false;
    m_video_tx_wait_for_idr = false;
    m_video_tx_dropped_frames = 0;
  }
  m_video_tx_thread = std::thread([this]() { video_tx_loop(); });
}

void ArtosynLink::stop_video_tx_thread() {
  {
    std::lock_guard<std::mutex> lock(m_video_tx_mutex);
    m_stop_video_tx = true;
    m_pending_video_frames.clear();
  }
  m_video_tx_cv.notify_all();
  if (m_video_tx_thread.joinable()) {
    m_video_tx_thread.join();
  }
}

void ArtosynLink::video_tx_loop() {
  while (true) {
    std::vector<std::pair<int, openhd::FragmentedVideoFrame>> pending;
    {
      std::unique_lock<std::mutex> lock(m_video_tx_mutex);
      m_video_tx_cv.wait(lock, [this]() {
        return m_stop_video_tx || !m_pending_video_frames.empty();
      });
      if (m_stop_video_tx) {
        return;
      }
      // At 120 fps four frames arrive in about 25ms. A single vendor call for
      // the group avoids the per-call acknowledgement ceiling while placing a
      // hard bound on both application queue depth and aggregation latency.
      m_video_tx_cv.wait_for(lock, std::chrono::milliseconds(25), [this]() {
        return m_stop_video_tx || m_pending_video_frames.size() >= 4;
      });
      if (m_stop_video_tx) {
        return;
      }
      pending.reserve(m_pending_video_frames.size());
      while (!m_pending_video_frames.empty()) {
        pending.emplace_back(std::move(m_pending_video_frames.front()));
        m_pending_video_frames.pop_front();
      }
    }
    bool complete = true;
    if (m_shared_socket) {
      // The vendor stream API acknowledges every bb_socket_write call. Sending
      // one call per ~1400-byte RTP packet limits throughput to a few dozen
      // packets/s even when the RF link has plenty of capacity. Coalesce all
      // framed RTP records belonging to this OpenHD frame. The ground parser
      // already treats the byte stream as a sequence of P401 records and
      // restores the original UDP packet boundaries.
      size_t batch_size = 0;
      for (const auto& frame : pending) {
        for (const auto& fragment : frame.second.rtp_fragments) {
          batch_size += 7U + fragment->size();
        }
      }
      std::vector<uint8_t> batch;
      batch.reserve(batch_size);
      for (const auto& frame : pending) {
        for (const auto& fragment : frame.second.rtp_fragments) {
          if (fragment->empty() || fragment->size() > 65535U) {
            complete = false;
            break;
          }
          const auto size = static_cast<uint16_t>(fragment->size());
          batch.insert(batch.end(), {'P', '4', '0', '1', 2});
          batch.push_back(static_cast<uint8_t>((size >> 8U) & 0xffU));
          batch.push_back(static_cast<uint8_t>(size & 0xffU));
          batch.insert(batch.end(), fragment->begin(), fragment->end());
        }
        if (!complete) {
          break;
        }
      }
      int written = -1;
      if (complete && !batch.empty()) {
        std::lock_guard<std::mutex> write_lock(m_radio_write_mutex);
        // A large IDR may need longer than the normal control-message timeout,
        // but this remains bounded so shutdown/reconnect cannot hang forever.
        const int video_timeout_ms = std::max(500, m_cfg.read_timeout_ms);
        written = bb_socket_write(m_video_fd, batch.data(),
                                  static_cast<uint32_t>(batch.size()),
                                  video_timeout_ms);
        complete = written == static_cast<int>(batch.size());
      }
      if (!complete) {
        log_tx_error_throttled("video frame", written);
      } else {
        for (const auto& frame : pending) {
          for (const auto& fragment : frame.second.rtp_fragments) {
            const auto accounted_bytes =
                static_cast<uint64_t>(fragment->size() + 7U);
            m_video_bitrate_meter.on_tx_fragment(frame.first,
                                                 accounted_bytes);
            m_tx_total_bytes.fetch_add(accounted_bytes,
                                       std::memory_order_relaxed);
            m_tx_total_packets.fetch_add(1, std::memory_order_relaxed);
          }
        }
      }
    } else {
      for (const auto& frame : pending) {
        for (const auto& fragment : frame.second.rtp_fragments) {
          const int written =
              write_stream_packet(m_video_fd, 2, fragment->data(),
                                  static_cast<uint32_t>(fragment->size()));
          if (written < 0) {
            log_tx_error_throttled("video", written);
            complete = false;
            break;
          }
          const auto accounted_bytes =
              static_cast<uint64_t>(written > 0 ? written : fragment->size());
          m_video_bitrate_meter.on_tx_fragment(frame.first, accounted_bytes);
          m_tx_total_bytes.fetch_add(accounted_bytes,
                                     std::memory_order_relaxed);
          m_tx_total_packets.fetch_add(1, std::memory_order_relaxed);
        }
        if (!complete) {
          break;
        }
      }
    }
    {
      std::lock_guard<std::mutex> lock(m_video_tx_mutex);
      const bool contains_recovery_frame =
          std::any_of(pending.begin(), pending.end(), [](const auto& frame) {
            return frame.second.is_intra_stream || frame.second.is_idr_frame;
          });
      if (!complete) {
        m_video_tx_wait_for_idr = true;
        m_video_tx_dropped_frames += m_pending_video_frames.size();
        m_pending_video_frames.clear();
      } else if (contains_recovery_frame) {
        m_video_tx_wait_for_idr = false;
      }
    }
  }
}

void ArtosynLink::transmit_audio_data(
    const openhd::AudioPacket& audio_packet) {
  (void)audio_packet;
  // Not implemented in this simple integration.
}

void ArtosynLink::start_stats_thread() {
  if (m_stats_running.exchange(true)) {
    return;
  }
  m_stats_thread = std::thread([this]() { stats_loop(); });
}

void ArtosynLink::stop_stats_thread() {
  m_stats_running = false;
  if (m_stats_thread.joinable()) {
    m_stats_thread.join();
  }
}

void ArtosynLink::start_connect_worker() {
  if (m_connect_thread.joinable()) {
    return;
  }
  m_stop_connect_worker = false;
  m_connect_thread = std::thread([this]() { connect_loop(); });
}

void ArtosynLink::stop_connect_worker() {
  m_stop_connect_worker = true;
  if (m_connect_thread.joinable()) {
    m_connect_thread.join();
  }
}

void ArtosynLink::connect_loop() {
  int retry_count = 0;
  while (!m_stop_connect_worker) {
    if (init_device()) {
      m_console->warn("Artosyn daemon connected.");
      start_rx_threads();
      start_video_tx_thread();
      start_stats_thread();
      return;
    }
    ++retry_count;
    m_console->warn(
        "Artosyn reconnect retry {} in 5s (state: {})", retry_count,
        describe_artosyn_runtime_state(m_cfg));
    std::this_thread::sleep_for(std::chrono::seconds(5));
  }
}

void ArtosynLink::stats_loop() {
  while (m_stats_running) {
    try_open_sockets_if_ready();
    start_rx_threads();
    update_link_stats();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

void ArtosynLink::update_video_bitrate_recommendation(int capacity_kbits,
                                                      int64_t now_ms) {
  if (!m_profile.is_air || capacity_kbits <= 0) {
    return;
  }

  // The MCS throughput table is a capacity estimate. Do not use measured TX
  // traffic here: doing so would make every encoder reduction lower the next
  // recommendation and create a downward feedback spiral.
  if (m_bitrate_capacity_ema_kbits <= 0) {
    m_bitrate_capacity_ema_kbits = capacity_kbits;
  } else if (capacity_kbits < m_bitrate_capacity_ema_kbits) {
    // React immediately when the radio loses capacity.
    m_bitrate_capacity_ema_kbits = capacity_kbits;
  } else {
    // Require several good samples before using newly available capacity.
    m_bitrate_capacity_ema_kbits =
        (m_bitrate_capacity_ema_kbits * 3 + capacity_kbits) / 4;
  }

  static constexpr int kUtilizationPercent = 88;
  static constexpr int kProtocolAndTelemetryReserveKbits = 250;
  static constexpr int kMinEncoderBitrateKbits = 1000;
  static constexpr int kMaxEncoderBitrateKbits = 20000;
  static constexpr int kIncreasePerSecondKbits = 500;
  static constexpr int kChangeHysteresisKbits = 250;
  static constexpr int64_t kRefreshIntervalMs = 2000;

  int target_kbits =
      m_bitrate_capacity_ema_kbits * kUtilizationPercent / 100 -
      kProtocolAndTelemetryReserveKbits;
  target_kbits = std::clamp(target_kbits, kMinEncoderBitrateKbits,
                            kMaxEncoderBitrateKbits);

  int recommendation_kbits = target_kbits;
  if (m_last_recommended_bitrate_kbits > 0 &&
      target_kbits > m_last_recommended_bitrate_kbits) {
    const int64_t elapsed_ms =
        m_last_bitrate_calculation_ms > 0
            ? std::max<int64_t>(1, now_ms - m_last_bitrate_calculation_ms)
            : 1;
    const int max_increase_kbits = std::max<int>(
        1, static_cast<int>(elapsed_ms * kIncreasePerSecondKbits / 1000));
    recommendation_kbits =
        std::min(target_kbits,
                 m_last_recommended_bitrate_kbits + max_increase_kbits);
  }
  m_last_bitrate_calculation_ms = now_ms;

  const bool meaningful_change =
      m_last_recommended_bitrate_kbits <= 0 ||
      std::abs(recommendation_kbits - m_last_recommended_bitrate_kbits) >=
          kChangeHysteresisKbits;
  const bool refresh =
      m_last_bitrate_announcement_ms <= 0 ||
      (now_ms - m_last_bitrate_announcement_ms) >= kRefreshIntervalMs;
  if (!meaningful_change && !refresh) {
    return;
  }

  if (meaningful_change) {
    m_console->info(
        "Artosyn bitrate control: capacity={} kbit/s filtered={} kbit/s "
        "encoder_limit={} kbit/s",
        capacity_kbits, m_bitrate_capacity_ema_kbits,
        recommendation_kbits);
  }
  m_last_recommended_bitrate_kbits = recommendation_kbits;
  m_last_bitrate_announcement_ms = now_ms;
  openhd::LinkActionHandler::LinkBitrateInformation bitrate_info{};
  bitrate_info.recommended_encoder_bitrate_kbits = recommendation_kbits;
  bitrate_info.is_link_capacity_limit = true;
  openhd::LinkActionHandler::instance().action_request_bitrate_change_handle(
      bitrate_info);
}

void ArtosynLink::update_link_stats() {
  if (!m_dev) return;
  const int64_t now_ms = openhd::util::steady_clock_time_epoch_ms();
  const bool first_sample = m_last_stats_ts_ms == 0;
  int64_t dt_ms = first_sample ? 1000 : (now_ms - m_last_stats_ts_ms);
  if (dt_ms <= 0) return;

  const uint64_t tx_bytes = m_tx_total_bytes.load();
  const uint64_t tx_packets = m_tx_total_packets.load();
  const uint64_t rx_bytes = m_rx_total_bytes.load();
  const uint64_t rx_packets = m_rx_total_packets.load();
  const uint64_t tx_tele_bytes = m_tx_tele_bytes.load();
  const uint64_t tx_tele_packets = m_tx_tele_packets.load();
  const uint64_t rx_tele_bytes = m_rx_tele_bytes.load();
  const uint64_t rx_tele_packets = m_rx_tele_packets.load();

  const uint64_t d_tx_bytes =
      first_sample ? 0 : (tx_bytes - m_last_stats_tx_bytes);
  const uint64_t d_tx_packets =
      first_sample ? 0 : (tx_packets - m_last_stats_tx_packets);
  const uint64_t d_rx_bytes =
      first_sample ? 0 : (rx_bytes - m_last_stats_rx_bytes);
  const uint64_t d_rx_packets =
      first_sample ? 0 : (rx_packets - m_last_stats_rx_packets);
  const uint64_t d_tx_tele_bytes =
      first_sample ? 0 : (tx_tele_bytes - m_last_stats_tx_tele_bytes);
  const uint64_t d_tx_tele_packets =
      first_sample ? 0 : (tx_tele_packets - m_last_stats_tx_tele_packets);
  const uint64_t d_rx_tele_bytes =
      first_sample ? 0 : (rx_tele_bytes - m_last_stats_rx_tele_bytes);
  const uint64_t d_rx_tele_packets =
      first_sample ? 0 : (rx_tele_packets - m_last_stats_rx_tele_packets);

  const int64_t scale = 1000;
  const int64_t tx_bps =
      static_cast<int64_t>((d_tx_bytes * 8 * scale) / dt_ms);
  const int64_t rx_bps =
      static_cast<int64_t>((d_rx_bytes * 8 * scale) / dt_ms);
  const int64_t tx_pps =
      static_cast<int64_t>((d_tx_packets * scale) / dt_ms);
  const int64_t rx_pps =
      static_cast<int64_t>((d_rx_packets * scale) / dt_ms);
  const int64_t tx_tele_bps =
      static_cast<int64_t>((d_tx_tele_bytes * 8 * scale) / dt_ms);
  const int64_t rx_tele_bps =
      static_cast<int64_t>((d_rx_tele_bytes * 8 * scale) / dt_ms);
  const int64_t tx_tele_pps =
      static_cast<int64_t>((d_tx_tele_packets * scale) / dt_ms);
  const int64_t rx_tele_pps =
      static_cast<int64_t>((d_rx_tele_packets * scale) / dt_ms);

  m_last_stats_ts_ms = now_ms;
  m_last_stats_tx_bytes = tx_bytes;
  m_last_stats_tx_packets = tx_packets;
  m_last_stats_rx_bytes = rx_bytes;
  m_last_stats_rx_packets = rx_packets;
  m_last_stats_tx_tele_bytes = tx_tele_bytes;
  m_last_stats_tx_tele_packets = tx_tele_packets;
  m_last_stats_rx_tele_bytes = rx_tele_bytes;
  m_last_stats_rx_tele_packets = rx_tele_packets;

  openhd::link_statistics::StatsAirGround stats{};
  stats.is_air = m_profile.is_air;
  stats.ready = true;

  stats.monitor_mode_link.curr_tx_bps = clamp_int32(tx_bps);
  stats.monitor_mode_link.curr_rx_bps = clamp_int32(rx_bps);
  stats.monitor_mode_link.curr_tx_pps = clamp_int16(static_cast<int>(tx_pps));
  stats.monitor_mode_link.curr_rx_pps = clamp_int16(static_cast<int>(rx_pps));

  int link_state = -1;
  int rx_mcs = -1;
  int tx_mcs = -1;
  int bw = -1;
  int rx_bw = -1;
  int tx_phy_tp = -1;
  int tx_real_tp = -1;
  int rx_phy_tp = -1;
  int rx_real_tp = -1;
  int tx_tp_th = -1;
  int rx_tp_th = -1;
  int tx_freq_khz = -1;
  int rx_freq_khz = -1;
  const bool have_metrics =
      !first_sample &&
      read_metrics(&link_state, &rx_mcs, &tx_mcs, &bw, &tx_phy_tp, &tx_real_tp,
                   &tx_freq_khz, &rx_freq_khz, &rx_bw, &rx_phy_tp,
                   &rx_real_tp);
  (void)link_state;
  (void)rx_freq_khz;
  (void)rx_bw;
  if (!first_sample) {
    (void)read_mcs_throughput(&tx_tp_th, &rx_tp_th);
  }
  // Prefer the vendor MCS throughput ceiling. Physical throughput is a safe
  // fallback; current/real traffic is deliberately not used for control.
  const int bitrate_capacity_kbits =
      tx_tp_th > 0
          ? tx_tp_th
          : (tx_phy_tp > 0 ? (tx_phy_tp + 500) / 1000 : -1);
  if (!first_sample && have_metrics && bitrate_capacity_kbits > 0) {
    update_video_bitrate_recommendation(bitrate_capacity_kbits, now_ms);
  }
  if (have_metrics) {
    stats.monitor_mode_link.curr_tx_mcs_index = clamp_uint8(tx_mcs);
    stats.monitor_mode_link.curr_tx_channel_w_mhz =
        clamp_uint8(bandwidth_enum_to_mhz(bw));
    const int tx_mhz = tx_freq_khz > 0 ? tx_freq_khz / 1000 : 0;
    stats.monitor_mode_link.curr_tx_channel_mhz = clamp_uint16(tx_mhz);
    // The vendor status API reports physical and "real" throughput in bit/s,
    // while tx_tp_th and the MAVLink curr_rate field use kbit/s. Passing the
    // raw real-throughput value used to saturate the uint16 MAVLink field at
    // 65535, which made ground UIs display a bogus ~66 Mbit/s.
    const int tx_real_kbits =
        tx_real_tp > 0 ? (tx_real_tp + 500) / 1000 : -1;
    const int rate_kbits =
        tx_real_kbits > 0
            ? tx_real_kbits
            : (tx_tp_th > 0
                   ? tx_tp_th
                   : (tx_phy_tp > 0 ? (tx_phy_tp + 500) / 1000 : -1));
    stats.monitor_mode_link.curr_rate_kbits = clamp_uint16(rate_kbits);
  }
  const bool artosyn_debug_stats_enabled =
      is_openhd_debug_mode_enabled_cached(now_ms);
  if (artosyn_debug_stats_enabled) {
    // Artosyn-specific debug: expose additional RX/TX link metrics via spare
    // fields so ground tools can observe them continuously.
    stats.monitor_mode_link.dummy0 = clamp_int8(rx_mcs);
    const int tx_phy_rate_mbps =
        tx_phy_tp > 0 ? ((tx_phy_tp + 500) / 1000) : -1;
    stats.monitor_mode_link.dummy1 = clamp_int16(tx_phy_rate_mbps);
    const int rx_real_kbits =
        rx_real_tp > 0 ? (rx_real_tp + 500) / 1000 : -1;
    const int rx_rate_kbits =
        rx_real_kbits > 0
            ? rx_real_kbits
            : (rx_tp_th > 0
                   ? rx_tp_th
                   : (rx_phy_tp > 0 ? (rx_phy_tp + 500) / 1000 : -1));
    stats.monitor_mode_link.dummy2 = clamp_int32(rx_rate_kbits);
  } else {
    // Keep legacy semantics (non-Artosyn links use dummy1 for foreign pps).
    // Artosyn does not provide that value, so emit neutral defaults.
    stats.monitor_mode_link.dummy0 = 0;
    stats.monitor_mode_link.dummy1 = 0;
    stats.monitor_mode_link.dummy2 = 0;
  }

  const int64_t last_rx_ts =
      m_last_rx_packet_ts_ms.load(std::memory_order_relaxed);
  const bool rx_ok = last_rx_ts > 0 && (now_ms - last_rx_ts) <= 5000;
  const auto bitfield = openhd::link_statistics::MonitorModeLinkBitfield{
      false, false, false, rx_ok, true, artosyn_debug_stats_enabled, 0};
  stats.monitor_mode_link.bitfield =
      openhd::link_statistics::write_monitor_link_bitfield(bitfield);

  int role = -1;
  int mode = -1;
  int sync_mode = -1;
  int sync_master = -1;
  int cfg_sbmp = -1;
  int rt_sbmp = -1;
  int pair_state = -1;
  std::string peer_mac;
  const bool have_status =
      !first_sample &&
      read_status_extra(&role, &mode, &sync_mode, &sync_master, &cfg_sbmp,
                        &rt_sbmp, nullptr, &pair_state, &peer_mac, nullptr,
                        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                        nullptr);

  const int tx_effective_kbits =
      tx_real_tp > 0 ? tx_real_tp : (tx_tp_th > 0 ? tx_tp_th : tx_phy_tp);
  const int rx_effective_kbits =
      rx_real_tp > 0 ? rx_real_tp : (rx_tp_th > 0 ? rx_tp_th : rx_phy_tp);

  static int64_t s_last_diag_log_ms = 0;
  static bool s_last_rx_ok = true;
  static bool s_last_have_metrics = true;
  static int s_last_pair_state = std::numeric_limits<int>::min();
  const bool link_state_changed =
      !first_sample &&
      ((s_last_rx_ok != rx_ok) || (s_last_have_metrics != have_metrics) ||
       (have_status && s_last_pair_state != pair_state));
  if (link_state_changed) {
    m_console->warn(
        "Artosyn state change: rx_ok={} metrics={} role={} mode={} sync={} "
        "sync_master={} cfg_sbmp=0x{:x} rt_sbmp=0x{:x} pair={} peer={} "
        "tx_mcs={} rx_mcs={} tx_kbit={} rx_kbit={} tx_pps={} rx_pps={} "
        "video_fd={} tele_fd={}",
        rx_ok ? "yes" : "no", have_metrics ? "yes" : "no", role, mode,
        sync_mode, sync_master, cfg_sbmp, rt_sbmp, pair_state, peer_mac,
        tx_mcs, rx_mcs, tx_effective_kbits, rx_effective_kbits, tx_pps,
        rx_pps, m_video_fd, m_telemetry_fd);
    s_last_rx_ok = rx_ok;
    s_last_have_metrics = have_metrics;
    if (have_status) {
      s_last_pair_state = pair_state;
    }
  }
  const bool periodic_diag =
      !first_sample && (now_ms - s_last_diag_log_ms) >= 2000;
  if (periodic_diag && (artosyn_debug_stats_enabled || !rx_ok || !have_metrics)) {
    s_last_diag_log_ms = now_ms;
    m_console->info(
        "Artosyn diag: rx_ok={} metrics={} role={} mode={} sync={} "
        "sync_master={} cfg_sbmp=0x{:x} rt_sbmp=0x{:x} pair={} peer={} "
        "tx_mcs={} rx_mcs={} bw={} tx_real_kbit={} rx_real_kbit={} "
        "tx_phy_kbit={} rx_phy_kbit={} tx_tp_th={} rx_tp_th={} tx_bps={} "
        "rx_bps={} tx_pps={} rx_pps={} tx_tele_bps={} rx_tele_bps={} "
        "video_fd={} tele_fd={}",
        rx_ok ? "yes" : "no", have_metrics ? "yes" : "no", role, mode,
        sync_mode, sync_master, cfg_sbmp, rt_sbmp, pair_state, peer_mac,
        tx_mcs, rx_mcs, bw, tx_real_tp, rx_real_tp, tx_phy_tp, rx_phy_tp,
        tx_tp_th, rx_tp_th, tx_bps, rx_bps, tx_pps, rx_pps, tx_tele_bps,
        rx_tele_bps, m_video_fd, m_telemetry_fd);
  }

  stats.telemetry.curr_tx_bps = clamp_int32(tx_tele_bps);
  stats.telemetry.curr_rx_bps = clamp_int32(rx_tele_bps);
  stats.telemetry.curr_tx_pps =
      clamp_int16(static_cast<int>(tx_tele_pps));
  stats.telemetry.curr_rx_pps =
      clamp_int16(static_cast<int>(rx_tele_pps));
  if (m_profile.is_air) {
    for (int i = 0; i < openhd::non_wb::VideoBitrateMeter::kMaxStreams; ++i) {
      const auto sample = m_video_bitrate_meter.sample_stream(i, now_ms);
      openhd::link_statistics::Xmavlink_openhd_stats_wb_video_air_t air_video{};
      const auto cam_stats = openhd::LinkActionHandler::instance().get_cam_info(i);
      if (sample.total_packets == 0 && cam_stats.measured_bitrate_bps == 0) {
        continue;
      }
      air_video.link_index = static_cast<uint8_t>(i);
      air_video.curr_recommended_bitrate =
          cam_stats.target_bitrate_kbits > 0 ? cam_stats.target_bitrate_kbits
                                             : cam_stats.encoding_bitrate_kbits;
      if (cam_stats.measured_bitrate_bps > 0) {
        air_video.curr_measured_encoder_bitrate = static_cast<int32_t>(
            std::min<uint32_t>(cam_stats.measured_bitrate_bps,
                               static_cast<uint32_t>(
                                   std::numeric_limits<int32_t>::max())));
      } else {
        air_video.curr_measured_encoder_bitrate = 0;
      }
      // No additional link-layer FEC injection on this path.
      air_video.curr_injected_bitrate = clamp_int32(sample.bitrate_bps);
      air_video.dummy2 = clamp_int32(sample.bitrate_bps);
      air_video.curr_injected_pps = sample.packets_per_second;
      air_video.curr_dropped_frames = 0;
      air_video.curr_fec_percentage = 0;
      stats.stats_wb_video_air.push_back(air_video);
    }
  }

  auto& card = stats.cards.at(0);
  card.NON_MAVLINK_CARD_ACTIVE = true;
  card.card_index = 0;
  const auto artosyn_usb = detect_artosyn_usb_info_cached(now_ms);
  // This stats producer only exists for the Artosyn backend. Keep the UI and
  // telemetry identity stable even if the USB sysfs heuristic is incomplete.
  card.card_type = wifi_card_type_to_int(WiFiCardType::ARTOSYN);
  // Use card_sub_type to expose "HS mode" detail to UI if needed later.
  card.card_sub_type = artosyn_usb.hs_mode ? 1 : 0;
  card.tx_active = m_profile.is_air ? 1 : 0;
  card.rx_rssi = -127;
  card.rx_rssi_1 = -127;
  card.rx_rssi_2 = -127;
  card.rx_noise_adapter = 0;
  card.rx_noise_antenna1 = 0;
  card.rx_noise_antenna2 = 0;
  card.rx_signal_quality_adapter = 0;
  card.rx_signal_quality_antenna1 = 0;
  card.rx_signal_quality_antenna2 = 0;
  int snr = -1;
  if (!first_sample) {
    (void)read_quality_metrics(&snr, nullptr, nullptr, nullptr, nullptr);
  }
  if (snr >= 0) {
    card.rx_snr_antenna1 = clamp_int8(snr);
    card.rx_snr_antenna2 = clamp_int8(snr);
  } else {
    card.rx_snr_antenna1 = -128;
    card.rx_snr_antenna2 = -128;
  }
  card.card_temperature = 0;
  card.count_p_received =
      static_cast<uint32_t>(m_rx_total_packets.load());
  card.count_p_injected =
      static_cast<uint32_t>(m_tx_total_packets.load());
  card.curr_rx_packet_loss_perc = 0;
  int pwr_dbm = -1;
  if (!first_sample) {
    (void)read_power_metrics(nullptr, &pwr_dbm);
  }
  card.tx_power_current = pwr_dbm >= 0 ? clamp_int16(pwr_dbm) : 0;
  card.tx_power_armed = 0;
  card.tx_power_disarmed = 0;
  card.curr_status = m_dev ? 0 : 1;

  openhd::LinkActionHandler::instance().update_link_stats(stats);
}

void ArtosynLink::apply_link_settings() {
  if (!m_dev || !m_settings) return;
  const auto& s = m_settings->get_settings();
  auto timed_ioctl_set = [this](uint32_t req, const void* in,
                                const char* label) -> int {
    const auto t0 = std::chrono::steady_clock::now();
    const int ret = bb_ioctl(m_dev, req, const_cast<void*>(in), nullptr);
    const auto dt_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0)
            .count();
    if (ret != 0) {
      m_console->warn("Artosyn apply setting failed: {} req={} ret={} dt={}ms",
                      label, req, ret, dt_ms);
    } else if (dt_ms > 80) {
      m_console->info("Artosyn apply setting slow: {} req={} dt={}ms", label,
                      req, dt_ms);
    }
    return ret;
  };
  m_console->info("Artosyn apply settings begin");

  bb_set_mcs_mode_t mcs_mode{};
  mcs_mode.slot = static_cast<uint8_t>(m_cfg.slot);
  mcs_mode.auto_mode = (s.mcs_mode != 0);
  (void)timed_ioctl_set(BB_SET_MCS_MODE, &mcs_mode, "BB_SET_MCS_MODE");

  if (s.mcs_mode == 0 && s.mcs_value >= 0) {
    bb_set_mcs_t mcs{};
    mcs.slot = static_cast<uint8_t>(m_cfg.slot);
    mcs.mcs = static_cast<uint8_t>(s.mcs_value);
    (void)timed_ioctl_set(BB_SET_MCS, &mcs, "BB_SET_MCS");
  }
  if (s.mcs_min >= 0 || s.mcs_max >= 0) {
    bb_set_mcs_range_in_t range{};
    range.slot = static_cast<uint8_t>(m_cfg.slot);
    range.mcs_min = static_cast<uint8_t>(s.mcs_min >= 0 ? s.mcs_min : 0);
    range.mcs_max = static_cast<uint8_t>(s.mcs_max >= 0 ? s.mcs_max
                                                         : BB_PHY_MCS_MAX);
    (void)timed_ioctl_set(BB_SET_MCS_RANGE, &range, "BB_SET_MCS_RANGE");
  }

  // Select the RF band before applying a channel index. Channel indices are
  // band-specific in the Artosyn firmware; setting channel 11 while the radio
  // is still in 2 GHz mode can silently select a 2.4 GHz channel instead of
  // the intended 5225 MHz entry.
  if (s.band_mode >= 0) {
    bb_set_band_mode_t band_mode{};
    band_mode.auto_mode = (s.band_mode != 0);
    (void)timed_ioctl_set(BB_SET_BAND_MODE, &band_mode, "BB_SET_BAND_MODE");
  }
  if (s.band_value >= 0) {
    bb_set_band_t band{};
    band.target_band = static_cast<uint8_t>(s.band_value);
    if (timed_ioctl_set(BB_SET_BAND, &band, "BB_SET_BAND") == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
  }

  bb_set_bandwidth_mode_t bw_mode{};
  bw_mode.slot = static_cast<uint8_t>(m_cfg.slot);
  bw_mode.mode = (s.bw_mode != 0);
  (void)timed_ioctl_set(BB_SET_BANDWIDTH_MODE, &bw_mode,
                        "BB_SET_BANDWIDTH_MODE");

  if (s.bw_mode == 0 && s.bw_value >= 0) {
    bb_set_bandwidth_t bw{};
    bw.slot = static_cast<uint8_t>(m_cfg.slot);
    bw.bandwidth = static_cast<uint8_t>(s.bw_value);
    bw.dir = BB_DIR_TX;
    (void)timed_ioctl_set(BB_SET_BANDWIDTH, &bw, "BB_SET_BANDWIDTH_TX");
    bw.dir = BB_DIR_RX;
    (void)timed_ioctl_set(BB_SET_BANDWIDTH, &bw, "BB_SET_BANDWIDTH_RX");
  }

  bb_set_chan_mode_t chan_mode{};
  chan_mode.auto_mode = (s.chan_mode != 0);
  if (timed_ioctl_set(BB_SET_CHAN_MODE, &chan_mode, "BB_SET_CHAN_MODE") ==
      0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }

  if (s.chan_mode == 0 && s.chan_index >= 0) {
    bb_set_chan_t chan{};
    chan.chan_dir = BB_DIR_TX;
    chan.chan_index = static_cast<uint8_t>(s.chan_index);
    (void)timed_ioctl_set(BB_SET_CHAN, &chan, "BB_SET_CHAN");
  }

  bb_set_pwr_auto_in_t pwr_auto{};
  pwr_auto.pwr_auto = (s.power_auto != 0);
  (void)timed_ioctl_set(BB_SET_POWER_AUTO, &pwr_auto, "BB_SET_POWER_AUTO");

  if (s.power_auto == 0 && s.tx_power_dbm >= 0) {
    bb_set_pwr_in_t pwr{};
    pwr.usr = static_cast<uint8_t>(m_cfg.slot);
    pwr.pwr = static_cast<uint8_t>(s.tx_power_dbm);
    (void)timed_ioctl_set(BB_SET_POWER, &pwr, "BB_SET_POWER");
  }

  if (s.compliance_mode >= 0) {
    bb_set_compliance_mode_t cmp{};
    cmp.enable = (s.compliance_mode != 0);
    (void)timed_ioctl_set(BB_SET_COMPLIANCE_MODE, &cmp,
                          "BB_SET_COMPLIANCE_MODE");
  }
  if (s.power_mode >= 0) {
    bb_set_pwr_mode_in_t pwr_mode{};
    pwr_mode.pwr_mode = static_cast<uint8_t>(s.power_mode);
    (void)timed_ioctl_set(BB_SET_POWER_MODE, &pwr_mode, "BB_SET_POWER_MODE");
  }
  if (s.lna_mode >= 0) {
    bb_set_lna_mode_t lna_mode{};
    lna_mode.mode = (s.lna_mode != 0);
    (void)timed_ioctl_set(BB_SET_LNA_MODE, &lna_mode, "BB_SET_LNA_MODE");
  }
  if (s.lna_bypass >= 0) {
    bb_set_lna_t lna{};
    lna.lna_bypass = (s.lna_bypass != 0);
    (void)timed_ioctl_set(BB_SET_LNA, &lna, "BB_SET_LNA");
  }

  auto apply_rf = [this, &timed_ioctl_set](int state, bb_rf_path_e path,
                                           bb_dir_e dir) {
    if (state < 0) return;
    bb_set_rf_t rf{};
    rf.rf_path = static_cast<uint8_t>(path);
    rf.dir = static_cast<uint8_t>(dir);
    rf.state = (state != 0);
    const std::string label = "BB_SET_RF path=" + std::to_string(rf.rf_path) +
                              " dir=" + std::to_string(rf.dir);
    (void)timed_ioctl_set(BB_SET_RF, &rf, label.c_str());
  };
  apply_rf(s.rf_a_tx, BB_RF_PATH_A, BB_DIR_TX);
  apply_rf(s.rf_a_rx, BB_RF_PATH_A, BB_DIR_RX);
  apply_rf(s.rf_b_tx, BB_RF_PATH_B, BB_DIR_TX);
  apply_rf(s.rf_b_rx, BB_RF_PATH_B, BB_DIR_RX);
  m_console->info("Artosyn apply settings done");
}

bool ArtosynLink::read_metrics(int* link_state, int* rx_mcs, int* tx_mcs,
                               int* bw, int* phy_tp_kbps, int* real_tp_kbps,
                               int* tx_freq_khz, int* rx_freq_khz,
                               int* rx_bw, int* rx_phy_tp_kbps,
                               int* rx_real_tp_kbps) {
  if (!m_dev) return false;
  const int slot = m_cfg.slot;
  bb_get_status_in_t st_in{};
  bb_get_status_out_t st_out{};
  st_in.user_bmp = (1 << slot);
  if (bb_ioctl(m_dev, BB_GET_STATUS, &st_in, &st_out) != 0) {
    return false;
  }
  if (link_state) *link_state = st_out.link_status[slot].state;
  if (rx_mcs) *rx_mcs = st_out.link_status[slot].rx_mcs;
  if (tx_mcs) *tx_mcs = st_out.user_status[slot].tx_status.mcs;
  if (bw) *bw = st_out.user_status[slot].tx_status.bandwidth;
  if (tx_freq_khz) *tx_freq_khz = st_out.user_status[slot].tx_status.freq_khz;
  if (rx_freq_khz) *rx_freq_khz = st_out.user_status[slot].rx_status.freq_khz;
  if (rx_bw) *rx_bw = st_out.user_status[slot].rx_status.bandwidth;

  if (phy_tp_kbps || real_tp_kbps || rx_phy_tp_kbps || rx_real_tp_kbps) {
    bb_get_throughput_in_t tp_in{};
    bb_get_throughput_out_t tp_out{};
    tp_in.slot = slot;
    tp_in.dir_bmp = (1 << BB_DIR_TX) | (1 << BB_DIR_RX);
    if (bb_ioctl(m_dev, BB_GET_THROUGHPUT, &tp_in, &tp_out) == 0) {
      if (phy_tp_kbps) {
        *phy_tp_kbps = static_cast<int>(
            tp_out.throughput[BB_DIR_TX].phy_throughput);
      }
      if (real_tp_kbps) {
        *real_tp_kbps = static_cast<int>(
            tp_out.throughput[BB_DIR_TX].real_throughput);
      }
      if (rx_phy_tp_kbps) {
        *rx_phy_tp_kbps = static_cast<int>(
            tp_out.throughput[BB_DIR_RX].phy_throughput);
      }
      if (rx_real_tp_kbps) {
        *rx_real_tp_kbps = static_cast<int>(
            tp_out.throughput[BB_DIR_RX].real_throughput);
      }
    }
  }
  return true;
}

bool ArtosynLink::read_quality_metrics(int* snr, int* ldpc_err, int* ldpc_num,
                                       int* gain_a, int* gain_b) {
  if (!m_dev) return false;
  const int slot = m_cfg.slot;
  bb_get_user_quality_in_t q_in{};
  bb_get_user_quality_out_t q_out{};
  q_in.user_bmp = (1 << slot);
  q_in.average = 0;
  if (bb_ioctl(m_dev, BB_GET_USER_QUALITY, &q_in, &q_out) != 0) {
    return false;
  }
  const auto& q = q_out.qualities[slot];
  if (snr) *snr = q.snr;
  if (ldpc_err) *ldpc_err = q.ldpc_err;
  if (ldpc_num) *ldpc_num = q.ldpc_num;
  if (gain_a) *gain_a = q.gain_a;
  if (gain_b) *gain_b = q.gain_b;
  return true;
}

bool ArtosynLink::read_power_metrics(int* power_auto, int* power_dbm) {
  if (!m_dev) return false;
  bool ok = false;
  if (power_auto) {
    bb_get_pwr_auto_out_t pwr_auto{};
    if (bb_ioctl(m_dev, BB_GET_POWER_AUTO, nullptr, &pwr_auto) == 0) {
      *power_auto = pwr_auto.pwr_auto;
      ok = true;
    }
  }
  if (power_dbm) {
    bb_get_cur_pwr_in_t pwr_in{};
    bb_get_cur_pwr_out_t pwr_out{};
    pwr_in.usr = static_cast<uint8_t>(m_cfg.slot);
    if (bb_ioctl(m_dev, BB_GET_CUR_POWER, &pwr_in, &pwr_out) == 0) {
      *power_dbm = pwr_out.pwr;
      ok = true;
    }
  }
  return ok;
}

bool ArtosynLink::read_chan_metrics(int* chan_auto, int* work_chan,
                                    int* work_freq_khz) {
  if (!m_dev) return false;
  bb_get_chan_info_out_t chan_out{};
  if (bb_ioctl(m_dev, BB_GET_CHAN_INFO, nullptr, &chan_out) != 0) {
    return false;
  }
  if (chan_auto) *chan_auto = chan_out.auto_mode;
  if (work_chan) *work_chan = chan_out.work_chan;
  if (work_freq_khz) {
    if (chan_out.work_chan < chan_out.chan_num) {
      *work_freq_khz = static_cast<int>(
          chan_out.freq[chan_out.work_chan]);
    } else {
      *work_freq_khz = -1;
    }
  }
  return true;
}

bool ArtosynLink::read_band_metrics(int* band_auto, int* work_band) {
  if (!m_dev) return false;
  bb_get_band_info_out_t out{};
  if (bb_ioctl(m_dev, BB_GET_BAND_INFO, nullptr, &out) != 0) {
    return false;
  }
  if (band_auto) *band_auto = out.band_mode;
  if (work_band) *work_band = out.work_band;
  return true;
}

bool ArtosynLink::read_rf_metrics(int* a_tx, int* a_rx, int* b_tx, int* b_rx) {
  if (!m_dev) return false;
  bb_get_rf_out_t out{};
  if (bb_ioctl(m_dev, BB_GET_RF, nullptr, &out) != 0) {
    return false;
  }
  if (a_tx) *a_tx = out.path_a_tx_state;
  if (a_rx) *a_rx = out.path_a_rx_state;
  if (b_tx) *b_tx = out.path_b_tx_state;
  if (b_rx) *b_rx = out.path_b_rx_state;
  return true;
}

bool ArtosynLink::read_sys_info(uint64_t* uptime_ms, std::string* soft_ver,
                                std::string* hw_ver, std::string* fw_ver,
                                std::string* compile_time) {
  if (!m_dev) return false;
  bb_get_sys_info_out_t out{};
  if (bb_ioctl(m_dev, BB_GET_SYS_INFO, nullptr, &out) != 0) {
    return false;
  }
  if (uptime_ms) *uptime_ms = out.uptime;
  if (soft_ver) *soft_ver = out.soft_ver;
  if (hw_ver) *hw_ver = out.hardware_ver;
  if (fw_ver) *fw_ver = out.firmware_ver;
  if (compile_time) *compile_time = out.compile_time;
  return true;
}

bool ArtosynLink::read_runsys(int* runsys_id) {
  if (!m_dev) return false;
  bb_get_runsys_out_t out{};
  if (bb_ioctl(m_dev, BB_GET_RUN_SYS, nullptr, &out) != 0) {
    return false;
  }
  if (runsys_id) *runsys_id = static_cast<int>(out.runsys_id);
  return true;
}

bool ArtosynLink::read_mcs_throughput(int* tx_tp_kbps, int* rx_tp_kbps) {
  if (!m_dev) return false;
  const int slot = m_cfg.slot;
  bool ok = false;
  if (tx_tp_kbps) {
    bb_get_mcs_in_t in{};
    bb_get_mcs_out_t out{};
    in.dir = BB_DIR_TX;
    in.slot = static_cast<uint8_t>(slot);
    if (bb_ioctl(m_dev, BB_GET_MCS, &in, &out) == 0) {
      *tx_tp_kbps = static_cast<int>(out.throughput);
      ok = true;
    }
  }
  if (rx_tp_kbps) {
    bb_get_mcs_in_t in{};
    bb_get_mcs_out_t out{};
    in.dir = BB_DIR_RX;
    in.slot = static_cast<uint8_t>(slot);
    if (bb_ioctl(m_dev, BB_GET_MCS, &in, &out) == 0) {
      *rx_tp_kbps = static_cast<int>(out.throughput);
      ok = true;
    }
  }
  return ok;
}

bool ArtosynLink::read_peer_quality(int* snr, int* ldpc_err, int* ldpc_num,
                                    int* gain_a, int* gain_b) {
  if (!m_dev) return false;
  const int slot = m_cfg.slot;
  bb_get_peer_quality_in_t in{};
  bb_get_peer_quality_out_t out{};
  in.slot_bmp = (1 << slot);
  in.arverage = 0;
  if (bb_ioctl(m_dev, BB_GET_PEER_QUALITY, &in, &out) != 0) {
    return false;
  }
  const auto& q = out.qualities[slot];
  if (snr) *snr = q.snr;
  if (ldpc_err) *ldpc_err = q.ldpc_err;
  if (ldpc_num) *ldpc_num = q.ldpc_num;
  if (gain_a) *gain_a = q.gain_a;
  if (gain_b) *gain_b = q.gain_b;
  return true;
}

bool ArtosynLink::read_ap_time(int* ap_time_ms) {
  if (!m_dev) return false;
  bb_get_ap_time_out_t out{};
  if (bb_ioctl(m_dev, BB_GET_AP_TIME, nullptr, &out) != 0) {
    return false;
  }
  if (ap_time_ms) *ap_time_ms = static_cast<int>(out.timestamp);
  return true;
}

bool ArtosynLink::read_1v1_info(bb_info_t* self, bb_info_t* peer) {
  if (!m_dev) return false;
  bb_get_1v1_info_in_t in{};
  bb_get_1v1_info_out_t out{};
  in.frame_num = 0;
  if (bb_ioctl(m_dev, BB_GET_1V1_INFO, &in, &out) != 0) {
    return false;
  }
  if (self) *self = out.self;
  if (peer) *peer = out.peer;
  return true;
}

bool ArtosynLink::read_sock_info(int port, bb_sock_info_t* out_info) {
  if (!m_dev || port < 0 || port >= BB_SOCK_INFO_NUM || !out_info) {
    return false;
  }
  bb_get_sock_info_in_t in{};
  bb_get_sock_info_out_t out{};
  in.slot = static_cast<uint8_t>(m_cfg.slot);
  in.port = static_cast<int8_t>(port);
  if (bb_ioctl(m_dev, BB_GET_SOCK_INFO, &in, &out) != 0) {
    return false;
  }
  if ((out.port_bmp & (1 << port)) == 0) {
    return false;
  }
  *out_info = out.sock_info[port];
  return true;
}

bool ArtosynLink::read_status_extra(int* role, int* mode, int* sync_mode,
                                    int* sync_master, int* cfg_sbmp,
                                    int* rt_sbmp, std::string* local_mac,
                                    int* pair_state, std::string* peer_mac,
                                    int* tx_rf_mode, int* rx_rf_mode,
                                    int* tx_tintlv_en, int* rx_tintlv_en,
                                    int* tx_tintlv_num, int* rx_tintlv_num,
                                    int* tx_tintlv_len, int* rx_tintlv_len) {
  if (!m_dev) return false;
  const int slot = m_cfg.slot;
  bb_get_status_in_t st_in{};
  bb_get_status_out_t st_out{};
  st_in.user_bmp = (1 << slot);
  if (bb_ioctl(m_dev, BB_GET_STATUS, &st_in, &st_out) != 0) {
    return false;
  }
  if (role) *role = st_out.role;
  if (mode) *mode = st_out.mode;
  if (sync_mode) *sync_mode = st_out.sync_mode;
  if (sync_master) *sync_master = st_out.sync_master;
  if (cfg_sbmp) *cfg_sbmp = st_out.cfg_sbmp;
  if (rt_sbmp) *rt_sbmp = st_out.rt_sbmp;
  if (local_mac) *local_mac = mac_to_string(st_out.mac);

  if (pair_state) *pair_state = st_out.link_status[slot].pair_state;
  if (peer_mac) *peer_mac = mac_to_string(st_out.link_status[slot].peer_mac);

  if (tx_rf_mode) *tx_rf_mode = st_out.user_status[slot].tx_status.rf_mode;
  if (rx_rf_mode) *rx_rf_mode = st_out.user_status[slot].rx_status.rf_mode;
  if (tx_tintlv_en)
    *tx_tintlv_en = st_out.user_status[slot].tx_status.tintlv_enable;
  if (rx_tintlv_en)
    *rx_tintlv_en = st_out.user_status[slot].rx_status.tintlv_enable;
  if (tx_tintlv_num)
    *tx_tintlv_num = st_out.user_status[slot].tx_status.tintlv_num;
  if (rx_tintlv_num)
    *rx_tintlv_num = st_out.user_status[slot].rx_status.tintlv_num;
  if (tx_tintlv_len)
    *tx_tintlv_len = st_out.user_status[slot].tx_status.tintlv_len;
  if (rx_tintlv_len)
    *rx_tintlv_len = st_out.user_status[slot].rx_status.tintlv_len;
  return true;
}

std::vector<openhd::Setting> ArtosynLink::get_all_settings() {
  using namespace openhd;
  std::vector<Setting> ret;
  if (!m_settings) return ret;

  auto cb_addr = [this](std::string, std::string value) {
    m_settings->unsafe_get_settings().addr = value;
    m_settings->persist();
    return true;
  };
  ret.push_back(
      Setting{AR_ADDR, StringSetting{m_settings->get_settings().addr, cb_addr}});

  auto cb_port = [this](std::string, int value) {
    m_settings->unsafe_get_settings().port = value;
    m_settings->persist();
    return true;
  };
  ret.push_back(
      Setting{AR_PORT, IntSetting{m_settings->get_settings().port, cb_port}});

  auto cb_slot = [this](std::string, int value) {
    m_settings->unsafe_get_settings().slot = value;
    m_settings->persist();
    return true;
  };
  ret.push_back(
      Setting{AR_SLOT, IntSetting{m_settings->get_settings().slot, cb_slot}});

  auto cb_vport = [this](std::string, int value) {
    m_settings->unsafe_get_settings().video_port = value;
    m_settings->persist();
    return true;
  };
  ret.push_back(Setting{
      AR_VPORT, IntSetting{m_settings->get_settings().video_port, cb_vport}});

  auto cb_tport = [this](std::string, int value) {
    m_settings->unsafe_get_settings().telemetry_port = value;
    m_settings->persist();
    return true;
  };
  ret.push_back(Setting{AR_TPORT, IntSetting{m_settings->get_settings().telemetry_port,
                                             cb_tport}});

  auto cb_datagram = [this](std::string, int value) {
    m_settings->unsafe_get_settings().use_datagram = value ? 1 : 0;
    m_settings->persist();
    return true;
  };
  ret.push_back(Setting{AR_DGRAM, IntSetting{m_settings->get_settings().use_datagram,
                                             cb_datagram}});

  auto cb_rx = [this](std::string, int value) {
    m_settings->unsafe_get_settings().rx_buf_size = value;
    m_settings->persist();
    return true;
  };
  ret.push_back(
      Setting{AR_RXBUF, IntSetting{m_settings->get_settings().rx_buf_size,
                                   cb_rx}});

  auto cb_tx = [this](std::string, int value) {
    m_settings->unsafe_get_settings().tx_buf_size = value;
    m_settings->persist();
    return true;
  };
  ret.push_back(
      Setting{AR_TXBUF, IntSetting{m_settings->get_settings().tx_buf_size,
                                   cb_tx}});

  auto cb_to = [this](std::string, int value) {
    m_settings->unsafe_get_settings().read_timeout_ms = value;
    m_settings->persist();
    return true;
  };
  ret.push_back(
      Setting{AR_RDTMO, IntSetting{m_settings->get_settings().read_timeout_ms,
                                   cb_to}});

  ret.push_back(create_read_only_int(AR_DMN_AUTO, 0));
  ret.push_back(
      create_read_only_string(AR_DMN_CMD, "managed-by-sysutils (daemon+tunnel)"));

  auto cb_mcs_mode = [this](std::string, int value) {
    m_settings->unsafe_get_settings().mcs_mode = value ? 1 : 0;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_MCS_MD, IntSetting{m_settings->get_settings().mcs_mode,
                                    cb_mcs_mode}});

  auto cb_mcs_val = [this](std::string, int value) {
    m_settings->unsafe_get_settings().mcs_value = value;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_MCS_VAL, IntSetting{m_settings->get_settings().mcs_value,
                                     cb_mcs_val}});

  auto cb_mcs_min = [this](std::string, int value) {
    m_settings->unsafe_get_settings().mcs_min = value;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_MCS_MIN, IntSetting{m_settings->get_settings().mcs_min,
                                     cb_mcs_min}});

  auto cb_mcs_max = [this](std::string, int value) {
    m_settings->unsafe_get_settings().mcs_max = value;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_MCS_MAX, IntSetting{m_settings->get_settings().mcs_max,
                                     cb_mcs_max}});

  auto cb_bw_mode = [this](std::string, int value) {
    m_settings->unsafe_get_settings().bw_mode = value ? 1 : 0;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_BW_MD, IntSetting{m_settings->get_settings().bw_mode,
                                   cb_bw_mode}});

  auto cb_bw_val = [this](std::string, int value) {
    m_settings->unsafe_get_settings().bw_value = value;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_BW_VAL, IntSetting{m_settings->get_settings().bw_value,
                                    cb_bw_val}});

  auto cb_chan_mode = [this](std::string, int value) {
    m_settings->unsafe_get_settings().chan_mode = value ? 1 : 0;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_CHN_MD, IntSetting{m_settings->get_settings().chan_mode,
                                    cb_chan_mode}});

  auto cb_chan_idx = [this](std::string, int value) {
    m_settings->unsafe_get_settings().chan_index = value;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_CHN_IDX, IntSetting{m_settings->get_settings().chan_index,
                                     cb_chan_idx}});

  auto cb_pwr_auto = [this](std::string, int value) {
    m_settings->unsafe_get_settings().power_auto = value ? 1 : 0;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_PWR_ATO, IntSetting{m_settings->get_settings().power_auto,
                                     cb_pwr_auto}});

  auto cb_pwr_dbm = [this](std::string, int value) {
    m_settings->unsafe_get_settings().tx_power_dbm = value;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_PWR_DBM, IntSetting{m_settings->get_settings().tx_power_dbm,
                                     cb_pwr_dbm}});

  auto cb_band_mode = [this](std::string, int value) {
    m_settings->unsafe_get_settings().band_mode = value ? 1 : 0;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_BND_MD, IntSetting{m_settings->get_settings().band_mode,
                                    cb_band_mode}});

  auto cb_band_val = [this](std::string, int value) {
    m_settings->unsafe_get_settings().band_value = value;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_BND_VAL, IntSetting{m_settings->get_settings().band_value,
                                     cb_band_val}});

  auto cb_cmp_mode = [this](std::string, int value) {
    m_settings->unsafe_get_settings().compliance_mode = value ? 1 : 0;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_CMP_MD, IntSetting{m_settings->get_settings().compliance_mode,
                                    cb_cmp_mode}});

  auto cb_pwr_mode = [this](std::string, int value) {
    m_settings->unsafe_get_settings().power_mode = value;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_PWR_MD, IntSetting{m_settings->get_settings().power_mode,
                                    cb_pwr_mode}});

  auto cb_lna_mode = [this](std::string, int value) {
    m_settings->unsafe_get_settings().lna_mode = value ? 1 : 0;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_LNA_MD, IntSetting{m_settings->get_settings().lna_mode,
                                    cb_lna_mode}});

  auto cb_lna_bp = [this](std::string, int value) {
    m_settings->unsafe_get_settings().lna_bypass = value ? 1 : 0;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_LNA_BP, IntSetting{m_settings->get_settings().lna_bypass,
                                    cb_lna_bp}});

  auto cb_rf_atx = [this](std::string, int value) {
    m_settings->unsafe_get_settings().rf_a_tx = value ? 1 : 0;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_RF_ATX, IntSetting{m_settings->get_settings().rf_a_tx,
                                    cb_rf_atx}});

  auto cb_rf_arx = [this](std::string, int value) {
    m_settings->unsafe_get_settings().rf_a_rx = value ? 1 : 0;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_RF_ARX, IntSetting{m_settings->get_settings().rf_a_rx,
                                    cb_rf_arx}});

  auto cb_rf_btx = [this](std::string, int value) {
    m_settings->unsafe_get_settings().rf_b_tx = value ? 1 : 0;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_RF_BTX, IntSetting{m_settings->get_settings().rf_b_tx,
                                    cb_rf_btx}});

  auto cb_rf_brx = [this](std::string, int value) {
    m_settings->unsafe_get_settings().rf_b_rx = value ? 1 : 0;
    m_settings->persist(false);
    apply_link_settings();
    return true;
  };
  ret.push_back(
      Setting{AR_RF_BRX, IntSetting{m_settings->get_settings().rf_b_rx,
                                    cb_rf_brx}});

  if (!expose_artosyn_diagnostic_settings()) {
    return ret;
  }
  m_console->warn(
      "OHD_ARTOSYN_DIAG_PARAMS=1 enabled: exposing live Artosyn diagnostic "
      "parameters during settings registration.");

  int link_state = -1;
  int rx_mcs = -1;
  int tx_mcs = -1;
  int bw = -1;
  int rx_bw = -1;
  int phy_tp = -1;
  int real_tp = -1;
  int tx_freq_khz = -1;
  int rx_freq_khz = -1;
  (void)read_metrics(&link_state, &rx_mcs, &tx_mcs, &bw, &phy_tp, &real_tp,
                     &tx_freq_khz, &rx_freq_khz, &rx_bw, nullptr, nullptr);
  ret.push_back(create_read_only_int(AR_LK_STATE, link_state));
  ret.push_back(create_read_only_int(AR_RX_MCS, rx_mcs));
  ret.push_back(create_read_only_int(AR_TX_MCS, tx_mcs));
  ret.push_back(create_read_only_int(AR_BW, bw));
  ret.push_back(create_read_only_int(AR_RX_BW, rx_bw));
  ret.push_back(create_read_only_int(AR_PHY_TP, phy_tp));
  ret.push_back(create_read_only_int(AR_REAL_TP, real_tp));
  ret.push_back(create_read_only_int(AR_TX_FREQ, tx_freq_khz));
  ret.push_back(create_read_only_int(AR_RX_FREQ, rx_freq_khz));

  int role = -1;
  int mode = -1;
  int sync_mode = -1;
  int sync_master = -1;
  int cfg_sbmp = -1;
  int rt_sbmp = -1;
  std::string local_mac;
  int pair_state = -1;
  std::string peer_mac;
  int tx_rf_mode = -1;
  int rx_rf_mode = -1;
  int tx_tintlv_en = -1;
  int rx_tintlv_en = -1;
  int tx_tintlv_num = -1;
  int rx_tintlv_num = -1;
  int tx_tintlv_len = -1;
  int rx_tintlv_len = -1;
  if (read_status_extra(&role, &mode, &sync_mode, &sync_master, &cfg_sbmp,
                        &rt_sbmp, &local_mac, &pair_state, &peer_mac,
                        &tx_rf_mode, &rx_rf_mode, &tx_tintlv_en,
                        &rx_tintlv_en, &tx_tintlv_num, &rx_tintlv_num,
                        &tx_tintlv_len, &rx_tintlv_len)) {
    ret.push_back(create_read_only_int(AR_ROLE, role));
    ret.push_back(create_read_only_int(AR_MODE, mode));
    ret.push_back(create_read_only_int(AR_SYNC, sync_mode));
    ret.push_back(create_read_only_int(AR_SYNC_M, sync_master));
    ret.push_back(create_read_only_int(AR_CFG_SBM, cfg_sbmp));
    ret.push_back(create_read_only_int(AR_RT_SBM, rt_sbmp));
    if (!local_mac.empty()) {
      ret.push_back(create_read_only_string(AR_LMAC, local_mac));
    }
    ret.push_back(create_read_only_int(AR_PAIR, pair_state));
    if (!peer_mac.empty()) {
      ret.push_back(create_read_only_string(AR_PMAC, peer_mac));
    }
    ret.push_back(create_read_only_int(AR_TX_RFM, tx_rf_mode));
    ret.push_back(create_read_only_int(AR_RX_RFM, rx_rf_mode));
    ret.push_back(create_read_only_int(AR_TX_TEN, tx_tintlv_en));
    ret.push_back(create_read_only_int(AR_RX_TEN, rx_tintlv_en));
    ret.push_back(create_read_only_int(AR_TX_TNM, tx_tintlv_num));
    ret.push_back(create_read_only_int(AR_RX_TNM, rx_tintlv_num));
    ret.push_back(create_read_only_int(AR_TX_TLN, tx_tintlv_len));
    ret.push_back(create_read_only_int(AR_RX_TLN, rx_tintlv_len));
  }

  int snr = -1;
  int ldpc_err = -1;
  int ldpc_num = -1;
  int gain_a = -1;
  int gain_b = -1;
  if (read_quality_metrics(&snr, &ldpc_err, &ldpc_num, &gain_a, &gain_b)) {
    ret.push_back(create_read_only_int(AR_SNR, snr));
    ret.push_back(create_read_only_int(AR_LDPC_E, ldpc_err));
    ret.push_back(create_read_only_int(AR_LDPC_N, ldpc_num));
    ret.push_back(create_read_only_int(AR_GAIN_A, gain_a));
    ret.push_back(create_read_only_int(AR_GAIN_B, gain_b));
  }

  int pwr_dbm = -1;
  if (read_power_metrics(nullptr, &pwr_dbm)) {
    if (pwr_dbm >= 0) {
      ret.push_back(create_read_only_int(AR_PWR_CUR, pwr_dbm));
    }
  }

  int chan_auto = -1;
  int work_chan = -1;
  int work_freq_khz = -1;
  if (read_chan_metrics(&chan_auto, &work_chan, &work_freq_khz)) {
    ret.push_back(create_read_only_int(AR_CHN_AUT, chan_auto));
    ret.push_back(create_read_only_int(AR_CHN_CUR, work_chan));
    ret.push_back(create_read_only_int(AR_CHN_FK, work_freq_khz));
  }

  int band_auto = -1;
  int work_band = -1;
  if (read_band_metrics(&band_auto, &work_band)) {
    ret.push_back(create_read_only_int(AR_BND_AUT, band_auto));
    ret.push_back(create_read_only_int(AR_BND_CUR, work_band));
  }

  int a_tx = -1;
  int a_rx = -1;
  int b_tx = -1;
  int b_rx = -1;
  if (read_rf_metrics(&a_tx, &a_rx, &b_tx, &b_rx)) {
    ret.push_back(create_read_only_int(AR_RF_ATX_R, a_tx));
    ret.push_back(create_read_only_int(AR_RF_ARX_R, a_rx));
    ret.push_back(create_read_only_int(AR_RF_BTX_R, b_tx));
    ret.push_back(create_read_only_int(AR_RF_BRX_R, b_rx));
  }

  uint64_t uptime_ms = 0;
  std::string soft_ver;
  std::string hw_ver;
  std::string fw_ver;
  std::string compile_time;
  if (read_sys_info(&uptime_ms, &soft_ver, &hw_ver, &fw_ver, &compile_time)) {
    ret.push_back(create_read_only_int(AR_UPTIME,
                                       static_cast<int>(uptime_ms / 1000)));
    ret.push_back(create_read_only_string(AR_SW_VER, soft_ver));
    ret.push_back(create_read_only_string(AR_HW_VER, hw_ver));
    ret.push_back(create_read_only_string(AR_FW_VER, fw_ver));
    ret.push_back(create_read_only_string(AR_CMP_TM, compile_time));
  }

  int runsys_id = -1;
  if (read_runsys(&runsys_id)) {
    ret.push_back(create_read_only_int(AR_RUNSYS, runsys_id));
  }

  int tx_tp_th = -1;
  int rx_tp_th = -1;
  (void)read_mcs_throughput(&tx_tp_th, &rx_tp_th);
  ret.push_back(create_read_only_int(AR_TX_TPTH, tx_tp_th));
  ret.push_back(create_read_only_int(AR_RX_TPTH, rx_tp_th));

  int p_snr = -1;
  int p_ldpc_err = -1;
  int p_ldpc_num = -1;
  int p_gain_a = -1;
  int p_gain_b = -1;
  if (read_peer_quality(&p_snr, &p_ldpc_err, &p_ldpc_num, &p_gain_a,
                        &p_gain_b)) {
    ret.push_back(create_read_only_int(AR_P_SNR, p_snr));
    ret.push_back(create_read_only_int(AR_P_LDPC_E, p_ldpc_err));
    ret.push_back(create_read_only_int(AR_P_LDPC_N, p_ldpc_num));
    ret.push_back(create_read_only_int(AR_P_GAIN_A, p_gain_a));
    ret.push_back(create_read_only_int(AR_P_GAIN_B, p_gain_b));
  }

  int ap_time = -1;
  if (read_ap_time(&ap_time)) {
    ret.push_back(create_read_only_int(AR_AP_TIME, ap_time));
  }

  bb_info_t self{};
  bb_info_t peer{};
  if (read_1v1_info(&self, &peer)) {
    ret.push_back(create_read_only_int(AR1_S_SNR, self.snr));
    ret.push_back(create_read_only_int(AR1_S_LDPT, self.ldpc_tlv_err_ratio));
    ret.push_back(create_read_only_int(AR1_S_LDPN, self.ldpc_num_err_ratio));
    ret.push_back(create_read_only_int(AR1_S_GNA, self.gain_a));
    ret.push_back(create_read_only_int(AR1_S_GNB, self.gain_b));
    ret.push_back(create_read_only_int(AR1_S_MCS, self.tx_mcs));
    ret.push_back(create_read_only_int(AR1_S_CHN, self.tx_chan));
    ret.push_back(create_read_only_int(AR1_S_PWR, self.tx_power));
    ret.push_back(create_read_only_int(AR1_S_LNI, self.lna_inner_bypass));
    ret.push_back(create_read_only_int(AR1_S_LNF, self.lna_fem_bypass));
    ret.push_back(create_read_only_int(AR1_S_1TX, self.rf_1tx));
    ret.push_back(create_read_only_int(AR1_S_TFK,
                                       static_cast<int>(self.tx_freq_khz)));
    ret.push_back(
        create_read_only_int(AR1_S_LSN, self.lfs_low_band.chan_snr));
    ret.push_back(create_read_only_int(AR1_S_LGA,
                                       self.lfs_low_band.gain_a));
    ret.push_back(create_read_only_int(AR1_S_LGB,
                                       self.lfs_low_band.gain_b));
    ret.push_back(
        create_read_only_int(AR1_S_HSN, self.lfs_high_band.chan_snr));
    ret.push_back(create_read_only_int(AR1_S_HGA,
                                       self.lfs_high_band.gain_a));
    ret.push_back(create_read_only_int(AR1_S_HGB,
                                       self.lfs_high_band.gain_b));

    ret.push_back(create_read_only_int(AR1_P_SNR, peer.snr));
    ret.push_back(create_read_only_int(AR1_P_LDPT, peer.ldpc_tlv_err_ratio));
    ret.push_back(create_read_only_int(AR1_P_LDPN, peer.ldpc_num_err_ratio));
    ret.push_back(create_read_only_int(AR1_P_GNA, peer.gain_a));
    ret.push_back(create_read_only_int(AR1_P_GNB, peer.gain_b));
    ret.push_back(create_read_only_int(AR1_P_MCS, peer.tx_mcs));
    ret.push_back(create_read_only_int(AR1_P_CHN, peer.tx_chan));
    ret.push_back(create_read_only_int(AR1_P_PWR, peer.tx_power));
    ret.push_back(create_read_only_int(AR1_P_LNI, peer.lna_inner_bypass));
    ret.push_back(create_read_only_int(AR1_P_LNF, peer.lna_fem_bypass));
    ret.push_back(create_read_only_int(AR1_P_1TX, peer.rf_1tx));
    ret.push_back(create_read_only_int(AR1_P_TFK,
                                       static_cast<int>(peer.tx_freq_khz)));
    ret.push_back(
        create_read_only_int(AR1_P_LSN, peer.lfs_low_band.chan_snr));
    ret.push_back(create_read_only_int(AR1_P_LGA,
                                       peer.lfs_low_band.gain_a));
    ret.push_back(create_read_only_int(AR1_P_LGB,
                                       peer.lfs_low_band.gain_b));
    ret.push_back(
        create_read_only_int(AR1_P_HSN, peer.lfs_high_band.chan_snr));
    ret.push_back(create_read_only_int(AR1_P_HGA,
                                       peer.lfs_high_band.gain_a));
    ret.push_back(create_read_only_int(AR1_P_HGB,
                                       peer.lfs_high_band.gain_b));
  }

  bb_sock_info_t vinfo{};
  if (read_sock_info(m_cfg.video_port, &vinfo)) {
    ret.push_back(create_read_only_int(AR_V_RX_AV,
                                       vinfo.uni_info[BB_DIR_RX].available));
    ret.push_back(create_read_only_int(AR_V_RX_OV,
                                       vinfo.uni_info[BB_DIR_RX].overflow_cnt));
    ret.push_back(create_read_only_int(AR_V_RX_BS,
                                       static_cast<int>(
                                           vinfo.uni_info[BB_DIR_RX].buf_size)));
    ret.push_back(create_read_only_int(AR_V_RX_DS,
                                       static_cast<int>(
                                           vinfo.uni_info[BB_DIR_RX].data_size)));
    ret.push_back(create_read_only_int(AR_V_TX_AV,
                                       vinfo.uni_info[BB_DIR_TX].available));
    ret.push_back(create_read_only_int(AR_V_TX_OV,
                                       vinfo.uni_info[BB_DIR_TX].overflow_cnt));
    ret.push_back(create_read_only_int(AR_V_TX_BS,
                                       static_cast<int>(
                                           vinfo.uni_info[BB_DIR_TX].buf_size)));
    ret.push_back(create_read_only_int(AR_V_TX_DS,
                                       static_cast<int>(
                                           vinfo.uni_info[BB_DIR_TX].data_size)));
  }

  bb_sock_info_t tinfo{};
  if (read_sock_info(m_cfg.telemetry_port, &tinfo)) {
    ret.push_back(create_read_only_int(AR_T_RX_AV,
                                       tinfo.uni_info[BB_DIR_RX].available));
    ret.push_back(create_read_only_int(AR_T_RX_OV,
                                       tinfo.uni_info[BB_DIR_RX].overflow_cnt));
    ret.push_back(create_read_only_int(AR_T_RX_BS,
                                       static_cast<int>(
                                           tinfo.uni_info[BB_DIR_RX].buf_size)));
    ret.push_back(create_read_only_int(AR_T_RX_DS,
                                       static_cast<int>(
                                           tinfo.uni_info[BB_DIR_RX].data_size)));
    ret.push_back(create_read_only_int(AR_T_TX_AV,
                                       tinfo.uni_info[BB_DIR_TX].available));
    ret.push_back(create_read_only_int(AR_T_TX_OV,
                                       tinfo.uni_info[BB_DIR_TX].overflow_cnt));
    ret.push_back(create_read_only_int(AR_T_TX_BS,
                                       static_cast<int>(
                                           tinfo.uni_info[BB_DIR_TX].buf_size)));
    ret.push_back(create_read_only_int(AR_T_TX_DS,
                                       static_cast<int>(
                                           tinfo.uni_info[BB_DIR_TX].data_size)));
  }

  return ret;
}
