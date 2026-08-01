#include "ip_camera_network.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "openhd_config.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

namespace {

bool is_ethernet_interface(const ifaddrs& interface) {
  if (interface.ifa_name == nullptr ||
      (interface.ifa_flags & IFF_LOOPBACK) != 0 ||
      (interface.ifa_flags & IFF_UP) == 0) {
    return false;
  }
  const std::string name{interface.ifa_name};
  return name.rfind("en", 0) == 0 || name.rfind("eth", 0) == 0;
}

std::vector<std::string> active_ethernet_interfaces() {
  const auto config = openhd::load_config();
  if (openhd::nw_ethernet_card_manual_active(config) &&
      OHDFilesystemUtil::exists("/sys/class/net/" + config.NW_ETHERNET_CARD)) {
    return {config.NW_ETHERNET_CARD};
  }
  std::set<std::string> names;
  ifaddrs* interfaces = nullptr;
  if (getifaddrs(&interfaces) != 0) return {};
  for (auto* current = interfaces; current != nullptr;
       current = current->ifa_next) {
    if (current->ifa_name != nullptr && is_ethernet_interface(*current)) {
      names.emplace(current->ifa_name);
    }
  }
  freeifaddrs(interfaces);
  return {names.begin(), names.end()};
}

bool has_local_subnet(uint32_t network_host_order) {
  ifaddrs* interfaces = nullptr;
  if (getifaddrs(&interfaces) != 0) return false;
  bool found = false;
  for (auto* current = interfaces; current != nullptr;
       current = current->ifa_next) {
    if (current->ifa_addr == nullptr ||
        current->ifa_addr->sa_family != AF_INET ||
        !is_ethernet_interface(*current)) {
      continue;
    }
    const auto* address =
        reinterpret_cast<const sockaddr_in*>(current->ifa_addr);
    if ((ntohl(address->sin_addr.s_addr) & 0xffffff00U) == network_host_order) {
      found = true;
      break;
    }
  }
  freeifaddrs(interfaces);
  return found;
}

std::string address_to_string(uint32_t address_network_order) {
  in_addr address{};
  address.s_addr = address_network_order;
  char text[INET_ADDRSTRLEN]{};
  return inet_ntop(AF_INET, &address, text, sizeof(text)) == nullptr
             ? std::string{}
             : std::string{text};
}

}  // namespace

bool ensure_ip_camera_route(const std::string& camera_address) {
  in_addr parsed{};
  if (inet_pton(AF_INET, camera_address.c_str(), &parsed) != 1) return false;
  const uint32_t camera_host_order = ntohl(parsed.s_addr);
  const uint32_t network = camera_host_order & 0xffffff00U;
  if (has_local_subnet(network)) return true;

  const auto interfaces = active_ethernet_interfaces();
  if (interfaces.empty()) return false;
  uint32_t local_host_order = network | 20U;
  if (local_host_order == camera_host_order) local_host_order = network | 21U;
  const auto local_address = address_to_string(htonl(local_host_order));
  if (local_address.empty()) return false;
  return OHDUtil::run_command("ip",
                              {"address", "add", local_address + "/24", "dev",
                               interfaces.front()},
                              false) == 0 ||
         has_local_subnet(network);
}
