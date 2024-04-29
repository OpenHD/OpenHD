//
// Created by consti10 on 25.04.24.
//

#include "ethernet_manager.h"

#include <regex>
#include <utility>

#include "networking_settings.h"
#include "openhd_config.h"
#include "openhd_external_device.h"
#include "openhd_platform.h"
#include "openhd_profile.h"
#include "openhd_util.h"
#include "openhd_util_async.h"

// Quick helper methods for ethernet (used for automatic data forwarding
// detection)
namespace openhd::ethernet {

// Check if the given ethernet device is in an "up" state by reading linux
// file(s)
static bool check_eth_adapter_up(const std::string& eth_device_name = "eth0") {
  const auto filename_operstate =
      fmt::format("/sys/class/net/{}/operstate", eth_device_name);
  if (!OHDFilesystemUtil::exists(filename_operstate)) return false;
  const auto content_opt = OHDFilesystemUtil::opt_read_file(filename_operstate);
  if (!content_opt.has_value()) return false;
  const auto& content = content_opt.value();
  if (OHDUtil::contains(content, "up")) {
    return true;
  }
  return false;
}

// find / get the ip address in the given string with the following layout
// ...(192.168.2.158)... where "192.168.2.158" can be any ip address
static std::string get_ip_address_in_between_brackets(
    const std::string& s, const bool debug = false) {
  const std::regex base_regex("\\((.*)\\)");
  std::smatch base_match;
  std::string matched;
  if (std::regex_search(s, base_match, base_regex)) {
    // The first sub_match is the whole string; the next
    // sub_match is the first parenthesized expression.
    if (base_match.size() == 2) {
      matched = base_match[1].str();
    }
  }
  if (debug) {
    openhd::log::get_default()->debug("Given:[{}] Result:[{}]", s, matched);
  }
  return matched;
}

}  // namespace openhd::ethernet

/**
 * (quite specific, but proven to be
 * popular) functionality of configuring the ground station to act as a DHCP
 * provider (Hotspot) on the ethernet port and then detecting if a device is
 * connected via ethernet - this device then becomes a classic "external device"
 * regarding video and telemetry forwarding.
 * NOTE: Enabling / disabling this feature requires a reboot of the system (this
 * stuff is just way too "dirty" to do it any other way) and we try and avoid
 * touching the networking of the host device from the OpenHD main executable
 * for users that run OpenHD / QOpenHD on their own ubuntu installation.
 */
static constexpr auto OHD_ETHERNET_HOTSPOT_CONNECTION_NAME = "ohd_eth_hotspot";

static std::string get_ohd_eth_hotspot_connection_nm_filename() {
  return fmt::format("/etc/NetworkManager/system-connections/{}.nmconnection",
                     OHD_ETHERNET_HOTSPOT_CONNECTION_NAME);
}

static void delete_existing_hotspot_connection() {
  const auto filename = get_ohd_eth_hotspot_connection_nm_filename();
  if (OHDFilesystemUtil::exists(filename)) {
    OHDUtil::run_command(
        "nmcli", {"con", "delete", OHD_ETHERNET_HOTSPOT_CONNECTION_NAME});
  }
}

static void create_ethernet_hotspot_connection_if_needed(
    const std::shared_ptr<spdlog::logger>& m_console,
    const std::string& eth_device_name) {
  // sudo nmcli con add type ethernet con-name "ohd_eth_hotspot" ipv4.method
  // shared ifname eth0 ipv4.addresses 192.168.2.1/24 gw4 192.168.2.1 sudo nmcli
  // con add type ethernet ifname eth0 con-name ohd_eth_hotspot autoconnect no
  // sudo nmcli con modify ohd_eth_hotspot ipv4.method shared ifname eth0
  // ipv4.addresses 192.168.2.1/24 gw4 192.168.2.1
  if (OHDFilesystemUtil::exists(get_ohd_eth_hotspot_connection_nm_filename())) {
    m_console->debug("Eth hs connection already exists");
    return;
  }
  m_console->debug("begin create hotspot connection");
  OHDUtil::run_command(
      "nmcli", {"con add type ethernet ifname", eth_device_name, "con-name",
                OHD_ETHERNET_HOTSPOT_CONNECTION_NAME});
  OHDUtil::run_command("nmcli",
                       {"con modify", OHD_ETHERNET_HOTSPOT_CONNECTION_NAME,
                        "ipv4.method shared ifname eth0 ipv4.addresses "
                        "192.168.2.1/24 gw4 192.168.2.1"});
  m_console->debug("end create hotspot connection");
}

static std::optional<std::string> find_ethernet_device_name() {
  auto devices = OHDFilesystemUtil::getAllEntriesFilenameOnlyInDirectory(
      "/sys/class/net/");
  for (auto& device : devices) {
    if (OHDUtil::startsWith(device, "enx") ||
        OHDUtil::startsWith(device, "eth") ||
        OHDUtil::startsWith(device, "enp")) {
      return device;
    }
  }
  return std::nullopt;
}

EthernetManager::EthernetManager() {
  m_console = openhd::log::create_or_get("eth_manager");
}

void EthernetManager::async_initialize(int operating_mode) {
  auto runnable = [this, operating_mode]() { loop(operating_mode); };
  m_terminate = false;
  m_thread = std::make_shared<std::thread>(runnable);
}

void EthernetManager::loop(int operating_mode) {
  if (operating_mode == ETHERNET_OPERATING_MODE_UNTOUCHED) {
    delete_existing_hotspot_connection();
    return;
  }
  // First, we need to find the ethernet adapter. On some platforms, it's name
  // is fixed (built in) If a usb to ethernet is used, that's not the case
  std::optional<std::string> opt_ethernet_card = std::nullopt;
  const auto platform = OHDPlatform::instance();
  const auto config = openhd::load_config();
  if (openhd::nw_ethernet_card_manual_active(config)) {
    opt_ethernet_card = config.NW_ETHERNET_CARD;
  } else if (platform.is_rpi()) {
    opt_ethernet_card = std::string("eth0");
  } else if (platform.is_rock5_b()) {
    opt_ethernet_card = "enP4p65s0";
  } else if (platform.is_rock5_a()) {
    opt_ethernet_card = "eth0";
  }
  if (opt_ethernet_card == std::nullopt) {
    // We need to figure out the ethernet card ourselves
    while (!m_terminate) {
      auto card = find_ethernet_device_name();
      if (card.has_value()) {
        opt_ethernet_card = card;
        break;
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  if (opt_ethernet_card) {
    configure(operating_mode, opt_ethernet_card.value());
  }
}

void EthernetManager::stop() {
  m_console->debug("stop begin");
  m_terminate = true;
  if (m_thread) {
    m_thread->join();
    m_thread = nullptr;
  }
  m_console->debug("stop end");
}

void EthernetManager::configure(int operating_mode,
                                const std::string& ethernet_card) {
  m_console->debug("configure {}", ethernet_card);
  if (operating_mode == ETHERNET_OPERATING_MODE_HOTSPOT) {
    create_ethernet_hotspot_connection_if_needed(m_console, ethernet_card);
  } else {
    while (!m_terminate) {
      loop_ethernet_external_device_listener(ethernet_card);
    }
  }
}

void EthernetManager::loop_ethernet_external_device_listener(
    const std::string& device_name) {
  while (!m_terminate) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (openhd::ethernet::check_eth_adapter_up(device_name)) {
      m_console->debug("Eth0 is up");
      break;
    }
  }
  const auto run_command_result_opt = OHDUtil::run_command_out(
      fmt::format("ip route list dev {}", device_name));
  if (run_command_result_opt == std::nullopt) {
    m_console->warn("run command out no result");
    return;
  }
  const auto& run_command_result = run_command_result_opt.value();
  const auto ip_external_device =
      OHDUtil::string_in_between("default via ", " proto", run_command_result);
  // const auto ip_self_network= OHDUtil::string_in_between("src ","
  // metric",run_command_result);
  const std::string tag = "ETH_" + device_name;
  const auto external_device = openhd::ExternalDevice{tag, ip_external_device};
  // Check if both are valid IPs (otherwise, perhaps the parsing got fucked up)
  if (!external_device.is_valid()) {
    m_console->warn("{} not valid", external_device.to_string());
    return;
  }
  m_console->info("found device:{}", external_device.to_string());
  openhd::ExternalDeviceManager::instance().on_new_external_device(
      external_device, true);
  // check in regular intervals if the device disconnects
  while (!m_terminate) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // check if the state is still okay
    if (!openhd::ethernet::check_eth_adapter_up(device_name)) {
      m_console->debug("Eth0 is not up anymore,removing ext device");
      break;
    }
  }
  openhd::ExternalDeviceManager::instance().on_new_external_device(
      external_device, false);
}
