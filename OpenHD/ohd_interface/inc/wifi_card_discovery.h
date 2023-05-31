#ifndef OHD_DISCOVER_WiFI_CARDS
#define OHD_DISCOVER_WiFI_CARDS

#include <array>
#include <chrono>
#include <optional>
#include <vector>

#include "openhd_platform.h"
#include "openhd_profile.h"
#include "wifi_card.h"

/**
 * Discover all connected wifi cards.
 */
namespace DWifiCards {

// There is no way to tell if a card supports injection in monitor mode other than keeping track of a list of the cards
// which we know can do injection
bool is_known_for_injection(const WiFiCardType& type);

// this should never fail (return std::nullopt) if the given interface_name is valid
std::optional<WiFiCard> fill_linux_wifi_card_identifiers(const std::string& interface_name);

// helper to figure out more info about a semi-discovered wifi card
std::optional<WiFiCard> process_card(const std::string &interface_name);

// discover all connected wifi cards and their capabilities
std::vector<WiFiCard> discover_connected_wifi_cards();

// Calculate how many cards support injection
int n_cards_supporting_injection(const std::vector<WiFiCard>& cards);

// Return true if any of the given wifi cards supports injection in monitor mode
bool any_wifi_card_supporting_injection(const std::vector<WiFiCard>& cards);

// Return true if any of the given wifi cards supports monitor mode (but perhaps / quite likely no injection
bool any_wifi_card_supporting_monitor_mode(const std::vector<WiFiCard>& cards);

struct ProcessedWifiCards{
  std::vector<WiFiCard> monitor_mode_cards;
  std::optional<WiFiCard> hotspot_card;
};

ProcessedWifiCards process_and_evaluate_cards(const std::vector<WiFiCard>& discovered_cards,const OHDPlatform& platform,const OHDProfile& profile);

// for users who use the manual file to define their card(s)
ProcessedWifiCards find_cards_from_manual_file(const std::vector<std::string>& wifibroadcast_cards,const std::string& opt_hotspot_card);

};

#endif //OHD_DISCOVER_WiFI_CARDS

