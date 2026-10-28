#include "ground_config.h"
#include "default_keys.h"
#include "openhd_ncurses_ui.h"
#include "openhd_udp.h"  // Winsock before any Windows headers.
#ifdef OPENHD_ENABLE_GUI
#include "openhd_gui.h"
#endif
#include <libusb-1.0/libusb.h>
#include <csignal>
#include <cctype>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <set>
#include "WBStreamRx.h"
#include "WBStreamTx.h"
#include "encryption/EncryptionFsUtils.h"
#include "openhd_global_constants.hpp"

namespace {
volatile std::sig_atomic_t stopping = 0;
void stop(int) { stopping = 1; }
void help() {
  std::cout << "OpenHD portable ground backend (Devourer USB)\n"
    "  No arguments: saved settings and interactive OpenHD dashboard\n"
    "  --configure                    Open ground settings\n"
    "  --config FILE                  Use another saved profile\n"
    "  --no-ui                        Headless operation\n"
    "  --list                         List USB addresses (no device reset)\n"
    "  --device devourer-usb-BUS-ADDR  Select USB adapter; repeat for diversity\n"
    "  --usb VID:PID                  Select a single adapter by stable USB IDs\n"
    "  --keypair FILE                 128-byte OpenHD keypair matching air\n"
    "  --frequency MHZ                Radio frequency (default 5180)\n"
    "  --width MHZ                    5, 10, 20, 40 or 80 (default 20)\n"
    "  --destination IPv4             Forwarding destination (default 127.0.0.1)\n"
    "  --video-port PORT              Primary video output (default 5600)\n"
    "  --video2-port PORT             Secondary video output (default 5601)\n"
    "  --telemetry-port PORT          Telemetry output (default 14550)\n"
    "  --uplink-port PORT             Localhost telemetry input (default 14551)\n"
    "Ctrl+C stops reception. Requires a Devourer-supported USB radio and WinUSB on Windows.\n";
}
int number(const std::string& value, int minimum, int maximum) {
  std::size_t used = 0;
  const int result = std::stoi(value, &used);
  if (used != value.size() || result < minimum || result > maximum)
    throw std::invalid_argument("Value out of range: " + value);
  return result;
}
void list_usb() {
  libusb_context* context = nullptr;
  if (libusb_init(&context) != 0) throw std::runtime_error("libusb initialization failed");
  libusb_device** devices = nullptr;
  const auto count = libusb_get_device_list(context, &devices);
  if (count < 0) { libusb_exit(context); throw std::runtime_error("USB enumeration failed"); }
  std::cout << "USB devices (listing does not imply Devourer chipset support):\n";
  for (decltype(libusb_get_device_list(context, &devices)) i = 0; i < count; ++i) {
    libusb_device_descriptor descriptor{};
    if (libusb_get_device_descriptor(devices[i], &descriptor) != 0) continue;
    std::cout << "devourer-usb-" << int(libusb_get_bus_number(devices[i])) << '-'
              << int(libusb_get_device_address(devices[i])) << "  VID:PID "
              << std::hex << std::setfill('0') << std::setw(4) << descriptor.idVendor
              << ':' << std::setw(4) << descriptor.idProduct << std::dec << '\n';
  }
  libusb_free_device_list(devices, 1);
  libusb_exit(context);
}
}


namespace {
using openhd::ground::Config;
int run_backend(const Config& c, std::atomic<bool>& requested_stop,
                const std::function<void(const openhd::ui::DashboardStatus&)>& publish) {
    openhd::ground::validate_config(c);
    const auto& keyfile = c.keyfile; const auto& destination = c.destination;
    const int frequency = c.frequency, width = c.width, video_port = c.video_port,
      video2_port = c.video2_port, telemetry_port = c.telemetry_port, uplink_port = c.uplink_port;
    const auto devices = c.devices.empty() ? openhd::ground::discover_radios() : c.devices;
    if (devices.empty()) throw std::runtime_error("No Devourer USB radio found. Connect an adapter and press R to retry.");
    std::vector<wifibroadcast::WifiCard> cards;
    for (const auto& device : devices) cards.push_back({device, 1});
    const auto keypair = keyfile.empty() ? openhd::ground::derive_default_wb_keypair() : wb::read_keypair_from_file(keyfile);
    if (!keypair) throw std::runtime_error("Cannot load or derive OpenHD keypair");
    // Validate and open UDP endpoints before touching USB hardware.
    openhd::UDPForwarder video(destination, video_port), video2(destination, video2_port);
    openhd::UDPForwarder telemetry(destination, telemetry_port);
    std::atomic<uint64_t> video_packets{0}, telemetry_packets{0};
    std::shared_ptr<WBStreamTx> telemetry_tx;
    openhd::UDPReceiver uplink(openhd::ADDRESS_LOCALHOST, uplink_port,
      [&telemetry_tx](const uint8_t* data, std::size_t size) {
        if (telemetry_tx && size > 0 && size <= 1024)
          telemetry_tx->try_enqueue_packet(std::make_shared<std::vector<uint8_t>>(data, data + size));
      });
    WBTxRx::Options options{};
    options.use_gnd_identifier = true;
    options.require_root = false;
    options.secure_keypair = *keypair;
    options.use_devourer = true;
    options.devourer_frequency_mhz = frequency;
    options.devourer_channel_width_mhz = width;
    auto header = std::make_shared<RadiotapHeaderTxHolder>();
    header->update_channel_width(width);
    auto session_header = std::make_shared<RadiotapHeaderTxHolder>();
    session_header->update_channel_width(width < 20 ? width : 20);
    auto radio = std::make_shared<WBTxRx>(cards, options, session_header);
    std::atomic<bool> failed{false};
    radio->m_fatal_error_cb = [&failed](int) { failed.store(true); };
    WBStreamRx::Options video_options{};
    video_options.radio_port = openhd::VIDEO_PRIMARY_RADIO_PORT;
    video_options.fec_rx_queue_depth = cards.size() > 1 ? 2 : 1;
    WBStreamRx primary(radio, video_options);
    primary.set_callback([&](const uint8_t* p, std::size_t n) { video.forwardPacketViaUDP(p, n); ++video_packets; });
    video_options.radio_port = openhd::VIDEO_SECONDARY_RADIO_PORT;
    WBStreamRx secondary(radio, video_options);
    secondary.set_callback([&](const uint8_t* p, std::size_t n) { video2.forwardPacketViaUDP(p, n); ++video_packets; });
    WBStreamRx::Options telemetry_options{};
    telemetry_options.enable_fec = false;
    telemetry_options.radio_port = openhd::TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT;
    WBStreamRx telemetry_rx(radio, telemetry_options);
    telemetry_rx.set_callback([&](const uint8_t* p, std::size_t n) { telemetry.forwardPacketViaUDP(p, n); ++telemetry_packets; });
    WBStreamTx::Options tx_options{};
    tx_options.enable_fec = false;
    tx_options.radio_port = openhd::TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT;
    telemetry_tx = std::make_shared<WBStreamTx>(radio, tx_options, header);
    telemetry_tx->set_encryption(true);
    struct StopReception {
      openhd::UDPReceiver& uplink;
      WBTxRx& radio;
      ~StopReception() { uplink.stopBackground(); radio.stop_receiving(); }
    } stop_reception{uplink, *radio};
    std::signal(SIGINT, stop);
    std::signal(SIGTERM, stop);
    radio->start_receiving();
    uplink.runInBackground();
    std::cout << "Ground active: " << frequency << " MHz / " << width
              << " MHz; video -> " << destination << ':' << video_port
              << "; telemetry -> " << telemetry_port << '\n';
    uint64_t previous_packets = 0;
    auto last_packet = std::chrono::steady_clock::time_point{};
    while (!requested_stop.load() && !stopping && !failed.load()) {
      openhd::ui::DashboardStatus status;
      status.version = "OpenHD - Ground";
#ifdef _WIN32
      status.platform = "Windows / Devourer USB";
#else
      status.platform = "Linux / Devourer USB";
#endif
      status.portable_controls = true; status.camera = "Ground receiver";
      status.wifi = std::to_string(cards.size()) + " USB adapter(s)";
      const auto packets = video_packets.load() + telemetry_packets.load();
      if (packets != previous_packets) { previous_packets = packets; last_packet = std::chrono::steady_clock::now(); }
      status.links[0] = std::chrono::steady_clock::now() - last_packet < std::chrono::seconds(2) ? "Receiving" : "Configured";
      for (int i = 1; i < 5; ++i) status.links[i] = "Unused";
      status.radio_summary = std::to_string(frequency) + " MHz / " + std::to_string(width) + " MHz";
      status.video_summary = "Video: " + std::to_string(video_packets.load()) + " packets";
      status.telemetry_summary = "Telemetry: " + std::to_string(telemetry_packets.load()) + " packets";
      publish(status);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    uplink.stopBackground();
    radio->stop_receiving();
    telemetry_tx.reset();
    return failed.load() ? 2 : 0;
}
int dashboard(Config& config, const std::filesystem::path& path, bool configure, const std::string& load_error) {
    using namespace openhd::ui;
    openhd::ground::configure_log_directory(path.parent_path() / "logs");
    init_ncurses();
    if (!ncurses_active()) throw std::runtime_error("Open in an interactive console for the dashboard, or use --no-ui");
    struct RestoreTerminal { ~RestoreTerminal() { shutdown_ncurses(); } } terminal;
    DashboardStatus status;
    status.version = "OpenHD - Ground";
#ifdef _WIN32
    status.platform = "Windows / Devourer USB";
#else
    status.platform = "Linux / Devourer USB";
#endif
    status.camera = "Ground receiver"; status.portable_controls = true;
    status.links[0] = "Stopped";
    for (int i = 1; i < 5; ++i) status.links[i] = "Unused";
    status.radio_summary = "Press C for settings";
    std::mutex mutex;
    std::atomic<bool> requested_stop{false};
    std::thread worker;
    auto stop_worker = [&] { requested_stop.store(true); if (worker.joinable()) worker.join(); };
    // Worker ownership must end before the mutex, status or console is destroyed.
    struct WorkerGuard { std::function<void()> stop; ~WorkerGuard() { stop(); } } guard{stop_worker};
    auto start_worker = [&] {
      stop_worker(); requested_stop.store(false);
      { std::lock_guard<std::mutex> lock(mutex); status.links[0] = "Starting"; status.radio_summary = "Opening USB radio..."; }
      worker = std::thread([&, snapshot = config] {
        try {
          const int code = run_backend(snapshot, requested_stop, [&](const DashboardStatus& value) {
            std::lock_guard<std::mutex> lock(mutex); status = value;
          });
          std::lock_guard<std::mutex> lock(mutex);
          status.links[0] = code ? "Error" : "Stopped";
          if (code) { status.radio_summary = "Adapter disconnected"; std::cerr << "Devourer adapter failed. Press R to retry or C for settings.\n"; }
        } catch (const std::exception& error) {
          openhd::ui::ncurses_log("Ground", 4, error.what());
          std::lock_guard<std::mutex> lock(mutex); status.links[0] = "Error"; status.radio_summary = "See logs; C settings / R retry";
        }
      });
    };
    if (!load_error.empty()) std::cerr << "Saved settings could not be loaded: " << load_error << '\n';
    if (configure || !std::filesystem::exists(path) || !load_error.empty()) {
      if (openhd::ground::edit_settings(config, path)) start_worker();
    } else if (config.autostart) start_worker();
    while (!stopping) {
      { std::lock_guard<std::mutex> lock(mutex); set_dashboard_status(status); }
      update_ncurses();
      const auto action = take_dashboard_action();
      if (action == DashboardAction::Exit) break;
      if (action == DashboardAction::Restart) start_worker();
      if (action == DashboardAction::Shutdown) {
        stop_worker(); std::lock_guard<std::mutex> lock(mutex); status.links[0] = "Stopped"; status.radio_summary = "Press R to start";
      }
      if (action == DashboardAction::Configure) {
        stop_worker();
        if (openhd::ground::edit_settings(config, path)) start_worker();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return 0;
}

}

int main(int argc, char** argv) {
  try {
#if defined(OPENHD_ENABLE_GUI) && defined(_WIN32)
    if (argc >= 2 && std::string(argv[1]) == "--install-winusb-device")
      return openhd::ui::run_windows_usb_driver_install(argc, argv);
#endif
    auto path = openhd::ground::default_config_path();
    for (int i = 1; i < argc; ++i) {
      const std::string arg = argv[i];
      if (arg == "--help" || arg == "-h") { help(); return 0; }
      if (arg == "--list") { list_usb(); return 0; }
      if (arg == "--config" && i + 1 < argc) path = argv[++i];
    }
#ifdef OPENHD_ENABLE_GUI
    bool gui_headless = false;
    for (int i = 1; i < argc; ++i) if (std::string(argv[i]) == "--no-ui") gui_headless = true;
    if (!gui_headless) return openhd::ui::run_wifi_test_gui();
#endif
    Config config;
    std::string load_error;
    try { config = openhd::ground::load_config(path); }
    catch (const std::exception& error) { load_error = error.what(); }
    auto& keyfile = config.keyfile; auto& destination = config.destination;
    auto& frequency = config.frequency; auto& width = config.width;
    auto& video_port = config.video_port; auto& video2_port = config.video2_port;
    auto& telemetry_port = config.telemetry_port; auto& uplink_port = config.uplink_port;
    bool headless = false, configure = false, devices_overridden = false;
    std::set<std::string> selected;
    for (int i = 1; i < argc; ++i) {
      const std::string arg = argv[i];
      if (arg == "--help" || arg == "-h") { help(); return 0; }
      if (arg == "--list") { list_usb(); return 0; }
      if (arg == "--no-ui") { headless = true; continue; }
      if (arg == "--configure") { configure = true; continue; }
      if (i + 1 == argc) throw std::invalid_argument("Missing value for " + arg);
      const std::string value = argv[++i];
      if (arg == "--config") { continue; }
      if (arg == "--usb") {
        const auto separator = value.find(':');
        if (separator == std::string::npos || separator != 4 || value.size() != 9 ||
            value.find_first_not_of("0123456789abcdefABCDEF", 0) != 4 ||
            value.substr(5).find_first_not_of("0123456789abcdefABCDEF") != std::string::npos ||
            std::stoul(value.substr(0, 4), nullptr, 16) == 0 ||
            std::stoul(value.substr(5), nullptr, 16) == 0)
          throw std::invalid_argument("USB IDs must be four hexadecimal digits each: VID:PID");
        auto name = "devourer-id-" + value.substr(0, 4) + '-' + value.substr(5);
        for (auto& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (!selected.insert(name).second) throw std::invalid_argument("Duplicate USB device");
        if (!devices_overridden) { config.devices.clear(); devices_overridden = true; }
        config.devices.push_back(name);
      } else if (arg == "--device") {
        const std::string prefix = "devourer-usb-";
        const auto separator = value.find('-', prefix.size());
        if (value.rfind(prefix, 0) != 0 || separator == std::string::npos)
          throw std::invalid_argument("Use a USB address from --list");
        number(value.substr(prefix.size(), separator - prefix.size()), 1, 255);
        number(value.substr(separator + 1), 1, 255);
        if (!selected.insert(value).second) throw std::invalid_argument("Duplicate USB device");
        if (!devices_overridden) { config.devices.clear(); devices_overridden = true; }
        config.devices.push_back(value);
      } else if (arg == "--keypair") keyfile = value;
      else if (arg == "--destination") destination = value;
      else if (arg == "--frequency") frequency = number(value, 2300, 6000);
      else if (arg == "--width") width = number(value, 5, 80);
      else if (arg == "--video-port") video_port = number(value, 1, 65535);
      else if (arg == "--video2-port") video2_port = number(value, 1, 65535);
      else if (arg == "--telemetry-port") telemetry_port = number(value, 1, 65535);
      else if (arg == "--uplink-port") uplink_port = number(value, 1, 65535);
      else throw std::invalid_argument("Unknown option: " + arg);
    }

    openhd::ground::validate_config(config);
    std::signal(SIGINT, stop); std::signal(SIGTERM, stop);
    if (headless) {
      if (!load_error.empty()) throw std::runtime_error("Saved settings: " + load_error);
      std::atomic<bool> requested_stop{false};
      return run_backend(config, requested_stop, [](const auto&) {});
    }
    return dashboard(config, path, configure, load_error);
  } catch (const std::exception& error) {
    std::cerr << "OpenHD ground: " << error.what() << '\n'; return 1;
  }
}
