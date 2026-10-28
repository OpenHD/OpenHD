#pragma once
#include <cstdint>
#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace openhd::wifi_test {
using Json = nlohmann::json;
struct Settings {
  std::string device;
  int frequency = 5745, width = 20, mcs = 2, power = 20, stbc = 1;
  bool ldpc = true, short_gi = false;
  bool no_ack = true;
  int fec = 20, block = 0, bitrate_kbps = 4000, packet_bytes = 1024;
  uint32_t seed = 42;
  bool video = false;
  bool operator==(const Settings& other) const;
};
Json settings_json(const Settings& s);
Settings settings_from_json(const Json& j);
void validate(const Settings& s);
Json channels();
Json control_options();
Json devices();
// Immutable source payloads, shared by the production worker and contract
// tests.
std::vector<std::vector<uint8_t>> sample_video_frames();
std::string sample_video_sha256();
class PayloadSource {
 public:
  explicit PayloadSource(const Settings& settings);
  ~PayloadSource();
  std::vector<std::shared_ptr<std::vector<uint8_t>>> next();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

class Controller {
 public:
  Controller();
  ~Controller();
  Controller(const Controller&) = delete;
  Controller& operator=(const Controller&) = delete;
  // Nonblocking commands. One worker owns the radio and all setting changes.
  void start(Settings settings, std::filesystem::path session_directory);
  void apply(Settings settings);
  void stop();
  Json snapshot() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
}  // namespace openhd::wifi_test
