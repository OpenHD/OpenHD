#ifndef OHD_DISCOVER_WiFI_CARDS
#define OHD_DISCOVER_WiFI_CARDS

#include <array>
#include <chrono>
#include <optional>
#include <vector>

#include "openhd_platform.h"
#include "openhd_profile.h"
#include "wifi_card.h"
#include "openhd_config.h"

/**
 * Discover all connected wifi cards.
 */
namespace DWifiCards {

void main_discover_an_process_wifi_cards(const openhd::Config& config,const OHDProfile& m_profile,
                                        const OHDPlatform& m_platform,
                                        const bool continue_without_wb_card,
                                        std::shared_ptr<spdlog::logger>& m_console,
                                        std::vector<WiFiCard>& m_monitor_mode_cards,
                                        std::optional<WiFiCard>& m_opt_hotspot_card)

// Returns true if the card is supported (we only support a few, but keep working with other cards)
bool is_openhd_supported(const WiFiCardType& type);

// this should never fail (return std::nullopt) if the given interface_name is valid
std::optional<WiFiCard> fill_linux_wifi_card_identifiers(const std::string& interface_name);

// helper to figure out more info about a semi-discovered wifi card
std::optional<WiFiCard> process_card(const std::string &interface_name);

// discover all connected wifi cards and their capabilities
std::vector<WiFiCard> discover_connected_wifi_cards();

// Calculate how many cards found are supported by OpenHD
int n_cards_openhd_supported(const std::vector<WiFiCard>& cards);

// Return true if any of the given wifi cards is supported by OpenHD
bool any_wifi_card_openhd_supported(const std::vector<WiFiCard>& cards);

// Return true if any of the given wifi cards supports monitor mode (but perhaps / quite likely no injection and definitely
// not the full openhd feature set)
bool any_wifi_card_supporting_monitor_mode(const std::vector<WiFiCard>& cards);

struct ProcessedWifiCards{
  std::vector<WiFiCard> monitor_mode_cards;
  std::optional<WiFiCard> hotspot_card;
};

ProcessedWifiCards process_and_evaluate_cards(const std::vector<WiFiCard>& discovered_cards,const OHDPlatform& platform,const OHDProfile& profile);

// for users who use the manual file to define their card(s)
ProcessedWifiCards find_cards_from_manual_file(const std::vector<std::string>& wifibroadcast_cards,const std::string& opt_hotspot_card);

WiFiCard create_card_monitor_emulate();
};

#endif //OHD_DISCOVER_WiFI_CARDS

