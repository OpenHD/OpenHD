#ifndef OHD_DISCOVER_WiFI_CARDS
#define OHD_DISCOVER_WiFI_CARDS

#include <array>
#include <chrono>
#include <optional>
#include <vector>

#include "openhd-platform.hpp"
#include "wifi_card.hpp"

/**
 * Discover all connected wifi cards.
 */
class DWifiCards {
 public:
  static std::vector<WiFiCard> discover();
  // Return true if any of the given wifi cards supports monitor mode
  static bool any_wifi_card_supporting_monitor(const std::vector<WiFiCard>& cards);
 private:
  static std::optional<WiFiCard> process_card(const std::string &interface_name);
};

#endif //OHD_DISCOVER_WiFI_CARDS

