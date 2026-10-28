#include "wifi_test.h"

#include <libusb-1.0/libusb.h>
#include <sodium.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <random>
#include <sstream>
#include <thread>

#include "WBStreamTx.h"
#include "WBTxRx.h"
#include "default_keys.h"
#include "devourer_usb_device_probe.h"
#include "openhd_global_constants.hpp"
#include "rtp-payload.h"
#include "wb_radio_controls.h"
#include "wifi_channel.h"
#include "wifi_test_video.h"

namespace openhd::wifi_test {
namespace {
using Clock = std::chrono::steady_clock;
int64_t utc_us() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}
std::string hash_bytes(const unsigned char* bytes, size_t count) {
  unsigned char digest[crypto_hash_sha256_BYTES];
  crypto_hash_sha256(digest, bytes, count);
  std::ostringstream s;
  for (auto c : digest)
    s << std::hex << std::setfill('0') << std::setw(2) << int(c);
  return s.str();
}
}  // namespace
Json settings_json(const Settings& s) {
  return {{"device", s.device},
          {"frequency_mhz", s.frequency},
          {"width_mhz", s.width},
          {"mcs", s.mcs},
          {"power_level_percent", s.power},
          {"stbc", s.stbc},
          {"ldpc", s.ldpc},
          {"short_gi", s.short_gi},
          {"no_ack", s.no_ack},
          {"fec_percent", s.fec},
          {"fec_block", s.block},
          {"bitrate_kbps", s.bitrate_kbps},
          {"packet_bytes", s.packet_bytes},
          {"seed", s.seed},
          {"payload", s.video ? "sample_video" : "garbage"}};
}
bool Settings::operator==(const Settings& other) const {
  return settings_json(*this) == settings_json(other);
}
Settings settings_from_json(const Json& j) {
  Settings s;
  s.device = j.at("device");
  s.frequency = j.at("frequency_mhz");
  s.width = j.at("width_mhz");
  s.mcs = j.at("mcs");
  s.power = j.at("power_level_percent");
  s.stbc = j.at("stbc");
  s.ldpc = j.at("ldpc");
  s.short_gi = j.at("short_gi");
  s.no_ack = j.at("no_ack");
  s.fec = j.at("fec_percent");
  s.block = j.at("fec_block");
  s.bitrate_kbps = j.at("bitrate_kbps");
  s.packet_bytes = j.at("packet_bytes");
  s.seed = j.at("seed");
  s.video = j.at("payload") == "sample_video";
  if (j.at("payload") != "sample_video" && j.at("payload") != "garbage")
    throw std::invalid_argument("Unknown payload");
  validate(s);
  return s;
}
void validate(const Settings& s) {
  if (s.device.empty()) throw std::invalid_argument("Select an adapter");
  if (!channel_from_frequency(s.frequency))
    throw std::invalid_argument("Frequency is not in OpenHD's channel table");
  if (!is_valid_channel_width(s.width))
    throw std::invalid_argument("Channel width is not enabled by OpenHD");
  if (!is_valid_mcs_index(s.mcs))
    throw std::invalid_argument("MCS is outside OpenHD's range");
  if (!is_valid_power_level(s.power))
    throw std::invalid_argument("Invalid OpenHD power level");
  if (!is_valid_stbc(s.stbc)) throw std::invalid_argument("STBC must be 0..3");
  if ((s.fec != 0 && !is_valid_fec_percentage_value(s.fec)) ||
      !is_valid_fec_block_length(s.block))
    throw std::invalid_argument("Invalid OpenHD FEC settings");
  if (s.bitrate_kbps < 100 || s.bitrate_kbps > 100000)
    throw std::invalid_argument("Payload rate must be 100..100000 kbit/s");
  if (s.packet_bytes < 64 || s.packet_bytes > FEC_PACKET_MAX_PAYLOAD_SIZE)
    throw std::invalid_argument("Invalid packet size");
  if (s.video && s.packet_bytes < 564)
    throw std::invalid_argument(
        "OpenHD's RTP packetizer requires an MTU of at least 564 bytes");
}
Json channels() {
  Json result = Json::array();
  for (const auto& c : get_all_channels_2G_5G())
    result.push_back({{"mhz", c.frequency},
                      {"channel", c.channel},
                      {"standard", c.is_standard}});
  return result;
}
Json control_options() {
  Json j = {{"width", Json::array()}, {"mcs", Json::array()},
            {"power", Json::array()}, {"stbc", Json::array()},
            {"fec", Json::array()},   {"block", Json::array()}};
  for (unsigned w = 1; w <= 160; ++w)
    if (is_valid_channel_width(w)) j["width"].push_back(w);
  for (unsigned m = 0; m <= radiotap::tx::MCS_MAX; ++m)
    if (is_valid_mcs_index(m)) j["mcs"].push_back(m);
  for (int p = WB_TX_POWER_LEVEL_MIN; p <= WB_TX_POWER_LEVEL_MAX_OVERDRIVE; ++p)
    if (is_valid_power_level(p)) j["power"].push_back(p);
  for (int n = 0; n <= 255; ++n) {
    if (is_valid_stbc(n)) j["stbc"].push_back(n);
    if (is_valid_fec_block_length(n)) j["block"].push_back(n);
  }
  j["fec"].push_back(0);
  for (int n = 1; n <= 1000; ++n)
    if (is_valid_fec_percentage_value(n)) j["fec"].push_back(n);
  return j;
}
std::optional<Json> describe_usb_radio(uint16_t vid, uint16_t pid,
                                       uint8_t chip_id) {
  const auto probe = devourer::classify_usb_device(vid, pid, chip_id);
  if (!probe || probe->chip == devourer::UsbChip::Unknown ||
      !probe->supported_by_build)
    return std::nullopt;
  std::string model;
  std::string chipset = probe->marketing_names;
  // Verified USB-AC56 identity; shared/generic Realtek IDs intentionally do
  // not get a retail model name because multiple vendors/chips reuse them.
  if (vid == 0x0b05 && pid == 0x17d2 &&
      probe->chip == devourer::UsbChip::Rtl8812A) {
    model = "ASUS USB-AC56";
    chipset = "RTL8812AU";
  }
  std::ostringstream label;
  if (!model.empty()) label << model << " | ";
  label << chipset;
  return Json{{"model", model},
              {"chipset", chipset},
              {"chip", probe->chip_name},
              {"chip_id", probe->chip_id},
              {"generation", devourer::generation_name(probe->generation)},
              {"supported_by_build", true},
              {"label", label.str()}};
}
Json devices() {
  libusb_context* context = nullptr;
  if (libusb_init(&context) != 0)
    throw std::runtime_error("USB discovery failed");
  struct ContextGuard {
    libusb_context* p;
    ~ContextGuard() { libusb_exit(p); }
  } cg{context};
  libusb_device** list = nullptr;
  const auto count = libusb_get_device_list(context, &list);
  if (count < 0) throw std::runtime_error("USB enumeration failed");
  struct ListGuard {
    libusb_device** p;
    ~ListGuard() { libusb_free_device_list(p, 1); }
  } lg{list};
  Json result = Json::array();
  for (int64_t i = 0; i < count; ++i) {
    libusb_device_descriptor d{};
    if (libusb_get_device_descriptor(list[i], &d) != 0) continue;
    const auto probe = devourer::probe_usb_device(list[i]);
    if (!probe) continue;
    auto identity = describe_usb_radio(d.idVendor, d.idProduct, probe->chip_id);
    if (!identity) continue;
    const int bus = libusb_get_bus_number(list[i]),
              address = libusb_get_device_address(list[i]);
    std::ostringstream label;
    label << identity->at("label").get<std::string>() << "  " << std::hex
          << std::setfill('0') << std::setw(4) << d.idVendor << ':'
          << std::setw(4) << d.idProduct << std::dec << "  USB " << bus << ':'
          << address;
    identity->update({{"selector", "devourer-usb-" + std::to_string(bus) + "-" +
                                       std::to_string(address)},
                      {"label", label.str()},
                      {"vid", d.idVendor},
                      {"pid", d.idProduct},
                      {"bus", bus},
                      {"address", address}});
    result.push_back(std::move(*identity));
  }
  return result;
}
std::string sample_video_sha256() {
  return hash_bytes(wifi_test_video, sizeof(wifi_test_video));
}
std::vector<std::vector<uint8_t>> sample_video_frames() {
  std::vector<size_t> starts;
  for (size_t i = 0; i + 5 < sizeof(wifi_test_video); ++i) {
    if (wifi_test_video[i] == 0 && wifi_test_video[i + 1] == 0 &&
        wifi_test_video[i + 2] == 0 && wifi_test_video[i + 3] == 1 &&
        (wifi_test_video[i + 4] & 31) == 9)
      starts.push_back(i);
  }
  if (starts.empty() || starts.front() != 0)
    throw std::runtime_error("Sample video is missing access-unit delimiters");
  starts.push_back(sizeof(wifi_test_video));
  std::vector<std::vector<uint8_t>> frames;
  for (size_t i = 1; i < starts.size(); ++i)
    frames.emplace_back(wifi_test_video + starts[i - 1],
                        wifi_test_video + starts[i]);
  return frames;
}

struct PayloadSource::Impl {
  Settings settings;
  std::mt19937 random;
  std::vector<std::vector<uint8_t>> video = sample_video_frames();
  size_t index = 0;
  uint32_t timestamp = 0;
  std::vector<std::shared_ptr<std::vector<uint8_t>>> fragments;
  std::vector<uint8_t> buffer = std::vector<uint8_t>(2048);
  rtp_payload_t handler{};
  void* rtp = nullptr;
  explicit Impl(const Settings& s) : settings(s), random(s.seed) {
    handler.alloc = [](void* p, int bytes) -> void* {
      auto* self = static_cast<Impl*>(p);
      return bytes >= 0 && size_t(bytes) <= self->buffer.size()
                 ? self->buffer.data()
                 : nullptr;
    };
    handler.free = [](void*, void*) {};
    handler.packet = [](void* p, const void* data, int bytes, uint32_t,
                        int) -> int {
      auto* self = static_cast<Impl*>(p);
      const auto* b = static_cast<const uint8_t*>(data);
      self->fragments.push_back(
          std::make_shared<std::vector<uint8_t>>(b, b + bytes));
      return 0;
    };
    rtp_packet_setsize(s.packet_bytes);
    rtp = rtp_payload_encode_create(96, "H264", 0, 0x4f484454, &handler, this);
    if (!rtp)
      throw std::runtime_error("OpenHD RTP packetizer could not initialize");
  }
  ~Impl() {
    if (rtp) rtp_payload_encode_destroy(rtp);
  }
  std::vector<std::shared_ptr<std::vector<uint8_t>>> next() {
    if (!settings.video) {
      auto data = std::make_shared<std::vector<uint8_t>>(settings.packet_bytes);
      for (auto& b : *data) b = uint8_t(random());
      return {data};
    }
    const auto& frame = video[index++ % video.size()];
    fragments.clear();
    if (rtp_payload_encode_input(rtp, frame.data(), int(frame.size()),
                                 timestamp) != 0)
      throw std::runtime_error("Sample RTP packetization failed");
    timestamp += 3000;
    return std::move(fragments);
  }
};
PayloadSource::PayloadSource(const Settings& s)
    : impl_(std::make_unique<Impl>(s)) {}
PayloadSource::~PayloadSource() = default;
std::vector<std::shared_ptr<std::vector<uint8_t>>> PayloadSource::next() {
  return impl_->next();
}

struct Controller::Impl {
  mutable std::mutex mutex;
  std::condition_variable wake;
  enum class Action { Start, Apply, Stop, Quit };
  struct Command {
    Action action;
    Settings settings;
    std::filesystem::path directory;
  };
  std::deque<Command> commands;
  Json state = {{"state", "idle"},
                {"message", "Select an adapter and press Start"},
                {"events", Json::array()}};
  std::thread worker;
  Clock::time_point began = Clock::now();
  std::ofstream journal, csv;
  uint64_t segment = 0, generated = 0, accepted = 0, rejected = 0, late = 0;
  bool running = false;
  Settings current;
  std::shared_ptr<WBTxRx> radio;
  std::shared_ptr<RadiotapHeaderTxHolder> header;
  std::shared_ptr<RadiotapHeaderTxHolder> session_header;
  std::unique_ptr<WBStreamTx> tx;
  std::atomic<bool> fatal{false};
  std::atomic<bool> cancel_requested{false};
  std::unique_ptr<PayloadSource> source;
  Clock::time_point next_packet, next_sample;
  Json applied = Json::object();
  Impl() {
    worker = std::thread([this] { loop(); });
  }
  ~Impl() {
    enqueue({Action::Quit, {}, {}});
    worker.join();
  }
  void enqueue(Command c) {
    std::lock_guard<std::mutex> lock(mutex);
    if (c.action == Action::Stop || c.action == Action::Quit)
      cancel_requested = true;
    if (c.action == Action::Start) cancel_requested = false;
    if (c.action == Action::Start) state["state"] = "starting";
    if (c.action == Action::Apply) state["state"] = "applying";
    if (c.action == Action::Stop) state["state"] = "stopping";
    commands.push_back(std::move(c));
    wake.notify_one();
  }
  int64_t elapsed() const {
    return std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() -
                                                                 began)
        .count();
  }
  void message(std::string phase, std::string text) {
    std::lock_guard<std::mutex> lock(mutex);
    state["state"] = phase;
    state["message"] = text;
  }
  void record(std::string type, Json extra = Json::object()) {
    Json row = {{"schema", 1},        {"type", type},
                {"utc_us", utc_us()}, {"elapsed_us", elapsed()},
                {"segment", segment}, {"settings", settings_json(current)}};
    row.update(extra);
    if (journal.is_open()) {
      journal << row.dump() << '\n';
      journal.flush();
      if (!journal) throw std::runtime_error("Session journal write failed");
    }
    if (type != "sample") {
      std::lock_guard<std::mutex> lock(mutex);
      auto& events = state["events"];
      events.push_back(row);
      if (events.size() > 2000) events.erase(events.begin());
    }
  }
  void open_journal(const std::filesystem::path& directory) {
    if (std::filesystem::exists(directory))
      throw std::runtime_error("Session directory already exists");
    std::filesystem::create_directories(directory);
    journal.clear();
    csv.clear();
    journal.open(directory / "session.jsonl", std::ios::out | std::ios::binary);
    csv.open(directory / "samples.csv", std::ios::out | std::ios::binary);
    if (!journal || !csv)
      throw std::runtime_error("Cannot create session files");
    csv << "utc_us,elapsed_us,segment,frequency_mhz,width_mhz,mcs,power_level_"
           "percent,applied_power_offset_qdb,stbc,ldpc,short_gi,fec_percent,"
           "fec_block,payload,source_kbps,generated,accepted,rejected,pacing_"
           "late,tx_packets,tx_bytes,tx_drop,tx_error_hint,thermal_raw\n";
    std::lock_guard<std::mutex> lock(mutex);
    state["events"] = Json::array();
    state["directory"] = directory.string();
    state.erase("sample");
  }
  void check_capabilities(const Settings& s) {
    const auto caps = radio->get_devourer_adapter_caps(0);
    if (!caps || !caps->supported || !caps->tx.supported)
      throw std::runtime_error("Adapter capabilities unavailable");
    const auto band = s.frequency < 3000 ? caps->tune_2g4 : caps->tune_5g;
    if (!band.valid || s.frequency < band.min_mhz || s.frequency > band.max_mhz)
      throw std::runtime_error(
          "Adapter cannot tune the selected OpenHD channel");
    const auto width_bit = s.width == 10   ? devourer::kBw10
                           : s.width == 20 ? devourer::kBw20
                                           : devourer::kBw40;
    if (!(caps->bw_mask & width_bit))
      throw std::runtime_error("Adapter does not support the selected width");
    if (!caps->txpwr.supported)
      throw std::runtime_error("Adapter does not expose TX power controls");
    if (s.mcs / 8 >= caps->tx.n_ss)
      throw std::runtime_error(
          "Selected MCS requires more spatial streams than this adapter has");
    if (s.stbc && !caps->tx.stbc_ok)
      throw std::runtime_error("Adapter does not support STBC");
    if (s.ldpc && !caps->tx.ldpc_ok)
      throw std::runtime_error("Adapter does not support LDPC");
    if (s.short_gi && !caps->tx.sgi_ok)
      throw std::runtime_error("Adapter does not support short GI");
    std::lock_guard<std::mutex> lock(mutex);
    state["capabilities"] = {
        {"chip", caps->chip_name},      {"streams", caps->tx.n_ss},
        {"width_mask", caps->bw_mask},  {"stbc", caps->tx.stbc_ok},
        {"ldpc", caps->tx.ldpc_ok},     {"short_gi", caps->tx.sgi_ok},
        {"tx_chains", caps->tx_chains}, {"rx_chains", caps->rx_chains}};
  }
  void configure(const Settings& s) {
    validate(s);
    check_capabilities(s);
    if (!radio->set_devourer_channel(s.frequency, s.width))
      throw std::runtime_error("Devourer rejected channel/width");
    const int offset = radio->set_devourer_tx_power_level(
        0, s.power, devourer_power_offset_qdb(s.power));
    const auto power = radio->get_devourer_tx_power_diagnostics(0);
    if (power.find("txagc_result=write_failed") != std::string::npos ||
        power.find("txagc_result=readback_mismatch") != std::string::npos)
      throw std::runtime_error("TX power write/readback failed: " + power);
    RadiotapHeaderTx::UserSelectableParams p;
    p.bandwidth = s.width;
    p.mcs_index = s.mcs;
    p.stbc = s.stbc;
    p.ldpc = s.ldpc;
    p.short_gi = s.short_gi;
    p.set_flag_tx_no_ack = s.no_ack;
    header = std::make_shared<RadiotapHeaderTxHolder>();
    header->thread_safe_set(p);
    auto session_params = p;
    session_params.bandwidth = std::min(s.width, 20);
    session_header->thread_safe_set(session_params);
    WBStreamTx::Options options;
    options.radio_port = openhd::VIDEO_PRIMARY_RADIO_PORT;
    options.enable_fec = s.fec > 0;
    options.default_packet_type = WB_PACKET_TYPE_VIDEO;
    tx = std::make_unique<WBStreamTx>(radio, options, header);
    tx->set_encryption(false);
    source = std::make_unique<PayloadSource>(s);
    applied = {
        {"power_offset_qdb", offset},
        {"power_diagnostics", power},
        {"effective_fec_block_limit",
         s.fec > 0 ? std::min(s.block == 0 ? 20 : s.block,
                              std::max(1, int(MAX_N_S_FRAGMENTS_PER_BLOCK) *
                                              100 / s.fec))
                   : 0},
        {"radiotap_settings", settings_json(s)},
        {"session_key_width_mhz", session_params.bandwidth},
        {"evidence", "backend_accepted; physical RF requires lab measurement"},
        {"rf_power_dbm", nullptr},
        {"receiver_loss_percent", nullptr}};
    {
      std::lock_guard<std::mutex> lock(mutex);
      applied["adapter_capabilities"] = state.at("capabilities");
    }
    current = s;
    generated = accepted = rejected = late = 0;
    next_packet = Clock::now();
    next_sample = next_packet;
    {
      std::lock_guard<std::mutex> lock(mutex);
      state["applied"] = applied;
      state["settings"] = settings_json(current);
    }
  }
  Json sample() {
    const auto t = radio->get_tx_stats();
    const auto thermal = radio->get_devourer_thermal_status(0);
    Json j = {{"generated", generated},
              {"accepted", accepted},
              {"rejected", rejected},
              {"pacing_late", late},
              {"tx_packets", t.n_injected_packets},
              {"tx_bytes", t.n_injected_bytes_excluding_overhead},
              {"tx_drop", t.count_tx_dropped_packets},
              {"tx_error_hint", t.count_tx_injections_error_hint},
              {"tx_bps", t.curr_bits_per_second_excluding_overhead},
              {"applied", applied},
              {"receiver_loss_percent", nullptr},
              {"rf_power_dbm", nullptr}};
    j["thermal_raw"] =
        thermal && thermal->valid ? Json(thermal->raw) : Json(nullptr);
    j["power_diagnostics"] = radio->get_devourer_tx_power_diagnostics(0);
    record("sample", {{"measurement", j}});
    csv << utc_us() << ',' << elapsed() << ',' << segment << ','
        << current.frequency << ',' << current.width << ',' << current.mcs
        << ',' << current.power << ',' << applied.value("power_offset_qdb", 0)
        << ',' << current.stbc << ',' << current.ldpc << ',' << current.short_gi
        << ',' << current.fec << ',' << current.block << ','
        << (current.video ? "sample_video" : "garbage") << ','
        << current.bitrate_kbps << ',' << generated << ',' << accepted << ','
        << rejected << ',' << late << ',' << t.n_injected_packets << ','
        << t.n_injected_bytes_excluding_overhead << ','
        << t.count_tx_dropped_packets << ',' << t.count_tx_injections_error_hint
        << ','
        << (thermal && thermal->valid ? std::to_string(thermal->raw) : "")
        << '\n';
    csv.flush();
    if (!csv) throw std::runtime_error("CSV write failed");
    {
      std::lock_guard<std::mutex> lock(mutex);
      state["sample"] = j;
      state["elapsed_us"] = elapsed();
    }
    return j;
  }
  void end_segment() {
    if (!tx) return;
    const auto last = tx->get_latest_stats();
    const int pending = tx->get_tx_queue_available_size_approximate();
    // Destroy/join the old queue before changing headers, channel or power.
    tx.reset();
    record("segment_end", {{"measurement", sample()},
                           {"stream_queue_drops", last.n_dropped_packets},
                           {"stream_frame_drops", last.n_dropped_frames},
                           {"pending_queue_items_at_stop", pending}});
  }
  void close_run(const std::string& reason) {
    running = false;
    end_segment();
    source.reset();
    radio.reset();
    header.reset();
    if (journal.is_open()) record("stop", {{"reason", reason}});
    journal.close();
    csv.close();
  }
  void produce() {
    auto fragments = source->next();
    if (!current.video && current.fec > 0) {
      const int k = applied.at("effective_fec_block_limit");
      for (int i = 1; i < k; ++i) {
        auto more = source->next();
        fragments.insert(fragments.end(), more.begin(), more.end());
      }
    }
    size_t bytes = 0;
    for (const auto& f : fragments) bytes += f->size();
    generated += fragments.size();
    if (current.fec > 0) {
      const int max_secondary_limited =
          std::max(1, int(MAX_N_S_FRAGMENTS_PER_BLOCK) * 100 / current.fec);
      const int k = std::min(current.block == 0 ? 20 : current.block,
                             max_secondary_limited);
      if (tx->try_enqueue_block(fragments, k, current.fec))
        accepted += fragments.size();
      else
        rejected += fragments.size();
    } else {
      for (const auto& f : fragments)
        if (tx->try_enqueue_packet(f))
          ++accepted;
        else
          ++rejected;
    }
    const auto interval = std::chrono::microseconds(
        std::max<int64_t>(1, int64_t(bytes) * 8000 / current.bitrate_kbps));
    next_packet += interval;
    // Never send an unbounded catch-up burst after the host stalls.
    if (Clock::now() > next_packet + interval) {
      ++late;
      next_packet = Clock::now() + interval;
    }
  }
  void handle(Command c) {
    if (c.action == Action::Stop) {
      close_run("user");
      message("idle", "Stopped; session saved");
      return;
    }
    if (c.action == Action::Start) {
      if (running) throw std::runtime_error("A test is already running");
      validate(c.settings);
      began = Clock::now();
      segment = 0;
      current = c.settings;
      open_journal(c.directory);
      message("starting", "Opening selected adapter...");
      record("start_requested",
             {{"source_revision", OPENHD_WIFI_TEST_REVISION},
              {"sample_video_sha256", sample_video_sha256()},
              {"generator", "mt19937 low byte per output"},
              {"wire_format",
               "OpenHD air identifier; primary video stream; authenticated; no "
               "video encryption"},
              {"lab_mode", "broadcast TX only; no receiver metrics"}});
      auto key = openhd::ground::derive_default_wb_keypair();
      if (!key) throw std::runtime_error("OpenHD key derivation failed");
      WBTxRx::Options o;
      o.require_root = false;
      o.use_gnd_identifier = false;
      o.use_devourer = true;
      o.secure_keypair = *key;
      o.devourer_frequency_mhz = c.settings.frequency;
      o.devourer_channel_width_mhz = c.settings.width;
      header = std::make_shared<RadiotapHeaderTxHolder>();
      header->update_channel_width(c.settings.width);
      session_header = header;
      fatal = false;
      radio = std::make_shared<WBTxRx>(
          std::vector<wifibroadcast::WifiCard>{{c.settings.device, 1}}, o,
          header);
      radio->m_fatal_error_cb = [this](int) { fatal = true; };
    } else {
      if (!running) throw std::runtime_error("No test is running");
      record("apply_requested", {{"requested", settings_json(c.settings)}});
      if (c.settings.device != current.device)
        throw std::runtime_error("Stop before changing the adapter");
      validate(c.settings);
      check_capabilities(c.settings);
      if (c.settings == current) {
        message("running", "Settings unchanged; continuing broadcast");
        return;
      }
      end_segment();
    }
    ++segment;
    const auto before = Clock::now();
    configure(c.settings);
    running = true;
    record("applied", {{"applied", applied},
                       {"apply_duration_us",
                        std::chrono::duration_cast<std::chrono::microseconds>(
                            Clock::now() - before)
                            .count()}});
    message("running", "Broadcasting selected settings");
  }
  void loop() {
    for (;;) {
      try {
        Command command;
        bool has_command = false;
        {
          std::unique_lock<std::mutex> lock(mutex);
          if (commands.empty()) {
            if (running)
              wake.wait_until(lock, std::min(next_packet, next_sample),
                              [&] { return !commands.empty(); });
            else
              wake.wait(lock, [&] { return !commands.empty(); });
          }
          if (!commands.empty()) {
            command = std::move(commands.front());
            commands.pop_front();
            has_command = true;
          }
        }
        if (has_command) {
          if (command.action == Action::Quit) {
            try {
              close_run("window_closed");
            } catch (...) {
              tx.reset();
              radio.reset();
            }
            return;
          }
          handle(std::move(command));
        }
        if (running && !cancel_requested) {
          if (fatal)
            throw std::runtime_error(
                "Radio disconnected or reported a fatal TX error");
          if (Clock::now() >= next_packet) produce();
          if (Clock::now() >= next_sample) {
            sample();
            next_sample = Clock::now() + std::chrono::milliseconds(250);
          }
        }
      } catch (const std::exception& e) {
        const std::string error = e.what();
        try {
          record("error", {{"message", error}});
          close_run("error");
        } catch (...) {
          running = false;
          tx.reset();
          radio.reset();
          journal.close();
          csv.close();
        }
        message("error", error);
      }
    }
  }
};
Controller::Controller() : impl_(std::make_unique<Impl>()) {}
Controller::~Controller() = default;
void Controller::start(Settings settings, std::filesystem::path directory) {
  impl_->enqueue(
      {Impl::Action::Start, std::move(settings), std::move(directory)});
}
void Controller::apply(Settings settings) {
  impl_->enqueue({Impl::Action::Apply, std::move(settings), {}});
}
void Controller::stop() { impl_->enqueue({Impl::Action::Stop, {}, {}}); }
Json Controller::snapshot() const {
  std::lock_guard<std::mutex> lock(impl_->mutex);
  return impl_->state;
}
}  // namespace openhd::wifi_test
