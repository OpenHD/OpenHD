#include "openhd_socket_platform.h"
#include "ground_config.h"
#include "devourer_usb_device_probe.h"
#include <libusb-1.0/libusb.h>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>

namespace openhd::ground {
std::filesystem::path default_config_path() {
  if (const auto* path = std::getenv("OPENHD_GROUND_CONFIG")) return path;
#ifdef _WIN32
  if (const auto* appdata = std::getenv("APPDATA")) return std::filesystem::path(appdata) / "OpenHD" / "ground.json";
#else
  if (const auto* config = std::getenv("XDG_CONFIG_HOME")) return std::filesystem::path(config) / "openhd" / "ground.json";
  if (const auto* home = std::getenv("HOME")) return std::filesystem::path(home) / ".config" / "openhd" / "ground.json";
#endif
  return std::filesystem::current_path() / "openhd-ground.json";
}
void validate_config(const Config& c) {
  in_addr address{};
  net::initialize();
  if (inet_pton(AF_INET, c.destination.c_str(), &address) != 1) throw std::invalid_argument("Invalid IPv4 address");
  if (c.width != 5 && c.width != 10 && c.width != 20 && c.width != 40 && c.width != 80)
    throw std::invalid_argument("Unsupported channel width");
  if (!((c.frequency >= 2412 && c.frequency <= 2472 && (c.frequency - 2407) % 5 == 0) ||
        c.frequency == 2484 || (c.frequency >= 5000 && c.frequency <= 6000 && c.frequency % 5 == 0)))
    throw std::invalid_argument("Unsupported frequency");
  for (const auto port : {c.video_port, c.video2_port, c.telemetry_port, c.uplink_port})
    if (port < 1 || port > 65535) throw std::invalid_argument("UDP port must be 1..65535");
  if ((c.destination == "127.0.0.1" || c.destination == "0.0.0.0") &&
      (c.uplink_port == c.video_port || c.uplink_port == c.video2_port || c.uplink_port == c.telemetry_port))
    throw std::invalid_argument("Uplink input must use a different port from outputs");
  if (!c.keyfile.empty()) {
    std::ifstream key(c.keyfile, std::ios::binary | std::ios::ate);
    if (!key || key.tellg() != 128) throw std::invalid_argument("Keypair must contain exactly 128 bytes");
  }
}
Config load_config(const std::filesystem::path& path) {
  if (!std::filesystem::exists(path)) return {};
  std::ifstream file(path);
  if (!file) throw std::runtime_error("Cannot read saved ground settings");
  const auto json = nlohmann::json::parse(file);
  if (!json.is_object() || json.value("version", 1) != 1) throw std::runtime_error("Unsupported ground settings format");
  Config c;
  c.devices = json.value("devices", c.devices);
  c.keyfile = json.value("keyfile", c.keyfile);
  c.destination = json.value("destination", c.destination);
  c.frequency = json.value("frequency", c.frequency); c.width = json.value("width", c.width);
  c.video_port = json.value("video_port", c.video_port); c.video2_port = json.value("video2_port", c.video2_port);
  c.telemetry_port = json.value("telemetry_port", c.telemetry_port); c.uplink_port = json.value("uplink_port", c.uplink_port);
  c.autostart = json.value("autostart", c.autostart);
  validate_config(c);
  return c;
}
void save_config(const Config& c, const std::filesystem::path& path) {
  validate_config(c);
  if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path());
  const nlohmann::json json{{"version", 1}, {"devices", c.devices}, {"keyfile", c.keyfile},
      {"destination", c.destination}, {"frequency", c.frequency}, {"width", c.width},
      {"video_port", c.video_port}, {"video2_port", c.video2_port},
      {"telemetry_port", c.telemetry_port}, {"uplink_port", c.uplink_port}, {"autostart", c.autostart}};
  auto temporary = path; temporary += ".tmp";
  { std::ofstream file(temporary, std::ios::trunc); file << json.dump(2) << '\n'; file.flush();
    if (!file) throw std::runtime_error("Cannot write ground settings"); }
#ifdef _WIN32
  if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    throw std::runtime_error("Cannot replace ground settings");
#else
  std::filesystem::rename(temporary, path);
#endif
}
std::vector<UsbAdapter> enumerate_usb() {
  libusb_context* context = nullptr;
  if (libusb_init(&context) != 0) throw std::runtime_error("USB enumeration failed");
  libusb_device** devices = nullptr;
  const auto count = libusb_get_device_list(context, &devices);
  if (count < 0) { libusb_exit(context); throw std::runtime_error("USB enumeration failed"); }
  std::vector<UsbAdapter> result;
  for (decltype(count + 0) i = 0; i < count; ++i) {
    libusb_device_descriptor id{};
    if (libusb_get_device_descriptor(devices[i], &id) != 0) continue;
    std::ostringstream ids;
    ids << std::hex << std::setfill('0') << std::setw(4) << id.idVendor << '-' << std::setw(4) << id.idProduct;
    const auto classified = devourer::classify_usb_device(id.idVendor, id.idProduct, 0);
    std::ostringstream details;
    details << "USB " << int(libusb_get_bus_number(devices[i])) << ':'
            << int(libusb_get_device_address(devices[i])) << "  VID:PID " << ids.str()
            << "  USB speed code=" << libusb_get_device_speed(devices[i])
            << "  bcdDevice=0x" << std::hex << id.bcdDevice << std::dec;
    if (classified) {
      details << "  " << classified->chip_name << "  " << classified->marketing_names
              << (classified->supported_by_build ? "  [supported by build]" : "  [USB candidate; silicon/access not verified]");
    }
    result.push_back({"devourer-id-" + ids.str(), details.str(), bool(classified)});
  }
  libusb_free_device_list(devices, 1); libusb_exit(context);
  return result;
}
std::vector<std::string> discover_radios() {
  std::map<std::string, int> candidates;
  for (const auto& adapter : enumerate_usb()) if (adapter.candidate) ++candidates[adapter.selector];
  std::vector<std::string> selected;
  for (const auto& entry : candidates) {
    if (entry.second > 1) throw std::runtime_error("Identical USB radios found. Disconnect extras or set explicit addresses in the saved profile.");
    selected.push_back(entry.first);
  }
  return selected;
}
}
