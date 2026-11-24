// SPDX-License-Identifier: GPL-3.0-only
#include "SerialPortDiscovery.h"

#include <algorithm>
#include <fstream>
#include <set>
#include <utility>

#include <nlohmann/json.hpp>

namespace openhd::telemetry {
namespace fs = std::filesystem;

std::vector<std::string> discover_serial_ports(const fs::path& tty_class,
                                                const fs::path& device_directory) {
  std::set<std::string> consoles;
  std::ifstream active_consoles(tty_class / "console" / "active");
  for (std::string name; active_consoles >> name;) {
    consoles.insert(name);
  }

  std::vector<std::string> ports;
  std::error_code ec;
  for (fs::directory_iterator entry(tty_class, ec), end; entry != end && !ec;
       entry.increment(ec)) {
    const auto tty = entry->path();
    const auto name = tty.filename().string();
    std::error_code device_error;
    if (consoles.count(name) || !fs::exists(tty / "device", device_error)) continue;

    // The 8250 driver registers placeholder ttyS nodes even without a UART.
    if (name.rfind("ttyS", 0) == 0) {
      std::ifstream type_file(tty / "type");
      int type;
      if ((type_file >> type) && type == 0) continue;
    }

    const auto node = device_directory / name;
    if (fs::is_character_file(node, device_error) && node.string().size() <= 128) {
      ports.push_back(node.string());
    }
  }
  std::sort(ports.begin(), ports.end());
  return ports;
}

nlohmann::json serial_port_metadata() {
  auto choices = nlohmann::json::array({{{"label", "Disabled"}, {"value", ""}}});
  for (const auto& port : discover_serial_ports()) {
    choices.push_back({{"label", port}, {"value", port}});
  }
  return {{"serialPortScan", true}, {"choices", std::move(choices)}};
}

}  // namespace openhd::telemetry
