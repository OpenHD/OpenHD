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

#include "ohd_interface.h"

#include <wifi_card_discovery.h>
#include <wifi_client.h>
#include <wifi_command_helper.h>

#include <algorithm>
#include <array>
#include <sstream>
#include <utility>

#include "config_paths.h"
#include "ethernet_link.h"
#include "lte_link.h"
#ifdef OHD_ENABLE_ARTOSYN
#include "artosyn_link.h"
#endif
#include "microhard_link.h"
#include "multi_link.h"
#include "openhd_config.h"
#include "openhd_global_constants.hpp"
#include "openhd_sock.h"
#include "openhd_util_filesystem.h"
#include "wb_link.h"
// Helper function to execute a shell command and return the output
std::string exec(const std::string& cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

// Helper function to check if a Microhard device is present
bool is_microhard_device_present() {
  if (!OHDFilesystemUtil::exists(std::string(getConfigBasePath()) +
                                 "wfb.txt") &&
      !OHDFilesystemUtil::exists(std::string(getConfigBasePath()) +
                                 "ethernet.txt")) {
    std::string output = exec("lsusb");
    return output.find("Microhard") != std::string::npos;
  }

  return false;
}

OHDInterface::OHDInterface(OHDProfile profile1, bool disable_wifi_hotspot)
    : m_profile(std::move(profile1)),
      m_disable_wifi_hotspot(disable_wifi_hotspot) {
  m_console = openhd::log::create_or_get("interface");
  assert(m_console);
  m_monitor_mode_cards = {};
  m_opt_hotspot_card = std::nullopt;
  const auto config = openhd::load_config();
  bool microhard_device_present = is_microhard_device_present();
  openhd::LinkActionHandler::instance().set_primary_link_type(
      openhd::LinkActionHandler::PRIMARY_LINK_NONE);
  m_multi_link = std::make_shared<MultiLink>();

  // SysUtils activates WireGuard before OpenHD starts and only exposes the
  // non-secret routing metadata. LTE is an independent MultiLink transport.
  if (const auto settings = openhd::request_sysutil_settings();
      settings && settings->lte_configured && settings->lte_active &&
      !settings->lte_device_id.empty() &&
      !settings->lte_fleetcontrol_address.empty()) {
    LteLinkConfig lte_config{settings->lte_device_id,
                             settings->lte_fleetcontrol_address,
                             settings->lte_video_port,
                             settings->lte_video2_port,
                             settings->lte_telemetry_port};
    m_lte_link = std::make_shared<LteLink>(std::move(lte_config));
    m_multi_link->add_link("LTE", m_lte_link);
  }

#ifdef OHD_ENABLE_ARTOSYN
  if (ArtosynLink::probe()) {
    m_artosyn_link = std::make_shared<ArtosynLink>(m_profile);
    m_multi_link->add_link("ARTOSYN", m_artosyn_link);
    openhd::LinkActionHandler::instance().set_primary_link_type(
        openhd::LinkActionHandler::PRIMARY_LINK_ARTOSYN);
    m_console->warn("artosyn found");
    return;
  }
  m_console->warn(
      "Artosyn probe did not select ArtosynLink, continuing with "
      "ethernet/microhard/wifibroadcast discovery.");
#endif

  if (OHDFilesystemUtil::exists(std::string(getConfigBasePath()) +
                                "ethernet.txt")) {
    m_ethernet_link = std::make_shared<EthernetLink>(m_profile);
    m_multi_link->add_link("ETHERNET", m_ethernet_link);
    openhd::LinkActionHandler::instance().set_primary_link_type(
        openhd::LinkActionHandler::PRIMARY_LINK_ETHERNET);
    m_console->warn("Configured Ethernet link enabled");
  }

  if (microhard_device_present) {
    m_microhard_link = std::make_shared<MicrohardLink>(m_profile);
    m_multi_link->add_link("MICROHARD", m_microhard_link);
    openhd::LinkActionHandler::instance().set_primary_link_type(
        openhd::LinkActionHandler::PRIMARY_LINK_MICROHARD);
    m_console->warn("mc found");
  }

  DWifiCards::main_discover_an_process_wifi_cards(
      config, m_profile, m_console, m_monitor_mode_cards, m_opt_hotspot_card);
  refresh_discovered_wifi_cards();
  m_console->debug("monitor_mode card(s):{}",
                   debug_cards(m_monitor_mode_cards));
  if (m_opt_hotspot_card.has_value()) {
    m_console->debug("Hotspot card:{}", m_opt_hotspot_card.value().device_name);
  } else {
    m_console->debug("No WiFi hotspot card");
  }
  // We don't have at least one card for monitor mode, which means we cannot
  // instantiate wb_link (no wifibroadcast connectivity at all)
  if (m_monitor_mode_cards.empty()) {
    if (!m_ethernet_link && !m_microhard_link) {
      m_console->warn(
          "No monitor-mode WiFi card found; enabling automatic Ethernet link");
      m_ethernet_link = std::make_shared<EthernetLink>(m_profile, true);
      m_multi_link->add_link("ETHERNET", m_ethernet_link);
      openhd::LinkActionHandler::instance().set_primary_link_type(
          openhd::LinkActionHandler::PRIMARY_LINK_ETHERNET);
    }
  } else {
    // Set the card(s) we have into monitor mode
    openhd::wb::takeover_cards_monitor_mode(m_monitor_mode_cards, m_console);
    m_wb_link = std::make_shared<WBLink>(m_profile, m_monitor_mode_cards);
    m_multi_link->add_link("WIFIBROADCAST", m_wb_link);
    m_wb_link->set_fatal_error_callback([this]() { request_wb_recovery(); });
    start_wb_recovery_supervisor();
    openhd::LinkActionHandler::instance().set_primary_link_type(
        openhd::LinkActionHandler::PRIMARY_LINK_WIFIBROADCAST);
    // Ethernet is an always-available secondary transport. It discovers its
    // peer independently and silently drops packets until one is present.
    if (!m_ethernet_link) {
      m_ethernet_link = std::make_shared<EthernetLink>(m_profile, true);
      m_multi_link->add_link("ETHERNET", m_ethernet_link);
    }
  }
  // The USB tethering listener is always enabled on ground - it doesn't
  // interfere with anything
  if (m_profile.is_ground()) {
    // The USB tethering listener is always enabled on ground - it doesn't
    // interfere with anything
    m_usb_tether_listener = std::make_unique<USBTetherListener>();
  }
  // Ethernet - optional, only on ground
  if (m_profile.is_ground()) {
    m_ethernet_manager = std::make_unique<EthernetManager>();
    m_ethernet_manager->async_initialize(
        m_nw_settings.get_settings().ethernet_operating_mode);
    // m_nw_settings.get_settings().ethernet_operating_mode
  }

  // use config defaults if user still relies on legacy WIFI_LOCAL_NETWORK
  if (config.WIFI_LOCAL_NETWORK_ENABLE) {
    auto& settings = m_nw_settings.unsafe_get_settings();
    if (settings.wifi_client_ssid.empty()) {
      settings.wifi_client_ssid = config.WIFI_LOCAL_NETWORK_SSID;
    }
    if (settings.wifi_client_password.empty()) {
      settings.wifi_client_password = config.WIFI_LOCAL_NETWORK_PASSWORD;
    }
    if (settings.wifi_client_interface.empty() && m_opt_hotspot_card) {
      settings.wifi_client_interface = m_opt_hotspot_card->device_name;
    }
    if (!is_valid_wifi_operating_mode(settings.wifi_operating_mode) ||
        settings.wifi_operating_mode == WIFI_MODE_HOTSPOT) {
      settings.wifi_operating_mode = WIFI_MODE_CLIENT;
    }
    m_nw_settings.persist();
  }

  apply_wifi_operating_mode();

  // automatically disable Wi-Fi hotspot if FC is armed
  auto cb = [this](bool /*armed*/) { update_wifi_hotspot_enable(); };
  openhd::ArmingStateHelper::instance().register_listener("ohd_interface_wfi",
                                                          cb);
  m_console->debug("OHDInterface::created");
}

OHDInterface::~OHDInterface() {
  stop_wifi_client();
  m_wifi_hotspot = nullptr;
  if (m_wb_link) m_wb_link->set_fatal_error_callback(nullptr);
  stop_wb_recovery_supervisor();
  // Disconnect callbacks first, then stop endpoint threads while the stable
  // facade and its callback gate are still alive.
  if (m_multi_link) m_multi_link->clear_links();
  m_wb_link = nullptr;
  m_ethernet_link = nullptr;
  m_microhard_link = nullptr;
#ifdef OHD_ENABLE_ARTOSYN
  m_artosyn_link = nullptr;
#endif
  m_multi_link = nullptr;
  // Then give the card(s) back to the system (no monitor mode)
  // give the monitor mode cards back to network manager
  openhd::wb::giveback_cards_monitor_mode(m_monitor_mode_cards, m_console);
  if (m_ethernet_manager) {
    m_ethernet_manager->stop();
    m_ethernet_manager = nullptr;
  }
}

void OHDInterface::refresh_discovered_wifi_cards() {
  m_discovered_wifi_cards = DWifiCards::discover_connected_wifi_cards();
}

bool OHDInterface::is_monitor_mode_card(const std::string& name) const {
  return std::any_of(
      m_monitor_mode_cards.begin(), m_monitor_mode_cards.end(),
      [&name](const auto& card) { return card.device_name == name; });
}

std::optional<WiFiCard> OHDInterface::find_card_by_name(
    const std::string& name) const {
  auto it = std::find_if(
      m_discovered_wifi_cards.begin(), m_discovered_wifi_cards.end(),
      [&name](const auto& card) { return card.device_name == name; });
  if (it != m_discovered_wifi_cards.end()) {
    return *it;
  }
  if (m_opt_hotspot_card && m_opt_hotspot_card->device_name == name) {
    return m_opt_hotspot_card;
  }
  return std::nullopt;
}

std::optional<WiFiCard> OHDInterface::get_configured_hotspot_card() {
  const auto settings = m_nw_settings.get_settings();
  std::string target = settings.wifi_hotspot_interface_override;
  if (target.empty() && m_opt_hotspot_card) {
    target = m_opt_hotspot_card->device_name;
  }
  if (target.empty()) return std::nullopt;
  auto card_opt = find_card_by_name(target);
  if (!card_opt.has_value()) {
    m_console->warn("Requested hotspot interface {} not present", target);
    return std::nullopt;
  }
  if (is_monitor_mode_card(target)) {
    m_console->warn(
        "Interface {} is used for wifibroadcast, cannot use it for "
        "hotspot/client",
        target);
    return std::nullopt;
  }
  return card_opt;
}

std::optional<WiFiCard> OHDInterface::get_configured_client_card() {
  const auto settings = m_nw_settings.get_settings();
  std::string target = settings.wifi_client_interface;
  if (target.empty() && !settings.wifi_hotspot_interface_override.empty()) {
    target = settings.wifi_hotspot_interface_override;
  }
  if (target.empty() && m_opt_hotspot_card) {
    target = m_opt_hotspot_card->device_name;
  }
  if (target.empty()) return std::nullopt;
  auto card_opt = find_card_by_name(target);
  if (!card_opt.has_value()) {
    m_console->warn("Requested client interface {} not present", target);
    return std::nullopt;
  }
  if (is_monitor_mode_card(target)) {
    m_console->warn(
        "Interface {} is used for wifibroadcast, cannot use it for "
        "wifi client mode",
        target);
    return std::nullopt;
  }
  return card_opt;
}

void OHDInterface::recreate_wifi_hotspot_if_needed() {
  if (m_disable_wifi_hotspot) {
    m_wifi_hotspot = nullptr;
    m_current_hotspot_card_name.clear();
    return;
  }
  const auto settings = m_nw_settings.get_settings();
  const auto hotspot_card = get_configured_hotspot_card();
  if (!hotspot_card.has_value()) {
    m_wifi_hotspot = nullptr;
    m_current_hotspot_card_name.clear();
    return;
  }
  const bool hotspot_settings_changed =
      settings.wifi_hotspot_ssid != m_current_hotspot_ssid ||
      settings.wifi_hotspot_password != m_current_hotspot_password;
  if (hotspot_settings_changed) {
    m_wifi_hotspot = nullptr;
    m_current_hotspot_card_name.clear();
  }
  if (m_wifi_hotspot &&
      hotspot_card->device_name == m_current_hotspot_card_name) {
    return;
  }
  const openhd::WifiSpace wb_frequency_space =
      (m_wb_link != nullptr) ? m_wb_link->get_current_frequency_channel_space()
                             : openhd::WifiSpace::G5_8;
  m_wifi_hotspot = std::make_unique<WifiHotspot>(
      m_profile, hotspot_card.value(), wb_frequency_space,
      settings.wifi_hotspot_ssid, settings.wifi_hotspot_password);
  m_current_hotspot_card_name = hotspot_card->device_name;
  m_current_hotspot_ssid = settings.wifi_hotspot_ssid;
  m_current_hotspot_password = settings.wifi_hotspot_password;
}

void OHDInterface::stop_wifi_client() {
  if (!m_wifi_client_active) return;
  WiFiClient::disconnect(m_console);
  m_wifi_client_active = false;
  m_active_wifi_client_card.clear();
}

bool OHDInterface::start_wifi_client() {
  const auto settings = m_nw_settings.get_settings();
  auto card_opt = get_configured_client_card();
  if (!card_opt.has_value()) {
    m_console->warn("No wifi card available for wifi client mode");
    m_wifi_client_active = false;
    m_active_wifi_client_card.clear();
    return false;
  }
  const bool success =
      WiFiClient::connect(card_opt->device_name, settings.wifi_client_ssid,
                          settings.wifi_client_password, m_console);
  m_wifi_client_active = success;
  m_active_wifi_client_card = success ? card_opt->device_name : "";
  return success;
}

std::string OHDInterface::describe_wifi_interfaces() {
  refresh_discovered_wifi_cards();
  if (m_discovered_wifi_cards.empty()) return "none";
  std::stringstream ss;
  for (size_t i = 0; i < m_discovered_wifi_cards.size(); ++i) {
    const auto& card = m_discovered_wifi_cards.at(i);
    std::string role = "idle";
    if (is_monitor_mode_card(card.device_name)) {
      role = "wb";
    } else if (card.device_name == m_current_hotspot_card_name) {
      role = "hotspot";
    } else if (card.device_name == m_active_wifi_client_card) {
      role = "client";
    }
    if (i > 0) ss << ",";
    ss << fmt::format("{}({},{})", card.device_name,
                      wifi_card_type_to_string(card.type), role);
  }
  return ss.str();
}

std::string OHDInterface::describe_discovered_wifi_cards_with_drivers() {
  refresh_discovered_wifi_cards();
  if (m_discovered_wifi_cards.empty()) return "none";
  std::stringstream ss;
  for (size_t i = 0; i < m_discovered_wifi_cards.size(); ++i) {
    const auto& card = m_discovered_wifi_cards.at(i);
    if (i > 0) ss << ", ";
    const std::string driver =
        card.driver_name.empty() ? "unknown" : card.driver_name;
    ss << fmt::format("{}(driver={})", card.device_name, driver);
  }
  return ss.str();
}

bool OHDInterface::apply_link_control(const LinkControlRequest& request,
                                      std::string* error) {
  if (!m_wb_link) {
    if (error) {
      *error = "Wifibroadcast link not active.";
    }
    return false;
  }

  bool ok = true;
  if (request.frequency_mhz.has_value()) {
    ok = m_wb_link->request_set_frequency(*request.frequency_mhz) && ok;
  }
  if (request.channel_width_mhz.has_value()) {
    ok = m_wb_link->request_set_air_tx_channel_width(
             *request.channel_width_mhz) &&
         ok;
  }
  if (request.mcs_index.has_value()) {
    ok = m_wb_link->request_set_air_mcs_index(*request.mcs_index) && ok;
  }
  if (request.tx_power_level.has_value()) {
    ok = m_wb_link->request_set_tx_power_level(*request.tx_power_level) && ok;
  }

  if (request.tx_power_mw.has_value() || request.tx_power_index.has_value()) {
    if (m_monitor_mode_cards.empty()) {
      if (error) {
        *error = "No monitor mode cards available.";
      }
      return false;
    }

    int card_index = 0;
    if (request.interface_name.has_value() &&
        !request.interface_name->empty()) {
      auto it =
          std::find_if(m_monitor_mode_cards.begin(), m_monitor_mode_cards.end(),
                       [&request](const auto& card) {
                         return card.device_name == *request.interface_name;
                       });
      if (it == m_monitor_mode_cards.end()) {
        if (error) {
          *error = "Requested interface is not a monitor mode card.";
        }
        return false;
      }
      card_index =
          static_cast<int>(std::distance(m_monitor_mode_cards.begin(), it));
    }

    if (request.tx_power_index.has_value()) {
      ok = m_wb_link->request_set_tx_power_rtl8812au(
               card_index, *request.tx_power_index, false) &&
           ok;
    } else if (request.tx_power_mw.has_value()) {
      ok = m_wb_link->request_set_tx_power_mw(card_index, *request.tx_power_mw,
                                              false) &&
           ok;
    }
  }

  if (!ok && error && error->empty()) {
    *error = "OpenHD rejected one or more values.";
  }
  return ok;
}

void OHDInterface::apply_wifi_operating_mode() {
  refresh_discovered_wifi_cards();
  auto settings = m_nw_settings.get_settings();
  if (!is_valid_wifi_operating_mode(settings.wifi_operating_mode)) {
    m_console->warn("Invalid wifi operating mode {}, defaulting to hotspot",
                    settings.wifi_operating_mode);
    m_nw_settings.unsafe_get_settings().wifi_operating_mode = WIFI_MODE_HOTSPOT;
    m_nw_settings.persist();
    settings = m_nw_settings.get_settings();
  }
  if (settings.wifi_operating_mode == WIFI_MODE_CLIENT) {
    m_wifi_hotspot = nullptr;
    m_current_hotspot_card_name.clear();
    stop_wifi_client();
    start_wifi_client();
    update_wifi_hotspot_enable();
    return;
  }
  stop_wifi_client();
  if (settings.wifi_operating_mode == WIFI_MODE_HOTSPOT) {
    recreate_wifi_hotspot_if_needed();
  } else {
    m_wifi_hotspot = nullptr;
    m_current_hotspot_card_name.clear();
  }
  update_wifi_hotspot_enable();
}

std::vector<openhd::Setting> OHDInterface::get_all_settings() {
  std::vector<openhd::Setting> ret;
  m_console->debug("get all settings");
  if (m_wb_link) {
    auto settings = m_wb_link->get_all_settings();
    OHDUtil::vec_append(ret, settings);
  }
  if (m_microhard_link) {
    auto settings = m_microhard_link->get_all_settings();
    OHDUtil::vec_append(ret, settings);
  }
#ifdef OHD_ENABLE_ARTOSYN
  if (m_artosyn_link) {
    auto settings = m_artosyn_link->get_all_settings();
    OHDUtil::vec_append(ret, settings);
  }
#endif
  const auto settings = m_nw_settings.get_settings();
  auto cb_wifi_mode = [this](std::string, int value) {
    if (!is_valid_wifi_operating_mode(value)) return false;
    m_nw_settings.unsafe_get_settings().wifi_operating_mode = value;
    m_nw_settings.persist();
    apply_wifi_operating_mode();
    return true;
  };
  ret.push_back(openhd::Setting{
      "WIFI_MODE",
      openhd::IntSetting{settings.wifi_operating_mode, cb_wifi_mode}});
  ret.push_back(openhd::create_read_only_string("WIFI_IFACES",
                                                describe_wifi_interfaces()));
  auto get_primary_link = [this]() -> std::string {
#ifdef OHD_ENABLE_ARTOSYN
    if (m_artosyn_link) {
      return "ARTOSYN";
    }
#endif
    if (m_wb_link) {
      return "WIFIBROADCAST";
    }
    if (m_microhard_link) {
      return "MICROHARD";
    }
    if (m_ethernet_link) {
      return "ETHERNET";
    }
    return "NONE";
  };
  ret.push_back(
      openhd::create_read_only_string("PRIMARY_LINK", get_primary_link()));
  std::string active_links;
  if (m_multi_link) {
    for (const auto& name : m_multi_link->link_names()) {
      if (!active_links.empty()) active_links += "+";
      active_links += name;
    }
  }
  if (active_links.empty()) active_links = "NONE";
  ret.push_back(
      openhd::create_read_only_string("ACTIVE_LINKS", active_links));
  auto cb_wifi_hotspot_mode = [this](std::string, int value) {
    if (!is_valid_wifi_hotspot_mode(value)) return false;
    m_nw_settings.unsafe_get_settings().wifi_hotspot_mode = value;
    m_nw_settings.persist();
    update_wifi_hotspot_enable();
    return true;
  };
  ret.push_back(openhd::Setting{
      "WIFI_HOTSPOT_E",
      openhd::IntSetting{settings.wifi_hotspot_mode, cb_wifi_hotspot_mode}});
  auto cb_hotspot_iface = [this](std::string, std::string value) {
    m_nw_settings.unsafe_get_settings().wifi_hotspot_interface_override = value;
    m_nw_settings.persist();
    apply_wifi_operating_mode();
    return true;
  };
  ret.push_back(openhd::Setting{
      "WIFI_HS_IFACE",
      openhd::StringSetting{settings.wifi_hotspot_interface_override,
                            cb_hotspot_iface}});
  auto cb_hotspot_ssid = [this](std::string, std::string value) {
    m_nw_settings.unsafe_get_settings().wifi_hotspot_ssid = value;
    m_nw_settings.persist();
    if (m_nw_settings.get_settings().wifi_operating_mode == WIFI_MODE_HOTSPOT) {
      recreate_wifi_hotspot_if_needed();
      update_wifi_hotspot_enable();
    }
    return true;
  };
  ret.push_back(openhd::Setting{
      "WIFI_HS_SSID",
      openhd::StringSetting{settings.wifi_hotspot_ssid, cb_hotspot_ssid}});
  auto cb_hotspot_pw = [this](std::string, std::string value) {
    m_nw_settings.unsafe_get_settings().wifi_hotspot_password = value;
    m_nw_settings.persist();
    if (m_nw_settings.get_settings().wifi_operating_mode == WIFI_MODE_HOTSPOT) {
      recreate_wifi_hotspot_if_needed();
      update_wifi_hotspot_enable();
    }
    return true;
  };
  ret.push_back(openhd::Setting{
      "WIFI_HS_PW",
      openhd::StringSetting{settings.wifi_hotspot_password, cb_hotspot_pw}});
  auto cb_client_iface = [this](std::string, std::string value) {
    m_nw_settings.unsafe_get_settings().wifi_client_interface = value;
    m_nw_settings.persist();
    apply_wifi_operating_mode();
    return true;
  };
  ret.push_back(openhd::Setting{
      "WIFI_CL_IFACE",
      openhd::StringSetting{settings.wifi_client_interface, cb_client_iface}});
  auto cb_client_ssid = [this](std::string, std::string value) {
    m_nw_settings.unsafe_get_settings().wifi_client_ssid = value;
    m_nw_settings.persist();
    apply_wifi_operating_mode();
    return true;
  };
  ret.push_back(openhd::Setting{
      "WIFI_CL_SSID",
      openhd::StringSetting{settings.wifi_client_ssid, cb_client_ssid}});
  auto cb_client_pw = [this](std::string, std::string value) {
    m_nw_settings.unsafe_get_settings().wifi_client_password = value;
    m_nw_settings.persist();
    apply_wifi_operating_mode();
    return true;
  };
  ret.push_back(openhd::Setting{
      "WIFI_CL_PW",
      openhd::StringSetting{settings.wifi_client_password, cb_client_pw}});
  if (m_profile.is_ground()) {
    auto cb_ethernet = [this](std::string, int value) {
      m_nw_settings.unsafe_get_settings().ethernet_operating_mode = value;
      m_nw_settings.persist();
      // Change requires reboot
      return true;
    };
    ret.push_back(openhd::Setting{
        "ETHERNET",
        openhd::IntSetting{settings.ethernet_operating_mode, cb_ethernet}});
  }
  /* if(m_opt_hotspot_card){
     auto setting=openhd::create_read_only_string(fmt::format("HOTSPOT_CARD"),
   wifi_card_type_to_string(m_opt_hotspot_card.value().type));
     ret.emplace_back(setting);
   }*/
  openhd::validate_provided_ids(ret);
  return ret;
}

void OHDInterface::print_internal_fec_optimization_method() {
  fec_stream_print_fec_optimization_method();
}

std::shared_ptr<OHDLink> OHDInterface::get_link_handle() {
  return m_multi_link && m_multi_link->link_count() > 0 ? m_multi_link
                                                        : nullptr;
}

void OHDInterface::start_wb_recovery_supervisor() {
  if (m_wb_recovery_thread.joinable()) return;
  {
    std::lock_guard<std::mutex> lock(m_wb_recovery_mutex);
    m_wb_recovery_shutdown = false;
    m_wb_recovery_requested = false;
  }
  m_wb_recovery_thread =
      std::thread(&OHDInterface::wb_recovery_loop, this);
}

void OHDInterface::stop_wb_recovery_supervisor() {
  {
    std::lock_guard<std::mutex> lock(m_wb_recovery_mutex);
    m_wb_recovery_shutdown = true;
  }
  m_wb_recovery_changed.notify_all();
  if (m_wb_recovery_thread.joinable()) m_wb_recovery_thread.join();
}

void OHDInterface::request_wb_recovery() {
  // Quarantine WFB immediately. MultiLink's independent Ethernet/IP transport
  // continues carrying video and telemetry while the supervisor re-probes.
  if (m_multi_link && m_wb_link) m_multi_link->remove_link(m_wb_link);
  {
    std::lock_guard<std::mutex> lock(m_wb_recovery_mutex);
    if (m_wb_recovery_shutdown) return;
    m_wb_recovery_requested = true;
  }
  m_wb_recovery_changed.notify_all();
}

std::optional<std::vector<WiFiCard>>
OHDInterface::find_replugged_wb_cards() {
  const auto discovered = DWifiCards::discover_connected_wifi_cards();
  std::vector<WiFiCard> recovered;
  recovered.reserve(m_monitor_mode_cards.size());
  std::vector<bool> used(discovered.size(), false);
  for (const auto& expected : m_monitor_mode_cards) {
    const auto found = std::find_if(
        discovered.begin(), discovered.end(),
        [&expected, &discovered, &used](const WiFiCard& candidate) {
          const auto index = static_cast<size_t>(&candidate - discovered.data());
          if (used[index]) return false;
          if (expected.driver_name == "devourer") {
            return candidate.driver_name == "devourer" &&
                   candidate.type == expected.type;
          }
          if (!expected.mac.empty()) {
            return candidate.mac == expected.mac;
          }
          return candidate.device_name == expected.device_name;
        });
    if (found == discovered.end()) return std::nullopt;
    used[static_cast<size_t>(found - discovered.begin())] = true;
    auto recovered_card = *found;
    if (recovered_card.device_name != expected.device_name &&
        expected.driver_name != "devourer") {
      m_console->warn("WFB card {} reappeared as {}; restoring its name",
                      expected.mac, recovered_card.device_name);
      if (!wifi::commandhelper::ip_link_rename(recovered_card.device_name,
                                               expected.device_name)) {
        return std::nullopt;
      }
      recovered_card.device_name = expected.device_name;
    }
    recovered.push_back(std::move(recovered_card));
  }
  return recovered;
}

void OHDInterface::wb_recovery_loop() {
  std::unique_lock<std::mutex> lock(m_wb_recovery_mutex);
  while (true) {
    m_wb_recovery_changed.wait(lock, [this]() {
      return m_wb_recovery_shutdown || m_wb_recovery_requested;
    });
    if (m_wb_recovery_shutdown) return;
    lock.unlock();

    const auto recovered_cards = find_replugged_wb_cards();
    bool recovered = false;
    if (recovered_cards) {
      m_console->info("Replugged WFB card(s) found; restoring monitor mode");
      openhd::wb::takeover_cards_monitor_mode(*recovered_cards, m_console);
      recovered = m_wb_link &&
                  m_wb_link->restart_after_card_replug(*recovered_cards);
      if (recovered && m_multi_link) {
        m_multi_link->add_link("WIFIBROADCAST", m_wb_link);
        openhd::LinkActionHandler::instance().set_primary_link_type(
            openhd::LinkActionHandler::PRIMARY_LINK_WIFIBROADCAST);
        m_console->info(
            "Wifibroadcast automatically rejoined the multi-link router");
      }
    }

    lock.lock();
    if (recovered) m_wb_recovery_requested = false;
    if (!m_wb_recovery_requested) continue;
    m_wb_recovery_changed.wait_for(
        lock, std::chrono::seconds(2),
        [this]() { return m_wb_recovery_shutdown; });
    if (m_wb_recovery_shutdown) return;
  }
}

bool OHDInterface::has_primary_link() const {
  return m_multi_link && m_multi_link->link_count() > 0;
}

bool OHDInterface::has_real_monitor_mode_cards() const {
  return std::any_of(m_monitor_mode_cards.begin(), m_monitor_mode_cards.end(),
                     [](const WiFiCard& card) {
                       return card.type != WiFiCardType::OPENHD_EMULATED;
                     });
}

void OHDInterface::generate_keys_from_pw_if_exists_and_delete() {
  // Make sure this stupid sodium init has been called
  if (sodium_init() == -1) {
    std::cerr << "Cannot init libsodium" << std::endl;
    exit(EXIT_FAILURE);
  }
  auto console = openhd::log::get_default();

  const auto password_path = std::string(getConfigBasePath()) + "password.txt";
  if (OHDFilesystemUtil::exists(password_path)) {
    console->debug("password.txt ignored.");
  }
}

void OHDInterface::update_wifi_hotspot_enable() {
  // A requested Nexmon survey temporarily owns the internal interface. Its
  // helper restores the saved NetworkManager state before releasing this file.
  if (OHDFilesystemUtil::exists("/run/openhd-nexmon-scout.json")) return;
  auto& action_handler = openhd::LinkActionHandler::instance();
  const auto& settings = m_nw_settings.get_settings();
  if (m_disable_wifi_hotspot &&
      settings.wifi_operating_mode == WIFI_MODE_HOTSPOT) {
    action_handler.m_wifi_hotspot_state = 0;
    action_handler.m_wifi_hotspot_frequency = 0;
    return;
  }
  if (settings.wifi_operating_mode == WIFI_MODE_OFF) {
    if (m_wifi_hotspot) {
      m_wifi_hotspot->set_enabled_async(false);
    }
    action_handler.m_wifi_hotspot_state = m_wifi_hotspot ? 1 : 0;
    action_handler.m_wifi_hotspot_frequency = 0;
    return;
  }
  if (settings.wifi_operating_mode == WIFI_MODE_CLIENT) {
    action_handler.m_wifi_hotspot_state = m_wifi_client_active ? 2 : 1;
    action_handler.m_wifi_hotspot_frequency = 0;
    return;
  }
  if (!m_wifi_hotspot) {
    action_handler.m_wifi_hotspot_state = 0;
    action_handler.m_wifi_hotspot_frequency = 0;
    return;
  }
  bool enable_wifi_hotspot = false;
  if (settings.wifi_hotspot_mode == WIFI_HOTSPOT_AUTO) {
    bool is_armed = openhd::ArmingStateHelper::instance().is_currently_armed();
    enable_wifi_hotspot = !is_armed;
  } else if (settings.wifi_hotspot_mode == WIFI_HOTSPOT_ALWAYS_OFF) {
    enable_wifi_hotspot = false;
  } else if (settings.wifi_hotspot_mode == WIFI_HOTSPOT_ALWAYS_ON) {
    enable_wifi_hotspot = true;
  } else {
    m_console->warn("Invalid wifi hotspot mode");
    enable_wifi_hotspot = false;
  }
  m_wifi_hotspot->set_enabled_async(enable_wifi_hotspot);
  action_handler.m_wifi_hotspot_state = enable_wifi_hotspot ? 2 : 1;
  action_handler.m_wifi_hotspot_frequency =
      enable_wifi_hotspot ? m_wifi_hotspot->get_frequency() : 0;
}
