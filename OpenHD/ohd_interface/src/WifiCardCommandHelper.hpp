//
// Created by consti10 on 13.07.22.
//

#ifndef OPENHD_WIFICARDSCOMMANDSHELPER_H
#define OPENHD_WIFICARDSCOMMANDSHELPER_H

#include "openhd-log.hpp"
#include "openhd-wifi.hpp"
#include "openhd-util.hpp"

// Helper for the commands we need to do stuff with a wifi card (like setting the frequency, txpower, monitor mode, ...)
// In general, just a wrapper around the corresponding system commands.

namespace WifiCardCommandHelper{
    static void setup_hotspot(const WiFiCard &card) {
        std::cerr << "Setup hotspot unimplemented right now\n";
        //std::cout << "WifiCards::setup_hotspot(" << card.name << ")" << std::endl;
    }

    static bool set_card_state(const WiFiCard &card, bool up) {
        std::cout << "WifiCards::set_card_state(" << up << ") for " << card.interface_name << ")" << std::endl;
        std::vector<std::string> args{"link", "set", "dev", card.interface_name, up ? "up" : "down"};
        bool success = OHDUtil::run_command("ip", args);
        return success;
    }

    static bool set_frequency(const WiFiCard &card, const std::string &frequency) {
        std::cout << "WifiCards::set_frequency(" << frequency << ") for " << card.interface_name << ")" << std::endl;
        std::vector<std::string> args{"dev", card.interface_name, "set", "freq", frequency};
        bool success = OHDUtil::run_command("iw", args);
        return success;
    }

    // Consti10: this at least changes what then iw dev displays. If it internally has an effect is not yet tested.
    static bool set_txpower(const WiFiCard &card, const std::string &txpower) {
        std::cout << "WifiCards::set_txpower(" << txpower << ") for " << card.interface_name << ")" << std::endl;
        std::vector<std::string> args{"dev", card.interface_name, "set", "txpower", "fixed", txpower};
        bool success = OHDUtil::run_command("iw", args);
        return success;
    }

    static bool enable_monitor_mode(const WiFiCard &card) {
        std::cout << "WifiCards::enable_monitor_mode(" << card.interface_name << ")" << std::endl;
        std::vector<std::string> args{"dev", card.interface_name, "set", "monitor", "otherbss"};
        bool success = OHDUtil::run_command("iw", args);
        return success;
    }
}
#endif //OPENHD_WIFICARDSCOMMANDSHELPER_H
