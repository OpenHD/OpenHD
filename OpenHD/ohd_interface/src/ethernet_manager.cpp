/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#include "ethernet_manager.h"

#include <utility>

#include "networking_settings.h"
#include "openhd_config.h"
#include "openhd_external_device.h"
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

static void delete_existing_hotspot_connection() {
  // Delete through NetworkManager even if the expected keyfile is missing.
  // Profiles can use a different filename or still be loaded in memory.
  OHDUtil::run_command(
      "nmcli", {"con", "delete", OHD_ETHERNET_HOTSPOT_CONNECTION_NAME},
      false);
}

static void create_ethernet_hotspot_connection_if_needed(
    const std::shared_ptr<spdlog::logger>& m_console,
    const std::string& eth_device_name) {
  // Recreate the profile so images upgraded from older OpenHD releases do not
  // retain a profile bound to eth0. Rock 5B commonly uses enP4p65s0 and newer
  // kernels may choose another predictable interface name.
  delete_existing_hotspot_connection();
  m_console->warn("Creating Ethernet hotspot on {}", eth_device_name);
  const auto create_result = OHDUtil::run_command(
      "nmcli",
      {"con", "add", "type", "ethernet", "ifname", eth_device_name,
       "con-name", OHD_ETHERNET_HOTSPOT_CONNECTION_NAME,
       "connection.autoconnect", "yes", "connection.autoconnect-priority",
       "100", "ipv4.method", "shared", "ipv4.addresses", "192.168.2.1/24",
       "ipv6.method", "disabled"});
  if (create_result != 0) {
    m_console->error("Cannot create Ethernet hotspot on {}", eth_device_name);
    return;
  }
  const auto activate_result = OHDUtil::run_command(
      "nmcli", {"con", "up", OHD_ETHERNET_HOTSPOT_CONNECTION_NAME, "ifname",
                eth_device_name});
  if (activate_result != 0) {
    m_console->error("Cannot activate Ethernet hotspot on {}", eth_device_name);
    return;
  }
  m_console->warn("Ethernet hotspot active on {} at 192.168.2.1",
                  eth_device_name);
}

static std::optional<std::string> find_ethernet_device_name() {
  auto devices = OHDFilesystemUtil::getAllEntriesFilenameOnlyInDirectory(
      "/sys/class/net/");
  for (auto& device : devices) {
    // Linux predictable Ethernet names include eno*, ens*, enp*, enx* and,
    // on Rock 5B, enP*. Some Rockchip kernels also expose eth*.
    if (OHDUtil::startsWith(device, "en") ||
        OHDUtil::startsWith(device, "eth")) {
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
  const auto config = openhd::load_config();
  if (openhd::nw_ethernet_card_manual_active(config)) {
    const auto configured_card = config.NW_ETHERNET_CARD;
    const auto configured_path =
        fmt::format("/sys/class/net/{}", configured_card);
    if (OHDFilesystemUtil::exists(configured_path)) {
      opt_ethernet_card = configured_card;
    } else {
      m_console->warn(
          "Configured Ethernet interface {} does not exist, auto-detecting",
          configured_card);
    }
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
  m_console->warn("stop begin");
  m_terminate = true;
  if (m_thread) {
    m_thread->join();
    m_thread = nullptr;
  }
  m_console->warn("stop end");
}

void EthernetManager::configure(int operating_mode,
                                const std::string& ethernet_card) {
  m_console->warn("configure {}", ethernet_card);
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
      m_console->warn("Eth0 is up");
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
      m_console->warn("Eth0 is not up anymore,removing ext device");
      break;
    }
  }
  openhd::ExternalDeviceManager::instance().on_new_external_device(
      external_device, false);
}
