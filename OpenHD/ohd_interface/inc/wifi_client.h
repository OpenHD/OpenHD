//
// Created by consti10 on 30.01.24.
//

#ifndef OPENHD_WIFI_CLIENT_H
#define OPENHD_WIFI_CLIENT_H

#include <string>
#include <optional>
#include "wifi_card.h"

/**
 * Looks if a file with wifi network name and password exists
 * If this file exists, try connecting to the given network
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
