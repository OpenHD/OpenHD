//
// Created by consti10 on 30.01.24.
//

#ifndef OPENHD_WIFI_CLIENT_H
#define OPENHD_WIFI_CLIENT_H

#include <string>
#include <optional>
#include "wifi_card.h"

/**
 * FEATURE: Disable hotspot and automatically connect to a given network
 * USAGE: Create file /boot/openhd/wifi_client.txt containing 2 lines -
 * ssid in first line, pw in second line
 * Then reboot.
 */
class WiFiClient {
public:
    struct Configuration{
        std::string network_name;
        std::string password;
    };
    // Returns a valid configuration if it exists,
    // Otherwise std::nullopt
    static std::optional<Configuration> get_configuration();
    static std::string create_command(const Configuration& configuration);

    static bool create_if_enabled();
};


#endif //OPENHD_WIFI_CLIENT_H
