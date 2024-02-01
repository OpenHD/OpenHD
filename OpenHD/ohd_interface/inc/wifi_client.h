//
// Created by consti10 on 30.01.24.
//

#ifndef OPENHD_WIFI_CLIENT_H
#define OPENHD_WIFI_CLIENT_H

#include <optional>
#include <string>

#include "wifi_card.h"

/**
 * FEATURE: Disable hotspot and automatically connect to a given network
 * USAGE: See hardware.config file !
 */
class WiFiClient {
 public:
  static bool create_if_enabled();
};

#endif  // OPENHD_WIFI_CLIENT_H
