#ifndef OHD_DISCOVER_WiFI_CARDS
#define OHD_DISCOVER_WiFI_CARDS

#include <array>
#include <chrono>
#include <vector>

#include "openhd-wifi.hpp"
#include "openhd-platform.hpp"
#include "openhd-discoverable.hpp"

/**
 * Discover all connected wifi cards and write them to json.
 */
class DWifiCards {
 public:
  explicit DWifiCards()=default;
  virtual ~DWifiCards() = default;
  std::vector<WiFiCard> discover();
 private:
  void process_card(const std::string &interface_name);
 private:
  std::vector<WiFiCard> m_wifi_cards;
};

#endif //OHD_DISCOVER_WiFI_CARDS

