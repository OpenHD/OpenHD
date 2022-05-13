#include "WifiCards.h"

#include <iostream>


#include "json.hpp"
#include "inja.hpp"
#include "openhd-log.hpp"
#include "openhd-settings.hpp"
#include "openhd-wifi.hpp"
#include "openhd-util.hpp"

WifiCards::WifiCards(const OHDProfile& profile): profile(profile){}


void WifiCards::configure() {
    std::cout << "WifiCards::configure()" << std::endl;
    //Find out which cards are connected first
    process_manifest();
    // Consti10 - now do some sanity checks. No idea if and how the settings from stephen handle default values.
    for (auto& card : m_wifi_cards) {
        if(card.use_for==WifiUseForMonitorMode && profile.is_air){
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
    m_wifi_cards=wificards_from_manifest();
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
            //m_broadcast_cards.push_back(card);
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
    //std::cout << "WifiCards::setup_hotspot(" << card.name << ")" << std::endl;
}


bool WifiCards::set_card_state(const WiFiCard& card, bool up) {
    std::cout << "WifiCards::set_card_state(" << up << ") for " << card.name <<  ")" << std::endl;
    std::vector<std::string> args { "link", "set", "dev", card.name, up ? "up" : "down" };
    bool success = run_command("ip", args);
    return success;
}

bool WifiCards::set_frequency(const WiFiCard& card, const std::string& frequency) {
    std::cout << "WifiCards::set_frequency(" << frequency << ") for " << card.name <<  ")" << std::endl;
    std::vector<std::string> args { "dev", card.name, "set", "freq", frequency };
    bool success = run_command("iw", args);
    return success;
}

bool WifiCards::set_txpower(const WiFiCard& card, const std::string& txpower) {
    std::cout << "WifiCards::set_txpower(" << txpower << ") for " << card.name <<  ")" << std::endl;
    std::vector<std::string> args { "dev", card.name, "set", "txpower", "fixed", txpower };
    bool success = run_command("iw", args);
    return success;
}

bool WifiCards::enable_monitor_mode(const WiFiCard& card) {
    std::cout << "WifiCards::enable_monitor_mode(" << card.name <<  ")" << std::endl;
    std::vector<std::string> args { "dev", card.name, "set", "monitor", "otherbss" };
    bool success = run_command("iw", args);
    return success;
}

void WifiCards::save_settings(const std::vector<WiFiCard>& cards, const std::string& settings_file) {
    std::cerr<<"Unimplemented\n";
}

std::vector<std::string> WifiCards::get_broadcast_card_names()const {
    std::vector<std::string> names;
    for(const auto& card: m_wifi_cards){
        if(card.use_for==WifiUseForMonitorMode){
            names.push_back(card.name);
        }
    }
    return names;
}
