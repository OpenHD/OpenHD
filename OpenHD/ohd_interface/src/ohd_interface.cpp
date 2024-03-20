//
// Created by consti10 on 02.05.22.
//

#include "ohd_interface.h"

#include <wifi_card_discovery.h>
#include <wifi_client.h>

#include <utility>

#include "openhd_config.h"
#include "openhd_global_constants.hpp"
#include "openhd_util_filesystem.h"
#include "wb_link.h"

OHDInterface::OHDInterface(OHDPlatform platform1, OHDProfile profile1)
    : m_platform(platform1), m_profile(std::move(profile1)) {
  m_console = openhd::log::create_or_get("interface");
  assert(m_console);
  m_monitor_mode_cards = {};
  m_opt_hotspot_card = std::nullopt;
  const auto config = openhd::load_config();
  DWifiCards::main_discover_an_process_wifi_cards(
      config, m_profile, m_platform, m_console, m_monitor_mode_cards,
      m_opt_hotspot_card);
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
    m_console->warn(
        "Cannot start ohd_interface, no wifi card for monitor mode");
    const std::string message_for_user = "No WiFi card found, please reboot";
    m_console->warn(message_for_user);
    openhd::LEDManager::instance().set_status_error();
    // TODO reason what to do. We do not support dynamically adding wifi cards
    // at run time, so somehow we need to signal to the user that something is
    // completely wrong. However, as an Ground pi, we can still run QOpenHD and
    // OpenHD, just it will never connect to an Air PI
  } else {
    // Set the card(s) we have into monitor mode
    openhd::wb::takeover_cards_monitor_mode(m_monitor_mode_cards, m_console);
    m_wb_link =
        std::make_shared<WBLink>(m_profile, m_platform, m_monitor_mode_cards);
  }
  // The USB tethering listener is always enabled on ground - it doesn't
  // interfere with anything
  if (m_profile.is_ground()) {
    // The USB tethering listener is always enabled on ground - it doesn't
    // interfere with anything
    m_usb_tether_listener = std::make_unique<USBTetherListener>();
  }
  // OpenHD can (but must not) "control" ethernet via network manager.
  // On rpi, we do that by default when on ground, on other platforms, only if
  // the user explicitly requested it.
  std::optional<std::string> opt_ethernet_card = std::nullopt;
  if (openhd::nw_ethernet_card_manual_active(config)) {
    opt_ethernet_card = config.NW_ETHERNET_CARD;
  } else if (m_profile.is_ground() && m_platform.is_rpi()) {
    opt_ethernet_card = std::string("eth0");
  }
  if (opt_ethernet_card != std::nullopt) {
    const std::string ethernet_card = opt_ethernet_card.value();
    m_console->debug("OpenHD manages {}", ethernet_card);
    // NOTE: Persistence is a bit complicated with ethernet hotspot (we write a
    // nm connection that needs to stay there such that after a reboot, the rpi
    // correctly the ethernet
    m_ethernet_hotspot = std::make_unique<EthernetHotspot>(ethernet_card);
    m_ethernet_hotspot->set_enabled(
        m_nw_settings.get_settings().ethernet_hotspot_enable);
    m_ethernet_listener = std::make_unique<EthernetListener>(ethernet_card);
    m_ethernet_listener->set_enabled(
        m_nw_settings.get_settings()
            .ethernet_nonhotspot_enable_auto_forwarding);
  } else {
    m_console->debug("OpenHD does not manage ethernet");
  }
  // Wi-Fi hotspot functionality if possible.
  if (m_opt_hotspot_card.has_value()) {
    if (WiFiClient::create_if_enabled()) {
      // Wifi client active
    } else {
      const openhd::WifiSpace wb_frequency_space =
          (m_wb_link != nullptr)
              ? m_wb_link->get_current_frequency_channel_space()
              : openhd::WifiSpace::G5_8;
      // OHD hotspot needs to know the wifibroadcast frequency - it is always on
      // the opposite spectrum
      m_wifi_hotspot = std::make_unique<WifiHotspot>(
          m_profile, m_opt_hotspot_card.value(), wb_frequency_space);
      update_wifi_hotspot_enable();
    }
  }
  // automatically disable Wi-Fi hotspot if FC is armed
  if (m_wifi_hotspot) {
    auto cb = [this](bool armed) { update_wifi_hotspot_enable(); };
    openhd::ArmingStateHelper::instance().register_listener("ohd_interface_wfi",
                                                            cb);
  }
  m_console->debug("OHDInterface::created");
}

std::vector<openhd::Setting> OHDInterface::get_all_settings() {
  std::vector<openhd::Setting> ret;
  if (m_wb_link) {
    auto settings = m_wb_link->get_all_settings();
    OHDUtil::vec_append(ret, settings);
  }
  if (m_wifi_hotspot != nullptr) {
    auto cb_wifi_hotspot_mode = [this](std::string, int value) {
      if (!is_valid_wifi_hotspot_mode(value)) return false;
      m_nw_settings.unsafe_get_settings().wifi_hotspot_mode = value;
      m_nw_settings.persist();
      update_wifi_hotspot_enable();
      return true;
    };
    ret.push_back(openhd::Setting{
        "WIFI_HOTSPOT_E",
        openhd::IntSetting{m_nw_settings.get_settings().wifi_hotspot_mode,
                           cb_wifi_hotspot_mode}});
  }
  if (m_ethernet_hotspot) {
    const auto settings = m_nw_settings.get_settings();
    auto cb_enable = [this](std::string, int value) {
      if (!openhd::validate_yes_or_no(value)) return false;
      // Cannot be enabled while ethernet passive forwarding is active
      if (m_nw_settings.unsafe_get_settings()
              .ethernet_nonhotspot_enable_auto_forwarding) {
        m_console->warn("Please disable ethernet passive forwarding");
        return false;
      }
      m_nw_settings.unsafe_get_settings().ethernet_hotspot_enable = value;
      m_nw_settings.persist();
      // to apply, might require reboot !!
      m_ethernet_hotspot->set_enabled(value);
      return true;
    };
    ret.push_back(openhd::Setting{
        "ETH_HOTSPOT_E",
        openhd::IntSetting{settings.ethernet_hotspot_enable, cb_enable}});
  }
  if (m_ethernet_listener) {
    const auto settings = m_nw_settings.get_settings();
    auto cb_enable = [this](std::string, int value) {
      if (!openhd::validate_yes_or_no(value)) return false;
      // Cannot be enabled while ethernet hotspot is active
      if (m_nw_settings.unsafe_get_settings().ethernet_hotspot_enable) {
        m_console->warn("Please disable ethernet hotspot");
        return false;
      }
      m_nw_settings.unsafe_get_settings()
          .ethernet_nonhotspot_enable_auto_forwarding = value;
      m_nw_settings.persist();
      // Doesn't need reboot
      m_ethernet_listener->set_enabled(value);
      return true;
    };
    ret.push_back(openhd::Setting{
        "ETH_PASSIVE_F",
        openhd::IntSetting{settings.ethernet_nonhotspot_enable_auto_forwarding,
                           cb_enable}});
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
  if (m_wb_link) {
    return m_wb_link;
  }
  return nullptr;
}

void OHDInterface::generate_keys_from_pw_if_exists_and_delete() {
  // Make sure this stupid sodium init has been called
  if (sodium_init() == -1) {
    std::cerr << "Cannot init libsodium" << std::endl;
    exit(EXIT_FAILURE);
  }
  auto console = openhd::log::get_default();
  static constexpr auto PW_FILENAME = "/boot/openhd/password.txt";
  if (OHDFilesystemUtil::exists(PW_FILENAME)) {
    auto pw = OHDFilesystemUtil::read_file(PW_FILENAME);
    OHDUtil::trim(pw);
    console->info("Generating key(s) from pw [{}]",
                  OHDUtil::password_as_hidden_str(pw));  // don't show the pw
    auto keys = wb::generate_keypair_from_bind_phrase(pw);
    if (wb::write_keypair_to_file(keys, openhd::SECURITY_KEYPAIR_FILENAME)) {
      console->debug("Keypair file successfully written");
      // delete the file
      OHDFilesystemUtil::remove_if_existing(PW_FILENAME);
      OHDFilesystemUtil::make_file_read_write_everyone(
          openhd::SECURITY_KEYPAIR_FILENAME);
    } else {
      console->error("Cannot write keypair file !");
      OHDFilesystemUtil::remove_if_existing(openhd::SECURITY_KEYPAIR_FILENAME);
    }
  }
  // If no keypair file exists (It was not created from the password.txt file)
  // we create the txrx.key once (from the default password) such that the boot
  // up time is sped up on successive boot(s)
  auto val = wb::read_keypair_from_file(openhd::SECURITY_KEYPAIR_FILENAME);
  if ((!OHDFilesystemUtil::exists(openhd::SECURITY_KEYPAIR_FILENAME)) ||
      (!val)) {
    console->debug("Creating txrx.key from default pw (once)");
    auto keys = wb::generate_keypair_from_bind_phrase(wb::DEFAULT_BIND_PHRASE);
    wb::write_keypair_to_file(keys, openhd::SECURITY_KEYPAIR_FILENAME);
  }
}

void OHDInterface::update_wifi_hotspot_enable() {
  assert(m_wifi_hotspot);
  const auto& settings = m_nw_settings.get_settings();
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
  openhd::LinkActionHandler::instance().m_wifi_hotspot_state =
      enable_wifi_hotspot ? 2 : 1;
  openhd::LinkActionHandler::instance().m_wifi_hotspot_frequency =
      m_wifi_hotspot->get_frequency();
}

OHDInterface::~OHDInterface() {
  // Terminate the link first
  m_wb_link = nullptr;
  // Then give the card(s) back to the system (no monitor mode)
  // give the monitor mode cards back to network manager
  openhd::wb::giveback_cards_monitor_mode(m_monitor_mode_cards, m_console);
}
