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

#include <utility>
#include <iostream>

#include "config_paths.h"
#include "ethernet_link.h"
#include "microhard_link.h"
#include "openhd_config.h"
#include "openhd_global_constants.hpp"
#include "openhd_util_filesystem.h"
#include "wb_link.h"

// Helper function to execute a shell command and return the output
std::string exec(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        std::cerr << "[DEBUG] popen() failed for command: " << cmd << std::endl;
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    std::cout << "[DEBUG] Command executed: " << cmd << ", Output: " << result << std::endl;
    return result;
}

// Helper function to check if a Microhard device is present
bool is_microhard_device_present() {
    const std::string wfb_path = std::string(getConfigBasePath()) + "wfb.txt";
    const std::string eth_path = std::string(getConfigBasePath()) + "ethernet.txt";
    std::cout << "[DEBUG] Checking if wfb.txt exists: " << wfb_path << "\n";
    std::cout << "[DEBUG] Checking if ethernet.txt exists: " << eth_path << "\n";

    if (!OHDFilesystemUtil::exists(wfb_path) &&
        !OHDFilesystemUtil::exists(eth_path)) {
        std::string output = exec("lsusb");
        std::cout << "[DEBUG] lsusb output: " << output << "\n";
        return output.find("Microhard") != std::string::npos;
    }
    return false;
}

OHDInterface::OHDInterface(OHDProfile profile1)
    : m_profile(std::move(profile1)) {
    m_console = openhd::log::create_or_get("interface");
    assert(m_console);
    std::cout << "[DEBUG] Console logger initialized\n";

    m_monitor_mode_cards = {};
    m_opt_hotspot_card = std::nullopt;
    const auto config = openhd::load_config();
    std::cout << "[DEBUG] Configuration loaded: " << config << "\n";

    bool microhard_device_present = is_microhard_device_present();
    std::cout << "[DEBUG] Microhard device present: " << microhard_device_present << "\n";

    if (OHDFilesystemUtil::exists(std::string(getConfigBasePath()) + "ethernet.txt")) {
        std::cout << "[DEBUG] Ethernet configuration file detected\n";
        m_ethernet_link = std::make_shared<EthernetLink>(config, m_profile);
        m_console->warn("Ethernet link initialized");
        return;
    }

    if (microhard_device_present) {
        std::cout << "[DEBUG] Microhard device found\n";
        m_microhard_link = std::make_shared<MicrohardLink>(m_profile);
        m_console->warn("Microhard link initialized");
        return;
    }

    std::cout << "[DEBUG] Discovering WiFi cards\n";
    DWifiCards::main_discover_an_process_wifi_cards(
        config, m_profile, m_console, m_monitor_mode_cards, m_opt_hotspot_card);
    std::cout << "[DEBUG] WiFi cards discovered. Monitor mode cards: " << debug_cards(m_monitor_mode_cards) << "\n";

    if (m_opt_hotspot_card.has_value()) {
        std::cout << "[DEBUG] Hotspot card: " << m_opt_hotspot_card.value().device_name << "\n";
    } else {
        std::cout << "[DEBUG] No WiFi hotspot card found\n";
    }

    if (m_monitor_mode_cards.empty()) {
        std::cerr << "[DEBUG] No WiFi cards available for monitor mode\n";
        const std::string message_for_user = "No WiFi card found, please reboot";
        m_console->warn(message_for_user);
        openhd::LEDManager::instance().set_status_error();
        return;
    } else {
        openhd::wb::takeover_cards_monitor_mode(m_monitor_mode_cards, m_console);
        m_wb_link = std::make_shared<WBLink>(m_profile, m_monitor_mode_cards);
        std::cout << "[DEBUG] Monitor mode cards initialized\n";
    }

    if (m_profile.is_ground()) {
        m_usb_tether_listener = std::make_unique<USBTetherListener>();
        std::cout << "[DEBUG] USB tethering listener initialized\n";

        m_ethernet_manager = std::make_unique<EthernetManager>();
        m_ethernet_manager->async_initialize(
            m_nw_settings.get_settings().ethernet_operating_mode);
        std::cout << "[DEBUG] Ethernet manager initialized with async settings\n";
    }

    if (m_opt_hotspot_card.has_value()) {
        if (WiFiClient::create_if_enabled()) {
            std::cout << "[DEBUG] WiFi client mode enabled\n";
        } else {
            const openhd::WifiSpace wb_frequency_space =
                (m_wb_link != nullptr)
                    ? m_wb_link->get_current_frequency_channel_space()
                    : openhd::WifiSpace::G5_8;
            m_wifi_hotspot = std::make_unique<WifiHotspot>(
                m_profile, m_opt_hotspot_card.value(), wb_frequency_space);
            update_wifi_hotspot_enable();
            std::cout << "[DEBUG] WiFi hotspot initialized\n";
        }
    }

    if (m_wifi_hotspot) {
        auto cb = [this](bool armed) { update_wifi_hotspot_enable(); };
        openhd::ArmingStateHelper::instance().register_listener("ohd_interface_wifi", cb);
        std::cout << "[DEBUG] Registered WiFi hotspot state callback\n";
    }

    m_console->debug("OHDInterface created successfully");
}

OHDInterface::~OHDInterface() {
    std::cout << "[DEBUG] Destructor called for OHDInterface\n";
    m_wb_link = nullptr;

    openhd::wb::giveback_cards_monitor_mode(m_monitor_mode_cards, m_console);
    std::cout << "[DEBUG] Monitor mode cards released\n";

    if (m_ethernet_manager) {
        m_ethernet_manager->stop();
        std::cout << "[DEBUG] Ethernet manager stopped\n";
        m_ethernet_manager = nullptr;
    }
}

std::vector<openhd::Setting> OHDInterface::get_all_settings() {
    std::cout << "[DEBUG] Gathering all settings\n";
    std::vector<openhd::Setting> ret;

    if (m_wb_link) {
        auto settings = m_wb_link->get_all_settings();
        OHDUtil::vec_append(ret, settings);
        std::cout << "[DEBUG] WBLink settings appended\n";
    }

    if (m_microhard_link) {
        auto settings = m_microhard_link->get_all_settings();
        OHDUtil::vec_append(ret, settings);
        std::cout << "[DEBUG] Microhard link settings appended\n";
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
            openhd::IntSetting{m_nw_settings.get_settings().wifi_hotspot_mode, cb_wifi_hotspot_mode}});
        std::cout << "[DEBUG] WiFi hotspot settings added\n";
    }

    if (m_profile.is_ground()) {
        const auto settings = m_nw_settings.get_settings();
        auto cb_ethernet = [this](std::string, int value) {
            m_nw_settings.unsafe_get_settings().ethernet_operating_mode = value;
            m_nw_settings.persist();
            return true;
        };
        ret.push_back(openhd::Setting{
            "ETHERNET",
            openhd::IntSetting{settings.ethernet_operating_mode, cb_ethernet}});
        std::cout << "[DEBUG] Ethernet settings added\n";
    }

    openhd::validate_provided_ids(ret);
    std::cout << "[DEBUG] Settings validation complete\n";
    return ret;
}
