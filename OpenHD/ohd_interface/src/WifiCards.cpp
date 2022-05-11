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

#include "WifiCards.h"

WifiCards::WifiCards(bool is_air, std::string unit_id): m_is_air(is_air), m_unit_id(unit_id) {}


void WifiCards::configure() {
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
        //if (setting_map.count("use_for")) card.use_for = setting_map["use_for"];
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
    }
    // fucking hell, just go with what we used to do in EZ-Wifibroadcast -
    for(auto& card: m_wifi_cards){
        if(card.supports_injection){
            card.use_for=WifiUseForMonitorMode;
        }else if(card.supports_hotspot){
            // if a card does not support injection, we use it for hotspot
            card.use_for=WifiUseForHotspot;
        }else{
            // and if a card supports neither hotspot nor injection, we use it for nothing
            card.use_for=WifiUseForUnknown;
        }
    }

    // Consti10 - now do some sanity checks. No idea if and how the settings from stephen handle default values.
    for (auto& card : m_wifi_cards) {
        if(card.use_for==WifiUseForMonitorMode && m_is_air){
            // There is no wifi hotspot created on the air pi
            std::cerr<<"No hotspot on air\n";
            card.use_for=WifiUseForUnknown;
        }
        if(card.use_for==WifiUseForMonitorMode && !card.supports_injection){
            std::cerr<<"Cannot use monitor mode on card that cannot inject\n";
            card.use_for=WifiUseForUnknown;
        }
        // Set the default frequency if not set yet.
        // If the card supports 5ghz, prefer the 5ghz band
        if(card.frequency.empty()){
            if(card.supports_5ghz){
                card.frequency=m_default_5ghz_frequency;
            }else{
                card.frequency=m_default_2ghz_frequency;
            }
        }
        //
        if(card.txpower.empty()){
            // No idea where this comes from, for now use it ??!!
            // TODO what should be the default value ?
            card.txpower="3100";
        }
    }
    // When we are done with the sanity checks, we can use the wifi card for its intended use case
    for(const auto& card: m_wifi_cards){
        setup_card(card);
    }

}


void WifiCards::process_manifest() {
    try {
        std::ifstream f("/tmp/wifi_manifest");
        nlohmann::json j;
        f >> j;

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


void WifiCards::setup_card(const WiFiCard &card) {
    std::cerr << "Setup card: " << card.name << std::endl;
    switch (card.use_for) {
        case WifiUseForMonitorMode:
            set_card_state(card, false);
            enable_monitor_mode(card);
            set_card_state(card, true);
            set_frequency(card, card.frequency);
            set_txpower(card, card.txpower);
            m_broadcast_cards.push_back(card);
            break;
        case WifiUseForHotspot:
            setup_hotspot(card);
            break;
        case WifiUseForUnknown:
        default:
            std::cerr<<"Card "<<card.name<<" unknown use for\n";
            return;
    }
}


void WifiCards::setup_hotspot(const WiFiCard &card) {
    std::cerr<<"Setup hotspot unimplemented right now\n";
    //std::cout << "WiFi::setup_hotspot(" << card.name << ")" << std::endl;
}


bool WifiCards::set_card_state(const WiFiCard& card, bool up) {
    std::cout << "WiFi::set_card_state(" << up << ") for " << card.name <<  ")" << std::endl;
    std::vector<std::string> args { "link", "set", "dev", card.name, up ? "up" : "down" };
    bool success = run_command("ip", args);
    return success;
}


bool WifiCards::set_frequency(const WiFiCard& card, const std::string& frequency) {
    std::cout << "WiFi::set_frequency(" << frequency << ") for " << card.name <<  ")" << std::endl;
    std::vector<std::string> args { "dev", card.name, "set", "freq", frequency };
    bool success = run_command("iw", args);
    return success;
}


bool WifiCards::set_txpower(const WiFiCard& card, const std::string& txpower) {
    std::cout << "WiFi::set_txpower(" << txpower << ") for " << card.name <<  ")" << std::endl;
    std::vector<std::string> args { "dev", card.name, "set", "txpower", "fixed", txpower };
    bool success = run_command("iw", args);
    return success;
}


bool WifiCards::enable_monitor_mode(const WiFiCard& card) {
    std::cout << "WiFi::enable_monitor_mode(" << card.name <<  ")" << std::endl;
    std::vector<std::string> args { "dev", card.name, "set", "monitor", "otherbss" };
    bool success = run_command("iw", args);
    return success;
}


void WifiCards::save_settings(const std::vector<WiFiCard>& cards, const std::string& settings_file) {
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
