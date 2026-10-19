#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace openhd::ground {
struct Config {
  std::vector<std::string> devices;  // Empty means discover supported USB adapters.
  std::string keyfile;
  std::string destination = "127.0.0.1";
  int frequency = 5180, width = 20;
  int video_port = 5600, video2_port = 5601, telemetry_port = 14550, uplink_port = 14551;
  bool autostart = true;
};
struct UsbAdapter { std::string selector, label; bool candidate = false; };
std::filesystem::path default_config_path();
Config load_config(const std::filesystem::path& path);
void save_config(const Config& config, const std::filesystem::path& path);
void validate_config(const Config& config);
std::vector<UsbAdapter> enumerate_usb();
std::vector<std::string> discover_radios();
bool edit_settings(Config& config, const std::filesystem::path& path);
void configure_log_directory(const std::filesystem::path& path);
void record_console_line(const std::string& line);
}
