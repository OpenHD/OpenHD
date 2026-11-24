// SPDX-License-Identifier: GPL-3.0-only
#include "SerialPortDiscovery.h"

#include <algorithm>
#include <array>
#include <iterator>
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

namespace {

std::string read_property(const fs::path& path) {
  std::ifstream input(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

// The same Linux tty can be muxed away from the header. Check the running
// device tree before claiming that a port is wired to these physical pins.
nlohmann::json zero3_header_routes(const std::vector<std::string>& ports,
                                   const fs::path& dt = "/sys/firmware/devicetree/base",
                                   const fs::path& tty_class = "/sys/class/tty") {
  auto routes = nlohmann::json::object();
  const auto compatible = read_property(dt / "compatible");
  if (compatible.find("radxa,zero3") == std::string::npos) return routes;

  struct HeaderUart {
    const char* symbol;
    int rx;
    int tx;
  };
  // Radxa ZERO 3 hardware interface, physical pin numbers.
  constexpr std::array<HeaderUart, 5> pin_pairs{{
      {"uart2m0_xfer", 10, 8}, {"uart3m0_xfer", 3, 5},
      {"uart4m1_xfer", 16, 18}, {"uart5m1_xfer", 33, 32},
      {"uart9m1_xfer", 24, 21},
  }};
  for (const auto& port : ports) {
    const auto pins = read_property(tty_class /
                                    fs::path(port).filename() /
                                    "device/of_node/pinctrl-0");
    if (pins.empty()) continue;
    for (const auto& pair : pin_pairs) {
      const auto node = read_property(dt / "__symbols__" / pair.symbol);
      if (node.empty() || node.front() != '/') continue;
      const auto phandle = read_property(dt / (node.c_str() + 1) / "phandle");
      if (phandle.size() != 4) continue;
      for (size_t offset = 0; offset + 4 <= pins.size(); offset += 4) {
        if (pins.compare(offset, 4, phandle) == 0) {
          routes[port] = {{"rx", pair.rx}, {"tx", pair.tx}};
          break;
        }
      }
      if (routes.contains(port)) break;
    }
  }
  return routes;
}

}  // namespace

nlohmann::json serial_port_metadata(const std::string& configured,
                                    const std::string& resolved) {
  const auto ports = discover_serial_ports();
  auto choices = nlohmann::json::array({{{"label", "Disabled"}, {"value", ""}}});
  for (const auto& port : ports) {
    choices.push_back({{"label", port}, {"value", port}});
  }
  std::error_code ec;
  const auto canonical = resolved.empty() ? fs::path{} : fs::canonical(resolved, ec);
  return {{"serialPortScan", true}, {"choices", std::move(choices)},
          {"serialHeaderRoutes", zero3_header_routes(ports)},
          {"serialConfigured", configured},
          {"serialResolved", ec ? "" : canonical.string()}};
}

}  // namespace openhd::telemetry
