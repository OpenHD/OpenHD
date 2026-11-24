// SPDX-License-Identifier: GPL-3.0-only
#include "SerialPortDiscovery.h"

#include <algorithm>
#include <iterator>
#include <fstream>
#include <set>
#include <string_view>
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

struct HeaderUart {
  const char* symbol;
  int rx;
  int tx;
};

bool contains_dt_string(const std::string& property, std::string_view value) {
  for (std::size_t start = 0; start < property.size();) {
    const auto end = property.find('\0', start);
    if (std::string_view(property).substr(start, end - start) == value) return true;
    if (end == std::string::npos) break;
    start = end + 1;
  }
  return false;
}

// Physical pin numbers from the Radxa hardware interface documentation.
// For boards with different revisions, use UART routes shared by the revisions.
std::vector<HeaderUart> rockchip_header_uarts(const std::string& compatible) {
  auto has_compatible = [&](std::string_view name) {
    return contains_dt_string(compatible, name);
  };

  if (has_compatible("radxa,zero3w") || has_compatible("radxa,zero3e") ||
      has_compatible("radxa,zero3") || has_compatible("radxa,rock-3c")) {
    return {{"uart2m0_xfer", 10, 8}, {"uart3m0_xfer", 3, 5},
            {"uart4m1_xfer", 16, 18}, {"uart5m1_xfer", 33, 32},
            {"uart9m1_xfer", 24, 21}};
  }
  if (has_compatible("radxa,rock3a") || has_compatible("radxa,rock-3a") ||
      has_compatible("radxa,rock-3b")) {
    return {{"uart2m0_xfer", 10, 8}, {"uart3m0_xfer", 3, 5},
            {"uart5m1_xfer", 33, 32}, {"uart9m1_xfer", 24, 21}};
  }
  if (has_compatible("radxa,rock-5a")) {
    return {{"uart2m0_xfer", 10, 8}, {"uart4m2_xfer", 29, 7},
            {"uart8m0_xfer", 40, 32}};
  }
  if (has_compatible("radxa,rock-5b") || has_compatible("radxa,rock-5b-plus")) {
    return {{"uart2m0_xfer", 10, 8}, {"uart2m2_xfer", 38, 36},
            {"uart3m1_xfer", 35, 12}, {"uart4m2_xfer", 19, 23},
            {"uart7m1_xfer", 11, 15}, {"uart7m2_xfer", 24, 26}};
  }
  return {};
}

nlohmann::json cubie_header_routes(const std::vector<std::string>& ports,
                                    const fs::path& dt,
                                    const fs::path& tty_class) {
  struct Route {
    const char* function;
    const char* rx_gpio;
    const char* tx_gpio;
    int rx;
    int tx;
  };
  // Both A733 Cubies share these positions on their main GPIO header. The
  // A7S auxiliary header is separate and has a different numbering space.
  constexpr Route wired_routes[] = {
      {"uart0", "PB10", "PB9", 10, 8}, {"uart0", "PB1", "PB0", 11, 7},
      {"uart2", "PB1", "PB0", 11, 7}, {"uart3", "PJ23", "PJ22", 3, 5},
      {"uart3", "PD17", "PD16", 27, 28}, {"uart4", "PJ25", "PJ24", 18, 16},
  };

  auto routes = nlohmann::json::object();
  std::error_code error;
  for (fs::recursive_directory_iterator group(dt, error), end;
       group != end && !error; group.increment(error)) {
    if (!group->is_directory(error) || error) continue;
    const auto phandle = read_property(group->path() / "phandle");
    if (phandle.size() != 4) continue;
    const auto function = read_property(group->path() / "function");
    if (function.compare(0, 4, "uart") != 0) continue;
    const auto gpios = read_property(group->path() / "pins");

    for (const auto& route : wired_routes) {
      if (!contains_dt_string(function, route.function) ||
          !contains_dt_string(gpios, route.rx_gpio) ||
          !contains_dt_string(gpios, route.tx_gpio)) continue;
      for (const auto& port : ports) {
        const auto pins = read_property(tty_class / fs::path(port).filename() /
                                        "device/of_node/pinctrl-0");
        for (size_t offset = 0; offset + 4 <= pins.size(); offset += 4) {
          if (pins.compare(offset, 4, phandle) == 0) {
            routes[port] = {{"rx", route.rx}, {"tx", route.tx}};
            break;
          }
        }
      }
    }
  }
  return routes;
}

// The same tty can be muxed away from the header. Match the actual pinctrl
// phandles against the documented header routes for this exact board family.
nlohmann::json serial_header_routes(const std::vector<std::string>& ports,
                                    const fs::path& dt,
                                    const fs::path& tty_class = "/sys/class/tty") {
  auto routes = nlohmann::json::object();
  const auto compatible = read_property(dt / "compatible");
  if (contains_dt_string(compatible, "radxa,cubie-a7s") ||
      contains_dt_string(compatible, "radxa,cubie-a7z")) {
    return cubie_header_routes(ports, dt, tty_class);
  }
  const auto pin_pairs = rockchip_header_uarts(compatible);
  if (pin_pairs.empty()) return routes;
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
  const fs::path dt = "/sys/firmware/devicetree/base";
  auto board_model = read_property(dt / "model");
  board_model.resize(std::min(board_model.size(), board_model.find('\0')));
  const auto ports = discover_serial_ports();
  auto choices = nlohmann::json::array({{{"label", "Disabled"}, {"value", ""}}});
  for (const auto& port : ports) {
    choices.push_back({{"label", port}, {"value", port}});
  }
  std::error_code ec;
  const auto canonical = resolved.empty() ? fs::path{} : fs::canonical(resolved, ec);
  return {{"serialPortScan", true}, {"choices", std::move(choices)},
          {"serialHeaderRoutes", serial_header_routes(ports, dt)},
          {"serialBoardModel", board_model},
          {"serialConfigured", configured},
          {"serialResolved", ec ? "" : canonical.string()}};
}

}  // namespace openhd::telemetry
