#include <cstdio>
#include <stdio.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
//#include <boost/algorithm/string.hpp>
//#include <boost/filesystem.hpp>


#include <boost/regex.hpp>

#include "json.hpp"
#include "inja.hpp"

#include "openhd-log.hpp"
#include "openhd-settings.hpp"
#include "openhd-wifi.hpp"
#include "openhd-util.hpp"

#include "wifi.h"

WiFi::WiFi(bool is_air, std::string unit_id):m_is_air(is_air), m_unit_id(unit_id) {}


void WiFi::configure() {
    std::cout << "WiFi::configure()" << std::endl;

    /*
     * Find out which cards are connected first
     */
    process_manifest();


    /*
     * Then get the local settings, if there are any
     */
    std::vector<std::map<std::string, std::string> > settings;

    try {
        const std::string settings_path = findOrCreateSettingsDirectory(m_is_air);
        std::string settings_file = settings_path + "/wifi.conf";
        std::cerr << "settings_file: " << settings_file << std::endl;
        settings = read_config(settings_file);
    } catch (std::exception &ex) {
        std::cerr << "WiFi settings load error: " << ex.what() << std::endl;
    }


    /*
     * Now use the settings to override the detected hardware configuration in each WiFiCard in m_wifi_cards
     *
     */
    std::vector<WiFiCard> save_cards;

    for (auto card : m_wifi_cards) {
        std::map<std::string, std::string> setting_map;

        for (auto & settings_for_card : settings) {
            if (settings_for_card.count("mac") == 1 && settings_for_card["mac"] == card.mac) {
                setting_map = settings_for_card;
                break;
            }
        }

        if (setting_map.count("frequency")) card.frequency = setting_map["frequency"];
        if (setting_map.count("txpower")) card.txpower = setting_map["txpower"];
        if (setting_map.count("use_for")) card.use_for = setting_map["use_for"];
        if (setting_map.count("wifi_client_ap_name")) card.wifi_client_ap_name = setting_map["wifi_client_ap_name"];
        if (setting_map.count("wifi_client_password")) card.wifi_client_password = setting_map["wifi_client_password"];
        if (setting_map.count("hotspot_channel")) card.hotspot_channel = setting_map["hotspot_channel"];
        if (setting_map.count("hotspot_password")) card.hotspot_password = setting_map["hotspot_password"];
        if (setting_map.count("hotspot_band")) card.hotspot_band = setting_map["hotspot_band"];

        save_cards.push_back(card);

            /*
        * And now save the complete set of wifi cards back to the settings file, ensuring that all hardware
        * ends up in the file automatically but users can change it as needed
        */
        try {
            const std::string settings_path = findOrCreateSettingsDirectory(m_is_air);
            std::string settings_file = settings_path + "/wifi.conf";
            save_settings(save_cards, settings_file);
        } catch (std::exception &ex) {
            ohd_log(STATUS_LEVEL_EMERGENCY, "WiFi settings save failed");
        }

        process_card(card);
    }  
}


void WiFi::process_manifest() {
    try {
        std::ifstream f("/tmp/wifi_manifest");
        nlohmann::json j;
        f >> j;

        m_wifi_hotspot_type = string_to_wifi_hotspot_type(j["hotspot"]);

        for (auto _card : j["cards"]) {

            WiFiCard card;
            card.name = _card["name"];

            card.type = string_to_wifi_card_type(_card["type"]);

            card.supports_5ghz = _card["supports_5ghz"];
            card.supports_2ghz = _card["supports_2ghz"];
            card.supports_injection = _card["supports_injection"];
            card.supports_hotspot = _card["supports_hotspot"];
            card.supports_rts = _card["supports_rts"];
            card.mac = _card["mac"];

            m_wifi_cards.push_back(card);
        }
    } catch (std::exception &ex) {
        // don't do anything, but send an error message to the user through the status service
        ohd_log(STATUS_LEVEL_EMERGENCY, "WiFi manifest processing failed");
        std::cerr << "WiFi::process_manifest: " << ex.what() << std::endl;
        return;
    }
}


void WiFi::process_card(WiFiCard &card) {
    std::cerr << "Processing card: " << card.name << std::endl;

    // if there isn't a settings file for this card yet, this field will not be set and it will
    // be up to the autodetected settings from the system service to decide what this card will be
    // used for
    if (card.use_for == "hotspot") {
        if (!m_is_air) {
            setup_hotspot(card);
        }
        return;
    }

    if (card.use_for == "client") {
        //setup_client(card);
        return;
    }


    if (!card.supports_injection) {
        if (!m_is_air) {
            std::cerr << "Card does not support injection: " << card.name << std::endl;
            setup_hotspot(card);
        }
        return;
    }

    card.use_for = "wifibroadcast";


    // todo: errors encountered here need to be submitted to the status service, users will never see stdout
    set_card_state(card, false);
    enable_monitor_mode(card);
    set_card_state(card, true);

    
    if (!card.frequency.empty()) {
        set_frequency(card, card.frequency);
    } else {
        if (card.supports_5ghz) {
            set_frequency(card, m_default_5ghz_frequency);
            card.frequency = m_default_5ghz_frequency;
        } else {
            set_frequency(card, m_default_2ghz_frequency);
            card.frequency = m_default_2ghz_frequency;
        }
    }

    if (!card.txpower.empty()) {
        set_txpower(card, card.txpower);
    } else {
        set_txpower(card, "3100");
        card.txpower = "3100";
    }
    std::cout << "WiFi::process card() push_back" << std::endl;
    m_broadcast_cards.push_back(card);
}


/*
 *
 * todo: deduplicate this and similar logic in Ethernet/LTE, they should be subclasses of a common base
 */
void WiFi::setup_hotspot(WiFiCard &card) {
    std::cout << "WiFi::setup_hotspot(" << card.name << ")" << std::endl;

    if (!card.supports_hotspot) {
        std::ostringstream message;
        message << "WiFi hotspot not supported on ";
        message << wifi_card_type_to_string(card.type);
        message << "cards (";
        message << card.name;
        message << ")";
        message << std::endl;
        ohd_log(STATUS_LEVEL_INFO, message.str());
        return;
    }

    bool success = false;

    if (m_hotspot_configured) {
        std::cout << "WiFi::setup_hotspot: already configured with another card" << std::endl;
        return;
    }

    card.use_for = "hotspot";

    // todo: allow the interface address to be configured. this requires changing the dnsmasq config file though, not
    //       just the interface address. 
    std::vector<std::string> args { card.name, m_wifi_hotspot_address, "up", };

    success = run_command("ifconfig", args);

    if (!success) {
        ohd_log(STATUS_LEVEL_WARNING, "Failed to enable wifi hotspot interface");
        return;
    }

    /*
     * Note: This is not currently choosing to use the band that wifibroadcast cards are not using. Most
     *       people seem to choose not to use the G band no matter what, so we might as well just default to 5Ghz
     *       and then perhaps lean on FEC to avoid interference, it's far more likely things will work that way
     *       than to prefer the 2.4ghz band in any situation.
     *
     */
    if (card.hotspot_channel.empty()) {
        if (card.supports_5ghz) {
            card.hotspot_channel = "36";
        } else {
            card.hotspot_channel = "11";
        }
    }

    if (card.hotspot_band.empty()) {
        if (card.supports_5ghz) {
            card.hotspot_band = "a";
        } else {
            card.hotspot_band = "g";
        }
    }

    if (card.txpower.empty()) {
        card.txpower = m_wifi_hotspot_txpower;
    }

    if (card.hotspot_password.empty()) {
        card.hotspot_password = "wifiopenhd";
    }

    std::ostringstream message1;
    message1 << "WiFi hotspot enabled on band ";
    message1 << card.hotspot_band;
    message1 << " channel ";
    message1 << card.hotspot_channel;
    message1 << std::endl;
    ohd_log(STATUS_LEVEL_INFO, message1.str());

    // TODO we should not just call scripts from code
    {
        std::vector<std::string> args { 
            "/usr/local/share/wifibroadcast-scripts/wifi_hotspot.sh", card.hotspot_band, card.hotspot_channel, card.name, card.txpower, card.hotspot_password
        };

        success = run_command("/bin/bash", args);

        if (!success) {
            ohd_log(STATUS_LEVEL_WARNING, "Failed to enable hostap on wifi hotspot");
            return;
        }
    }
    
    m_hotspot_configured = true;
}


bool WiFi::set_card_state(const WiFiCard& card, bool up) {
    std::cout << "WiFi::set_card_state(" << up << ") for " << card.name <<  ")" << std::endl;
    std::vector<std::string> args { "link", "set", "dev", card.name, up ? "up" : "down" };
    bool success = run_command("ip", args);
    return success;
}


bool WiFi::set_frequency(const WiFiCard& card, const std::string& frequency) {
    std::cout << "WiFi::set_frequency(" << frequency << ") for " << card.name <<  ")" << std::endl;
    std::vector<std::string> args { "dev", card.name, "set", "freq", frequency };
    bool success = run_command("iw", args);
    return success;
}


bool WiFi::set_txpower(const WiFiCard& card, const std::string& txpower) {
    std::cout << "WiFi::set_txpower(" << txpower << ") for " << card.name <<  ")" << std::endl;
    std::vector<std::string> args { "dev", card.name, "set", "txpower", "fixed", txpower };
    bool success = run_command("iw", args);
    return success;
}


bool WiFi::enable_monitor_mode(const WiFiCard& card) {
    std::cout << "WiFi::enable_monitor_mode(" << card.name <<  ")" << std::endl;
    std::vector<std::string> args { "dev", card.name, "set", "monitor", "otherbss" };
    bool success = run_command("iw", args);
    return success;
}


void WiFi::save_settings(const std::vector<WiFiCard>& cards, const std::string& settings_file) {
    inja::Environment env;

    // load the wifi card template, we format it once for each card and write that to the file
    std::ifstream template_file("/usr/local/share/openhd/wificard.template");
    std::string template_s((std::istreambuf_iterator<char>(template_file)),
                          std::istreambuf_iterator<char>());

    std::ofstream out(settings_file);

    // now fill in the template params
    for (auto & card : cards) {
        nlohmann::json data;

        data["type"] = wifi_card_type_to_string(card.type);
        data["mac"] = card.mac;
        data["name"] = card.name;
        data["vendor"] = card.vendor;
        data["frequency"] = card.frequency;
        data["txpower"] = card.txpower;
        data["use_for"] = card.use_for;
        data["wifi_client_ap_name"] = card.wifi_client_ap_name;
        data["wifi_client_password"] = card.wifi_client_password;
        data["hotspot_channel"] = card.hotspot_channel;
        data["hotspot_password"] = card.hotspot_password;
        data["hotspot_band"] = card.hotspot_band;

        inja::Template temp = env.parse(template_s.c_str());
        std::string rendered = env.render(temp, data);

        // and write this card to the settings file
        out << rendered;
        out << "\n\n";
    }
    
    out.close();
}
