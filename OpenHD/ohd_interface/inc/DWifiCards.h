#ifndef OHD_DISCOVER_WiFI_CARDS
#define OHD_DISCOVER_WiFI_CARDS

#include <array>
#include <chrono>
#include <vector>
#include <optional>

#include "openhd-wifi.hpp"
#include "openhd-platform.hpp"
#include "openhd-discoverable.hpp"

/**
 * Discover all connected wifi cards.
 */
class DWifiCards {
 public:
  static std::vector<WiFiCard> discover();
 private:
  static std::optional<WiFiCard> process_card(const std::string &interface_name);
};

#endif //OHD_DISCOVER_WiFI_CARDS

