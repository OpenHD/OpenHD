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
namespace DWifiCards {

// There is no way to tell if a card supports injection in monitor mode other than keeping track of a list of the cards
// which we know can do injection
static bool is_known_for_injection(const WiFiCardType& type);

// discover all connected wifi cards
std::vector<WiFiCard> discover();

// Return true if any of the given wifi cards supports monitor mode
bool any_wifi_card_supporting_monitor(const std::vector<WiFiCard>& cards);

// helper to figure out more info about a semi-discovered wifi card
std::optional<WiFiCard> process_card(const std::string &interface_name);

struct ProcessedWifiCards{
  std::vector<WiFiCard> monitor_mode_cards;
  std::optional<WiFiCard> hotspot_card;
};

ProcessedWifiCards process_and_evaluate_cards(std::vector<WiFiCard> discovered_cards,bool max_one_broadcast_card);

};

#endif //OHD_DISCOVER_WiFI_CARDS

