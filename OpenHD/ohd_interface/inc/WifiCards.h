#ifndef WIFI_H
#define WIFI_H

#include "openhd-wifi.hpp"

#include <vector>
#include <string>

/**
 * Provides access to the discovered wifi cards on the system.
 * There should only be one instance of this class in the whole OpenHD project.
 */
class WifiCards {
 public:
  static void setup_hotspot(const WiFiCard &card);
  static bool set_card_state(const WiFiCard &card, bool up);
  static bool set_frequency(const WiFiCard &card, const std::string &frequency);
  static bool set_txpower(const WiFiCard &card, const std::string &txpower);
  static bool enable_monitor_mode(const WiFiCard &card);
  static void save_settings(const std::vector<WiFiCard> &cards, const std::string &settings_file);
};

#endif

